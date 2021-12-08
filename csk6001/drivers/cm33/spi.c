/*
 * Copyright (c) 2020-2025 ChipSky Technology
 * All rights reserved.
 *
 */

#include "hal/spi.h"
#include "hal/ClockManager.h" // for CRM_GetSrcFreq()
// #include "venus_log.h"
#include <assert.h>
#include <stdio.h>

#define DEBUG_LOG   1 // 1
#if DEBUG_LOG
#define LOGD(format, ...)   {}
#else
#define LOGD(format, ...)   ((void)0)
#endif // DEBUG_LOG

#define CSK_SPI_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)

// driver version
static const
CSK_DRIVER_VERSION spi_driver_version = { CSK_SPI_API_VERSION, CSK_SPI_DRV_VERSION };

static void spi_dma_tx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param);
static void spi_dma_rx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param);
void spi_irq_handler(SPI_DEV *spi);
static void spi_abort_transfer(SPI_DEV *spi, bool keep_state);

//------------------------------------------------------------------------------------
// SPI0
static SPI_DEV spi0_dev;
static void spi0_irq_handler(void) { spi_irq_handler(&spi0_dev); }

static SPI_INFO spi0_info = { 0 };
static SPI_DEV spi0_dev = {
        CSK_SPI0,
        CSK_SPI0_CLK,
        CSK_SPI_TXFIFO_DEPTH,
        CSK_SPI_RXFIFO_DEPTH,
        IRQ_SPI0_VECTOR,
        spi0_irq_handler,

        .dma_tx = {
                DMA_CH_SPI_TX_DEF,
                DMA_HSID_SPI0_TX,
                spi_dma_tx_event,
                (uint32_t)(&spi0_dev)
        },

        .dma_rx = {
                DMA_CH_SPI_RX_DEF,
                DMA_HSID_SPI0_RX,
                spi_dma_rx_event,
                (uint32_t)(&spi0_dev)
        },

        &spi0_info
};

// export SPI API function: SPI0
void* SPI0() { return &spi0_dev; }


// SPI1
static SPI_DEV spi1_dev;
static void spi1_irq_handler(void) { spi_irq_handler(&spi1_dev); }

static SPI_INFO spi1_info = { 0 };
static SPI_DEV spi1_dev = {
        CSK_SPI1,
        CSK_SPI1_CLK,
        CSK_SPI_TXFIFO_DEPTH,
        CSK_SPI_RXFIFO_DEPTH,
        IRQ_SPI1_VECTOR,
        spi1_irq_handler,

        .dma_tx = {
                DMA_CH_SPI_TX_DEF,
                DMA_HSID_SPI1_TX,
                spi_dma_tx_event,
                (uint32_t)(&spi1_dev)
        },

        .dma_rx = {
                DMA_CH_SPI_RX_DEF,
                DMA_HSID_SPI1_RX,
                spi_dma_rx_event,
                (uint32_t)(&spi1_dev)
        },

        &spi1_info
};

// export SPI API function: SPI1
void* SPI1() { return &spi1_dev; }


//------------------------------------------------------------------------------------
static SPI_DEV * safe_spi_dev(void *spi_dev)
{
    //TODO: safe check of SPI device parameter
	if (spi_dev == &spi0_dev) {
		if (spi0_dev.reg != CSK_SPI0 || spi0_dev.irq_num != IRQ_SPI0_VECTOR) {
			//CLOGW("SPI0 device context has been tampered illegally!!\n");
			return NULL;
		}
	} else if (spi_dev == &spi1_dev) {
		if (spi1_dev.reg != CSK_SPI1 || spi1_dev.irq_num != IRQ_SPI1_VECTOR) {
			//CLOGW("SPI1 device context has been tampered illegally!!\n");
			return NULL;
		}
	} else {
		return NULL;
	}

    return (SPI_DEV *)spi_dev;
}

static inline uint16_t
spi_get_txfifo_size(SPI_DEV *spi)
{
    uint8_t size_shift = ((spi->reg->CONFIG >> 4) & 0x3);
    return (2 << size_shift);
}

static inline uint16_t
spi_get_rxfifo_size(SPI_DEV *spi)
{
    uint8_t size_shift = ((spi->reg->CONFIG) & 0x3);
    return (2 << size_shift);
}

static inline void
spi_polling_spiactive(SPI_DEV *spi)
{
    while ((spi->reg->STATUS) & 0x1)
        ;
}

// export SPI API function: SPI_GetVersion
CSK_DRIVER_VERSION
SPI_GetVersion()
{
    return spi_driver_version;
}

static uint32_t SPI_CLK(SPI_DEV *spi)
{
    uint32_t N, M;

    assert(spi != NULL);
    N = spi->ext_clk->bit.DIV_SPI_CLK_N;
    M = spi->ext_clk->bit.DIV_SPI_CLK_M;
    assert(N != 0 && M != 0);

#if (IC_BOARD == 0)
    return (CPUFREQ() * N / M); //48MHz
#else
    // it seems that max SPI clock is 60MHz (limited by SYSPLL_USB output)
    return CRM_GetSrcFreq(CRM_IpSrcSysPllUsb) * N / M;
#endif

}

// export SPI API function: SPI_Initialize
int32_t
SPI_Initialize(void *spi_dev, CSK_SPI_SignalEvent_t cb_event, uint32_t usr_param)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (spi->info->flags & SPI_FLAG_INITIALIZED)
        return CSK_DRIVER_OK;

    // initialize SPI run-time resources
    spi->info->cb_event = cb_event;
    spi->info->usr_param = usr_param;

//    spi->info->status.busy = 0U;
//    spi->info->status.data_lost = 0U;
//    spi->info->status.mode_fault = 0U;
    spi->info->status.all = 0U;

    spi->info->xfer.rx_buf = 0U;
    spi->info->xfer.tx_buf = 0U;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.dma_tx_done = 1; // initially done
    spi->info->xfer.dma_rx_done = 1; // initially done

    spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
    spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
    spi->info->data_bits = 8; //default 8 bit xfer on SPI cable
    spi->info->dma_width_shift = 0; // default DMA_WIDTH_BYTE

    //spi->info->txrx_mode = 0U;
    spi->info->txrx_mode = CSK_SPI_TXIO_AUTO | CSK_SPI_RXIO_AUTO;
    spi->info->flags = SPI_FLAG_INITIALIZED;  // SPI is initialized

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Uninitialize
int32_t
SPI_Uninitialize(void *spi_dev)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    //TODO: any other clean operations?

    // Power off SPI if Powered on
    if (spi->info->flags & SPI_FLAG_POWERED)
        SPI_PowerControl(spi_dev, CSK_POWER_OFF);

    spi->info->flags = 0U; // SPI is uninitialized
    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_PowerControl
int32_t
SPI_PowerControl(void *spi_dev, CSK_POWER_STATE state)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    switch (state) {
    case CSK_POWER_OFF:
        // disable SPI IRQ
        disable_IRQ(spi->irq_num);

        // abort current TX/RX and clean state if configured
        if (spi->info->flags & SPI_FLAG_CONFIGURED)
            spi_abort_transfer(spi, false);

        // DMA uninitialize
        dma_uninitialize();

        // reset SPI and TX/RX FIFOs
        //spi->reg->CTRL |= (TXFIFORST | RXFIFORST | SPIRST);
        spi->reg->CTRL |= SPIRST;

        // disable SPI clock here
        spi->ext_clk->all &= ~SPI_CLK_ENABLE;

        //spi->info->txrx_mode = 0U;
        spi->info->flags &= ~(SPI_FLAG_POWERED | SPI_FLAG_CONFIGURED);

        break;

    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if ((spi->info->flags & SPI_FLAG_INITIALIZED) == 0U)
            return CSK_DRIVER_ERROR;

        if ((spi->info->flags & SPI_FLAG_POWERED) != 0U)
            return CSK_DRIVER_OK;

        // enable SPI clock here
        {
    #if (IC_BOARD == 0)
        // FIXME: SPI CLK M = 1 CANNOT work!!
        spi->ext_clk->all = SPI_CLK_ENABLE | SPI_CLK_M(1) | SPI_CLK_N(1) | SPI_CLK_LD;
        //spi->ext_clk->all = SPI_CLK_ENABLE | SPI_CLK_M(2) | SPI_CLK_N(1) | SPI_CLK_LD;
        //spi->ext_clk->all = SPI_CLK_ENABLE | SPI_CLK_M(4) | SPI_CLK_N(1) | SPI_CLK_LD;
    #else
        uint32_t divNM, clk_out;
        clk_out = 60000; //KHz, FIXME: 60,000KHz? 100,000KHz?
        divNM = eval_freq_divNM(SPI_CLK_N_MAX, SPI_CLK_M_MAX, CRM_GetSrcFreq(CRM_IpSrcSysPllUsb)/1000, clk_out, NULL);
        assert(divNM != 0);
        spi->ext_clk->all = SPI_CLK_ENABLE | SPI_CLK_M(divNM & 0xFFFF) | SPI_CLK_N((divNM >> 16) & 0xFFFF) | SPI_CLK_LD;
    #endif
        }

        // DMA initialize
        dma_initialize();

        // reset SPI and TX/RX FIFOs
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST | SPIRST);
        while(spi->reg->CTRL & (TXFIFORST | RXFIFORST | SPIRST));

        spi->txfifo_depth = spi_get_txfifo_size(spi);
        spi->rxfifo_depth = spi_get_rxfifo_size(spi);

        // set TX FIFO threshold and RX FIFO threshold to half of TX/RX FIFO depth
        spi->reg->CTRL |= (TXTHRES(spi->txfifo_depth / 2) | RXTHRES(spi->rxfifo_depth / 2));
        spi->info->tx_bsize_shift = ITEMS_TO_BSIZE(spi->txfifo_depth / 2); //DMA_BSIZE_4;
        spi->info->rx_bsize_shift = ITEMS_TO_BSIZE(spi->rxfifo_depth / 2); //DMA_BSIZE_4;

        // clear SPI run-time resources
        spi->info->status.all = 0U;
        spi->info->xfer.rx_buf = 0U;
        spi->info->xfer.tx_buf = 0U;
        spi->info->xfer.rx_cnt = 0U;
        spi->info->xfer.tx_cnt = 0U;

        //spi->info->txrx_mode = 0U;
        spi->info->flags |= SPI_FLAG_POWERED;   // SPI is powered
        register_ISR(spi->irq_num , spi->irq_handler, NULL); // register ISR
        enable_IRQ(spi->irq_num); // enable SPI IRQ

        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }
    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Send
int32_t
SPI_Send(void *spi_dev, const void *data, uint32_t num)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    int32_t stat;

    if ((data == NULL) || (num == 0U) || (num > MAX_TRANCNT))
        return CSK_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return CSK_DRIVER_ERROR;

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    // set busy flag
    spi->info->status.bit.busy = 1U;
    spi->info->status.bit.data_unf = 0;
    spi->info->status.bit.tx_mode = NO_IO;

    spi->info->xfer.tx_buf = data;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_SEND;

    // wait prior transfer finish
    spi_polling_spiactive(spi);

    // set transfer mode to write only and transfer count for write data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    // DMA mode
    bool xfer_started = false;
    while ( TX_DMA(spi->info->txrx_mode) ) { // DMA TX
        bool dma_only = TX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_tx_done = 0;

        // enable TX DMA
        //spi->reg->CTRL |= TXDMAEN;

        // select some free DMA channel
        spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
        spi->info->tx_dyn_dma_ch = dma_channel_select(
                                        &spi->info->tx_dyn_dma_ch,
                                        spi->dma_tx.cb_event,
                                        spi->dma_tx.usr_param,
                                        DMA_CACHE_SYNC_SRC);
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

        uint32_t control, config_low, config_high;
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel);
        //config_high = DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel);

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
                            num, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->tx_dyn_dma_ch, false);
            spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

        // enable TX DMA
        spi->reg->CTRL |= TXDMAEN;

        // enable interrupts
        spi->reg->INTREN = SPI_ENDINT;

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)
        // enable interrupts
        spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);

        spi->info->status.bit.tx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA nor PIO is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // enable TX FIFO underrun interrupt when slave mode
    // set the Ready bit in the SPI Slave Status Register
    if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        spi->reg->INTREN |= SPI_TXFIFOURINT;
        spi->reg->SLVST |= SLVST_READY;
    }

    //while(SPI_TXFIFO_EMPTY(spi)); //TEST ONLY!

    // trigger transfer when SPI master mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER) {
        // wait until there's at least 1 data in the TX FIFO??
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi)) ;
        spi->reg->CMD = 0;
    }

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Receive
int32_t
SPI_Receive(void *spi_dev, void *data, uint32_t num)
{
    int32_t stat;
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((data == NULL) || (num == 0U) || (num > MAX_TRANCNT))
        return CSK_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return CSK_DRIVER_ERROR;

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    // set busy flag
    spi->info->status.bit.busy = 1U;
    spi->info->status.bit.data_ovf = 0U;
    spi->info->status.bit.rx_mode = NO_IO;

    spi->info->xfer.rx_buf = data;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.cur_op = SPI_RECEIVE;

    // wait prior transfer finish
    spi_polling_spiactive(spi);

    // set transfer mode to read only and transfer count for read data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    // DMA mode
    bool xfer_started = false;
    while ( RX_DMA(spi->info->txrx_mode) ) { // DMA RX
        bool dma_only = RX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

        // enable RX DMA
        //spi->reg->CTRL |= RXDMAEN;

        // select some free DMA channel
        spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        spi->info->rx_dyn_dma_ch = dma_channel_select(
                                        &spi->info->rx_dyn_dma_ch,
                                        spi->dma_rx.cb_event,
                                        spi->dma_rx.usr_param,
                                        DMA_CACHE_SYNC_DST);
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

        uint32_t control, config_low, config_high;
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel);

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
                            (uint32_t) spi->info->xfer.rx_buf,
                            num, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->rx_dyn_dma_ch, false);
            spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

        // enable RX DMA
        spi->reg->CTRL |= RXDMAEN;

        // enable interrupts
        spi->reg->INTREN = SPI_ENDINT;

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)
        // enable interrupts
        spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);

        spi->info->status.bit.rx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA nor PIO is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // enable RX FIFO overrun interrupt when slave mode
    // set slave cmd interrupt
	if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
	    CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
        //spi->reg->INTREN |= SPI_RXFIFOORINT;
        //spi->reg->INTREN |= SPI_SLVCMD;
	    spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
	}

    // trigger transfer when SPI master mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER) {
        spi->reg->CMD = 0;
    }

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Transfer
int32_t
SPI_Transfer(void *spi_dev, const void *data_out, void *data_in, uint32_t num)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    int32_t stat;

    if ((data_out == NULL) || (data_in == NULL) || (num == 0U) || (num > MAX_TRANCNT))
        return CSK_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return CSK_DRIVER_ERROR;

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    // set busy flag
    spi->info->status.bit.busy = 1U;
    spi->info->status.bit.data_unf = 0U;
    spi->info->status.bit.data_ovf = 0U;
    spi->info->status.bit.tx_mode = NO_IO;
    spi->info->status.bit.rx_mode = NO_IO;

    spi->info->xfer.rx_buf = data_in;
    spi->info->xfer.tx_buf = data_out;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_TRANSFER;

    // wait prior transfer finish
    spi_polling_spiactive(spi);

    // set transfer mode to write and read at the same time and transfer count for write/read data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num) |
                        (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    uint32_t control, config_low, config_high;
    bool xfer_started, dma_only;

    // DMA mode
    xfer_started = false;
    while ( TX_DMA(spi->info->txrx_mode) ) { // DMA TX
        dma_only = TX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_tx_done = 0;

        // enable TX DMA
        //spi->reg->CTRL |= TXDMAEN;

        // select some free DMA channel
        spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
        dma_channel_select(&spi->info->tx_dyn_dma_ch,
                            spi->dma_tx.cb_event,
                            spi->dma_tx.usr_param,
                            DMA_CACHE_SYNC_SRC);
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel);

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
                            num, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->tx_dyn_dma_ch, false);
            spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

        // enable TX DMA
        spi->reg->CTRL |= TXDMAEN;

        // enable interrupts
        spi->reg->INTREN = SPI_ENDINT;

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)
        // enable interrupts
        spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);

        spi->info->status.bit.tx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA TX nor PIO TX is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // enable TX FIFO underrun interrupt when slave mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        spi->reg->INTREN |= SPI_TXFIFOURINT;
    }

    xfer_started = false;
    while ( RX_DMA(spi->info->txrx_mode) ) { // DMA RX
        dma_only = RX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

        // enable RX DMA
        //spi->reg->CTRL |= RXDMAEN;

        // select some free DMA channel
        spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        dma_channel_select(&spi->info->rx_dyn_dma_ch,
                            spi->dma_rx.cb_event,
                            spi->dma_rx.usr_param,
                            DMA_CACHE_SYNC_DST);
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel);

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
                            (uint32_t) spi->info->xfer.rx_buf,
                            num, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->rx_dyn_dma_ch, false);
            spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

        // enable RX DMA
        spi->reg->CTRL |= RXDMAEN;

        // enable interrupts
        //spi->reg->INTREN = SPI_ENDINT;

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)
        // enable interrupts
        //spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);
        spi->reg->INTREN |= SPI_RXFIFOINT;

        spi->info->status.bit.rx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA RX nor PIO RX is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // enable RX FIFO overrun interrupt when slave mode
    // set the Ready bit in the SPI Slave Status Register
    if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
        spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
        spi->reg->SLVST |= SLVST_READY;
    }

    //while(SPI_TXFIFO_EMPTY(spi)); //TEST ONLY!

    // trigger transfer when SPI master mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER) {
        // wait until there's at least 1 data in the TX FIFO?
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi));
        spi->reg->CMD = 0;
    }

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_GetDataCount
uint32_t
SPI_GetDataCount(void *spi_dev)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return 0U;
    }

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return 0U;

    switch (spi->info->xfer.cur_op) {
    case SPI_SEND:
    case SPI_TRANSFER:
        if (spi->info->status.bit.tx_mode == DMA_IO && !spi->info->xfer.dma_tx_done)
            spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        return (spi->info->xfer.tx_cnt);

    case SPI_RECEIVE:
        if (spi->info->status.bit.rx_mode == DMA_IO && !spi->info->xfer.dma_rx_done)
            spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
        return (spi->info->xfer.rx_cnt);

    default:
        return CSK_DRIVER_OK;
    }
}


static void spi_abort_transfer(SPI_DEV *spi, bool keep_state)
{
    assert(spi != NULL);

    // if busy (Either TX or RX is ongoing), abort 1
    if (spi->info->status.bit.busy) {
        // disable SPI interrupts
        spi->reg->INTREN = 0;

        //clear the Ready bit in the SPI Slave Status Register
        if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
            spi->reg->SLVST &= ~SLVST_READY;
        }

        if (spi->info->status.bit.tx_mode == DMA_IO) {
            if (!spi->info->xfer.dma_tx_done) {
                spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
                dma_channel_disable(spi->info->tx_dyn_dma_ch, true);
                spi->info->xfer.dma_tx_done = 1;
            }
        }

        if (spi->info->status.bit.rx_mode == DMA_IO) {
            if (!spi->info->xfer.dma_rx_done) {
                spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
                dma_channel_disable(spi->info->rx_dyn_dma_ch, true);
                spi->info->xfer.dma_rx_done = 1;
            }
        }
    } // end if busy

    if (!keep_state) {
        // disable DMA and clear TX/RX FIFOs
        spi->reg->CTRL &= ~(TXDMAEN | RXDMAEN);
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST);

        // clear SPI run-time resources
         spi->info->status.all = 0U;
       //spi->info->status.bit.busy = 0U;
        //spi->info->status.bit.tx_mode = NO_IO;
        //spi->info->status.bit.rx_mode = NO_IO;

        spi->info->xfer.rx_buf = 0U;
        spi->info->xfer.tx_buf = 0U;
        spi->info->xfer.rx_cnt = 0U;
        spi->info->xfer.tx_cnt = 0U;
    } //end if clean_state
}

// meaningful only in SPI master mode
static bool spi_set_bus_speed(SPI_DEV *spi, uint32_t arg)
{
    assert(spi != NULL && arg != 0);

    uint32_t sclk_div, cs2sclk;
    sclk_div = (SPI_CLK(spi) / (2 * arg)) - 1;
    if (sclk_div >= 0xFF) {
        LOGD("%s: CANNOT support SCLK = %d (TOO LOW)! ", __func__, arg);
        return false;
    }

    //cs2sclk = 0;
    cs2sclk = (0x1 << 12); // 2 bits, can be 0x0 ~ 0x3
    //cs2sclk = (0x2 << 12);
    //cs2sclk = (0x3 << 12);

    //spi->reg->TIMING &= ~0xff;
    //spi->reg->TIMING |= sclk_div & 0xff;
    spi->reg->TIMING &= ~0x30ffUL;
    spi->reg->TIMING |= (sclk_div & 0xff) | (cs2sclk & 0x3000);

    //LOGD("%s: sclk_div = %d\r\n", __func__, (spi->reg->TIMING & 0xFF));
    LOGD("%s: sclk_div = %d, cs2sclk = %d\r\n",
        __func__, (spi->reg->TIMING & 0xFF), ((spi->reg->TIMING >> 12) & 0x3));
    return true;
}

static inline int32_t spi_get_bus_speed(SPI_DEV *spi)
{
    assert(spi != NULL);
    uint32_t sclk_div;
    sclk_div = spi->reg->TIMING & 0xff;
    return (SPI_CLK(spi) / ((sclk_div + 1) * 2));
}

// export SPI API function: SPI_Control
int32_t
SPI_Control(void *spi_dev, uint32_t control, uint32_t arg)
{
    uint32_t val;
    SPI_DEV *spi = safe_spi_dev(spi_dev);

    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(spi->info->flags & SPI_FLAG_POWERED))
        return CSK_DRIVER_ERROR;

    // exclusive MISC OP
    switch (control & CSK_SPI_EXCL_OP_Msk) {
        // NO MISC OP
        case CSK_SPI_EXCL_OP_UNSET:
            break;

        // abort SPI transfer
        case CSK_SPI_ABORT_TRANSFER:
            spi_abort_transfer(spi, arg);
            return CSK_DRIVER_OK;

        // set bus speed in bps; arg = value
        case CSK_SPI_SET_BUS_SPEED:
            if (arg == 0U)
                return CSK_DRIVER_ERROR;
            return (spi_set_bus_speed(spi, arg) ? CSK_DRIVER_OK : CSK_DRIVER_ERROR);

        // get bus speed in bps
        case CSK_SPI_GET_BUS_SPEED:
            return spi_get_bus_speed(spi);

        default:
            return CSK_DRIVER_ERROR_UNSUPPORTED;
    } // end exclusive MISC_OP

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    // SPI Mode
    switch (control & CSK_SPI_MODE_Msk) {
    // Keep mode unchanged
    case CSK_SPI_MODE_UNSET:
        break;

    // SPI master (output on MOSI, input on MISO); arg = bus speed in bps
    case CSK_SPI_MODE_MASTER:
        // set master mode and disable data merge mode
        spi->reg->TRANSFMT &= ~(SPI_MERGE | SPI_SLAVE);
        spi->info->txrx_mode &= ~CSK_SPI_MODE_Msk;
        spi->info->txrx_mode |= CSK_SPI_MODE_MASTER;
        spi->info->flags |= SPI_FLAG_CONFIGURED;
        if (arg != 0) {
            if (!spi_set_bus_speed(spi, arg))
                return CSK_DRIVER_ERROR_PARAMETER;
        }
        break;

    // SPI slave (output on MISO, input on MOSI)
    case CSK_SPI_MODE_SLAVE:
        // set slave mode and disable data merge mode
        spi->reg->TRANSFMT &= ~SPI_MERGE;
        spi->reg->TRANSFMT |= SPI_SLAVE;
        spi->info->txrx_mode &= ~CSK_SPI_MODE_Msk;
        spi->info->txrx_mode |= CSK_SPI_MODE_SLAVE;
        spi->info->flags |= SPI_FLAG_CONFIGURED;
        break;

    default:
        return CSK_SPI_ERROR_MODE;
    } // end SPI Mode

    // SPI TX IO
    val = control & CSK_SPI_TXIO_Msk;
    switch (val) {
    case CSK_SPI_TXIO_UNSET:
        break;
    case CSK_SPI_TXIO_DMA:
    case CSK_SPI_TXIO_PIO:
    case CSK_SPI_TXIO_BOTH:
        spi->info->txrx_mode &= ~CSK_SPI_TXIO_Msk;
        spi->info->txrx_mode |= val;
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // SPI RX IO
    val = control & CSK_SPI_RXIO_Msk;
    switch (val) {
    case CSK_SPI_RXIO_UNSET:
        break;
    case CSK_SPI_RXIO_DMA:
    case CSK_SPI_RXIO_PIO:
    case CSK_SPI_RXIO_BOTH:
        spi->info->txrx_mode &= ~CSK_SPI_RXIO_Msk;
        spi->info->txrx_mode |= val;
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // SPI frame format
    switch (control & CSK_SPI_FRAME_FORMAT_Msk) {
    case CSK_SPI_FRM_FMT_UNSET:
        break;
    case CSK_SPI_CPOL0_CPHA0:
        spi->reg->TRANSFMT &= ~(SPI_CPOL | SPI_CPHA);
        break;
    case CSK_SPI_CPOL0_CPHA1:
        spi->reg->TRANSFMT &= ~SPI_CPOL;
        spi->reg->TRANSFMT |= SPI_CPHA;
        break;
    case CSK_SPI_CPOL1_CPHA0:
        spi->reg->TRANSFMT |= SPI_CPOL;
        spi->reg->TRANSFMT &= ~SPI_CPHA;
        break;
    case CSK_SPI_CPOL1_CPHA1:
        spi->reg->TRANSFMT |= SPI_CPOL;
        spi->reg->TRANSFMT |= SPI_CPHA;
        break;
//    case CSK_SPI_TI_SSI:
//    case CSK_SPI_MICROWIRE:
    default:
        return CSK_SPI_ERROR_FRAME_FORMAT;
    }

    // SPI Data Bits
    val = ((control & CSK_SPI_DATA_BITS_Msk) >> CSK_SPI_DATA_BITS_Pos);
    if (val != 0) {
        if ((val < 1U) || (val > 32U))
            return CSK_SPI_ERROR_DATA_BITS;
        spi->info->data_bits = val;
        spi->reg->TRANSFMT |= DATA_BITS(val);
    } else {
        if (spi->info->data_bits == 0) {
            //CLOGW("%s: DATA_BITS NOT Set!!\n", __func__);
            return CSK_DRIVER_ERROR_PARAMETER;
        }
    }

    // DMA Src/Dst Width
    if (spi->info->data_bits <= 8)
        spi->info->dma_width_shift = DMA_WIDTH_BYTE; //0 = byte
    else if (spi->info->data_bits <= 16)
        spi->info->dma_width_shift = DMA_WIDTH_HALFWORD; //1 = halfword
    else if (spi->info->data_bits <= 32)
        spi->info->dma_width_shift = DMA_WIDTH_WORD; //2 = word

    // SPI Bit Order
    switch (control & CSK_SPI_BIT_ORDER_Msk) {
    case CSK_SPI_BIT_ORDER_UNSET:
        break;
    case CSK_SPI_LSB_MSB:
        spi->reg->TRANSFMT |= SPI_LSB;
        break;
    case CSK_SPI_MSB_LSB:
        spi->reg->TRANSFMT &= ~SPI_LSB;
        break;
    default:
        return CSK_SPI_ERROR_BIT_ORDER;
    }

//    //FIXME: Enable / Disable command phase (NO Cmd + dummy 2 leading bytes for SPI slave if disabled)
//    if (spi->info->txrx_mode & CSK_SPI_MODE_SLAVE)
//        spi->reg->TRANSCTRL |= (0x1 << 30); // Enable command phase, original default setting
//    else if (spi->info->txrx_mode & CSK_SPI_MODE_MASTER)
//        spi->reg->TRANSCTRL &= ~(0x1 << 30); // Disable command phase, original default setting

//    // set TX FIFO threshold and RX FIFO threshold to half of TX/RX FIFO depth
//    spi->reg->CTRL |= (TXTHRES(spi->txfifo_depth / 2) | RXTHRES(spi->rxfifo_depth / 2));
//    //spi->info->max_txfifo_refill = spi->txfifo_depth / 2;
//    spi->info->tx_bsize_shift = ITEMS_TO_BSIZE(spi->txfifo_depth / 2); //DMA_BSIZE_4;
//    spi->info->rx_bsize_shift = ITEMS_TO_BSIZE(spi->rxfifo_depth / 2); //DMA_BSIZE_4;

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_GetStatus
int32_t
SPI_GetStatus(void *spi_dev, CSK_SPI_STATUS *status)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL || status == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    status->all = spi->info->status.all;
    return CSK_DRIVER_OK;
}


static void
spi_dma_tx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param)
{
    uint8_t event_type = event_info & 0xFF;
    SPI_DEV *spi = (SPI_DEV *)usr_param;

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.tx_cnt = xfer_bytes;
        spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        spi->info->xfer.dma_tx_done = 1;
        //spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;

        break;

    case DMA_EVENT_ERROR:
        //TODO: do something here?
        break;

    default:
        break;
    }
}


static void
spi_dma_rx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param)
{
    uint8_t event_type = event_info & 0xFF;
    SPI_DEV *spi = (SPI_DEV *)usr_param;

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.rx_cnt = xfer_bytes;
        spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
        spi->info->xfer.dma_rx_done = 1;
        //spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
        break;

    case DMA_EVENT_ERROR:
        //TODO: do something here?
        break;

    default:
        break;
    }
}


void
spi_irq_handler(SPI_DEV *spi)
{
    uint32_t i, status; // j
    uint32_t data = 0;
    uint32_t rx_num = 0;
    uint32_t event = 0;

    // read status register
    status = spi->reg->INTRST;

    if ((status & SPI_RXFIFOORINT) || (status & SPI_TXFIFOURINT)) {
        // TX FIFO underrun or RX FIFO overrun interrupt status
        //spi->info->status.data_lost = 1U;
        if (status & SPI_RXFIFOORINT)
            spi->info->status.bit.data_ovf = 1U;
        if (status & SPI_TXFIFOURINT)
            spi->info->status.bit.data_unf = 1U;

        event |= CSK_SPI_EVENT_DATA_LOST;
    }
    if (status & SPI_SLVCMD) {
//    	spi->reg->INTRST &= ~SPI_SLVCMD;
    	switch(spi->reg->CMD) {
    	case 0x05:
    	case 0x15:
    	case 0x25:
    		event |= CSK_SPI_EVENT_SLV_CMD_S;
    		break;
    	case 0x0b:
    	case 0x0c:
    	case 0x0e:
    		event |= CSK_SPI_EVENT_SLV_CMD_R;
    		break;
    	case 0x51:
    	case 0x52:
    	case 0x54:
    		event |= CSK_SPI_EVENT_SLV_CMD_W;
    		break;
    	default:
    		break;
    	}
    }

    if (status & SPI_RXFIFOINT) {
        // get number of valid entries in the RX FIFO
        //rx_num = (spi->reg->STATUS >> 8) & 0x1f;
        i = SPI_RXFIFO_ENTRIES(spi);

        while (i > 0) {
            if (spi->info->xfer.rx_cnt >= spi->info->xfer.req_rx_cnt) {
                //LOGD("BSD: Warning!! More data received than requested!\n");
                break;
            }

            data = spi->reg->DATA;

            // handle the data frame format
            if (spi->info->data_bits <= 8) {
                uint8_t *rx_buf8 = (uint8_t *)spi->info->xfer.rx_buf;
                rx_buf8[spi->info->xfer.rx_cnt++] = data & 0xff;

            } else if (spi->info->data_bits <= 16) {
                uint16_t *rx_buf16 = (uint16_t *)spi->info->xfer.rx_buf;
                rx_buf16[spi->info->xfer.rx_cnt++] = data & 0xffff;
            } else {
                uint32_t *rx_buf32 = (uint32_t *)spi->info->xfer.rx_buf;
                rx_buf32[spi->info->xfer.rx_cnt++] = data;
            }

            // auto decrease, re-evaluate i if down to 0
            if (--i == 0)
                i = SPI_RXFIFO_ENTRIES(spi);
        } // end while i
    }

    if (status & SPI_TXFIFOINT) {
        i = SPI_TXFIFO_ENTRIES(spi);
        while ( i < spi->txfifo_depth) {

//            // auto increase i, re-evaluate i if up to TXFIFO depth
//            if (++i == spi->txfifo_depth)
//                i = SPI_TXFIFO_ENTRIES(spi);

            data = 0;
            if (spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
                // handle the data frame format
                if (spi->info->data_bits <= 8) { // data bits = 1....8
                    uint8_t *tx_buf8 = (uint8_t *)spi->info->xfer.tx_buf;
                    data = tx_buf8[spi->info->xfer.tx_cnt++];
                } else if (spi->info->data_bits <= 16) { // data bits = 9....16
                    uint16_t *tx_buf16 = (uint16_t *)spi->info->xfer.tx_buf;
                    data = tx_buf16[spi->info->xfer.tx_cnt++];
                } else { // data bits = 17....32
                    uint32_t *tx_buf32 = (uint32_t *)spi->info->xfer.tx_buf;
                    data = tx_buf32[spi->info->xfer.tx_cnt++];
                }
                spi->reg->DATA = data;
                i++; //BSD:

            } else {
                spi->reg->INTREN &= ~SPI_TXFIFOINT;
                break;
            }
        }
    }


    if (status & SPI_ENDINT) {
        volatile uint32_t count;

        // disable SPI interrupts
        spi->reg->INTREN = 0;

        //BSD: clear the Ready bit in the SPI Slave Status Register
        if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
            spi->reg->SLVST &= ~SLVST_READY;
        }

        if (spi->info->xfer.cur_op != SPI_RECEIVE) {
            if (spi->info->status.bit.tx_mode == DMA_IO && !spi->info->xfer.dma_tx_done) {
                spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
                //NOTE: when CS is changed to LOW (ENDINT interrupt is triggered then) and TX FIFO is full,
                //  the data may remain in DMA channel FIFO and therefore never drain, so DON'T wait here!!
                dma_channel_disable(spi->info->tx_dyn_dma_ch, false); // true

                //FIXME: still wait some cycles?
                //for (i=0; i<50; i++) { // 100
                //    count = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
                //}
                count = dma_channel_get_count(spi->info->tx_dyn_dma_ch);

                LOGD("BSD: dma tx data count: %d => %d\n", spi->info->xfer.tx_cnt, count);
                spi->info->xfer.tx_cnt = count;
                spi->info->xfer.dma_tx_done = 1;
            }

            // Check if there is remaining data left in TX FIFO!!
            //i = (spi->reg->STATUS >> 16) & 0x1f;
            i = SPI_TXFIFO_ENTRIES(spi);
            if (i > 0) {
                // The Data remained in TX FIFO is deducted from the total xfer count
                spi->info->xfer.tx_cnt -= i;
                LOGD("%s: Warning!! %d Data items remain in TX FIFO!\n",
                        __func__, i);
            }
            //FIXME:
            if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
                //count = (spi->reg->SLVDATACNT >> 16) & 0x1FF;
                count = SLV_WRCNT(spi);
                CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
                LOGD("%s: SLVDATACNT.WCnt = %d Data\n", __func__, count);
                spi->info->xfer.tx_cnt = count;
            }

            if (spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
                LOGD("%s: Warning!! %d Data items have not been sent!\n",
                        __func__, spi->info->xfer.req_tx_cnt - spi->info->xfer.tx_cnt);
            }
        }

        if (spi->info->xfer.cur_op != SPI_SEND) {
            if (spi->info->status.bit.rx_mode == DMA_IO && !spi->info->xfer.dma_rx_done) {
                spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
                //wait, for data may be in SPI RX FIFO or DMA channel FIFO
                dma_channel_disable(spi->info->rx_dyn_dma_ch, true);

                //FIXME: still wait some cycles?
                //for (i=0; i<50; i++) { //TODO: decrease to 20? 50? 100?
                //    count = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
                //}
                count = dma_channel_get_count(spi->info->rx_dyn_dma_ch);

                // LOGD("BSD: dma rx data count: %d => %d\n", spi->info->xfer.rx_cnt, count);
                spi->info->xfer.rx_cnt = count;

                spi->info->xfer.dma_rx_done = 1;
            }

            // get number of valid entries in the RX FIFO
            //rx_num = (spi->reg->STATUS >> 8) & 0x1f;
            rx_num = SPI_RXFIFO_ENTRIES(spi);
            if (rx_num) { //TODO: debug only!!
                LOGD("BSD: %d entries left in SPI RXFIFO\n", rx_num);
            }

            for (i = rx_num; i > 0; i--) {
                if (spi->info->xfer.rx_cnt >= spi->info->xfer.req_rx_cnt) {
                    //CLOGW("BSD: Warning!! More data received than requested!\n");
                    break;
                }
                data = spi->reg->DATA;
                // handle the data frame format
                if (spi->info->data_bits <= 8) {
                    uint8_t *rx_buf8 = (uint8_t *)spi->info->xfer.rx_buf;
                    rx_buf8[spi->info->xfer.rx_cnt++] = data & 0xff;
                } else if (spi->info->data_bits <= 16){
                    uint16_t *rx_buf16 = (uint16_t *)spi->info->xfer.rx_buf;
                    rx_buf16[spi->info->xfer.rx_cnt++] = data & 0xffff;
               } else if (spi->info->data_bits <= 32){
                    uint32_t *rx_buf32 = (uint32_t *)spi->info->xfer.rx_buf;
                    rx_buf32[spi->info->xfer.rx_cnt++] = data;
                }
            } //end for
            //FIXME:
            if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
                //count = spi->reg->SLVDATACNT & 0x1FF;
                count = SLV_RDCNT(spi);
                CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
                LOGD("%s: SLVDATACNT.RCnt = %d Data\n", __func__, count);
                spi->info->xfer.rx_cnt = count;
            }
        }

        // clear TX/RX FIFOs
        spi->reg->CTRL &= ~(TXDMAEN | RXDMAEN);
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST);

        spi->info->status.bit.busy = 0U;
        spi->info->status.bit.tx_mode = NO_IO;
        spi->info->status.bit.rx_mode = NO_IO;
        event |= CSK_SPI_EVENT_TRANSFER_COMPLETE;
    }

    // clear interrupt status
    spi->reg->INTRST = status;
    // make sure "write 1 clear" take effect before iret
    spi->reg->INTRST;

    if ((spi->info->cb_event != NULL) && (event != 0))
        spi->info->cb_event(event, spi->info->usr_param);

}
