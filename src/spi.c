/*
 * Copyright (c) 2020-2025 ChipSky Technology
 * All rights reserved.
 *
 */

#include "spi.h"
#include "ClockManager.h" // for CRM_GetSrcFreq() etc.
#include <assert.h>

#ifdef CSK6001_MPW
#define VENUS_V1_MPW    1
#define VENUS_V2_MP     0
#else
#define VENUS_V1_MPW    0
#define VENUS_V2_MP     1
#endif

#define DEBUG_LOG   1 // 1
#if DEBUG_LOG
#define LOGD(format, ...)   ((void)0)
#else
#define LOGD(format, ...)   ((void)0)
#endif // DEBUG_LOG

#define SPICLK_OWNER_SYS        0
#define SPICLK_OWNER_SELF       1
#define SPICLK_OWNER_OTHER      2

#define SPICLK_OWNER        SPICLK_OWNER_SELF // SPICLK_OWNER_SYS
<<<<<<< HEAD
#define SPICLK_CNST_FREQ    24000 //FIXME: other constant frequency?
=======
#define SPICLK_CNST_FREQ    24000000 //FIXME: other constant frequency?
>>>>>>> feature/zephyr

#define CSK_SPI_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)

// driver version
static const
CSK_DRIVER_VERSION spi_driver_version = { CSK_SPI_API_VERSION, CSK_SPI_DRV_VERSION };

static void spi_dma_tx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param);
static void spi_dma_rx_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param);
static void spi_irq_handler(SPI_DEV *spi);
static void spi_abort_transfer(SPI_DEV *spi, bool keep_state);

//------------------------------------------------------------------------------------
// SPI0
static SPI_DEV spi0_dev;
<<<<<<< HEAD
static void spi0_irq_handler(void) { spi_irq_handler(&spi0_dev); }

static SPI_INFO spi0_info = { 0 };
static SPI_DEV spi0_dev = {
=======
_FAST_FUNC_RO static void spi0_irq_handler(void) { spi_irq_handler(&spi0_dev); }

_FAST_DATA_ZI static SPI_INFO spi0_info = { 0 };
_FAST_DATA_VI static SPI_DEV spi0_dev = {
>>>>>>> feature/zephyr
        CSK_SPI0,
        CSK_SPI0_CLK,
        CSK_SPI_TXFIFO_DEPTH,
        CSK_SPI_RXFIFO_DEPTH,
        IRQ_SPI0_VECTOR,
<<<<<<< HEAD
=======
        0,
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
static void spi1_irq_handler(void) { spi_irq_handler(&spi1_dev); }

static SPI_INFO spi1_info = { 0 };
static SPI_DEV spi1_dev = {
=======
_FAST_FUNC_RO static void spi1_irq_handler(void) { spi_irq_handler(&spi1_dev); }

_FAST_DATA_ZI static SPI_INFO spi1_info = { 0 };
_FAST_DATA_VI static SPI_DEV spi1_dev = {
>>>>>>> feature/zephyr
        CSK_SPI1,
        CSK_SPI1_CLK,
        CSK_SPI_TXFIFO_DEPTH,
        CSK_SPI_RXFIFO_DEPTH,
        IRQ_SPI1_VECTOR,
<<<<<<< HEAD
=======
        1,
>>>>>>> feature/zephyr
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

<<<<<<< HEAD

//------------------------------------------------------------------------------------
static SPI_DEV * safe_spi_dev(void *spi_dev)
=======
//------------------------------------------------------------------------------------
_FAST_FUNC_RO static SPI_DEV * safe_spi_dev(void *spi_dev)
>>>>>>> feature/zephyr
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

<<<<<<< HEAD
=======
//Get SPI device index, return 0, 1,..., and 0xFF if not existing.
uint8_t SPI_Index(void *spi_dev)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL)
        return 0xFF;
    return (spi->spi_idx & 0xFF);
}

>>>>>>> feature/zephyr
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

<<<<<<< HEAD
=======
static inline void
spi_set_tx_fifo_threshold(SPI_DEV *spi, uint8_t thres)
{
    uint32_t val = spi->reg->CTRL;
    uint32_t tx_val = TXTHRES(thres);
    if ((val & TXTHRES_MASK) != tx_val) {
        val &= ~TXTHRES_MASK;
        val |= tx_val;
        spi->reg->CTRL = val;
    }
}

static inline void
spi_set_rx_fifo_threshold(SPI_DEV *spi, uint8_t thres)
{
    uint32_t val = spi->reg->CTRL;
    uint32_t rx_val = RXTHRES(thres);
    if ((val & RXTHRES_MASK) != rx_val) {
        val &= ~RXTHRES_MASK;
        val |= rx_val;
        spi->reg->CTRL = val;
    }
}

>>>>>>> feature/zephyr
// export SPI API function: SPI_GetVersion
CSK_DRIVER_VERSION
SPI_GetVersion()
{
    return spi_driver_version;
}

static uint32_t SPI_CLK(SPI_DEV *spi)
{
#if (SPICLK_OWNER == SPICLK_OWNER_SYS) // system clock manager
    if (spi->reg == CSK_SPI0) {
        return CRM_GetSpi0Freq();
    } else if (spi->reg == CSK_SPI1) {
        return CRM_GetSpi1Freq();
    } else {
        LOGD("%s: invalid SPI device (reg = 0x%08x)!", __func__, spi->reg);
        return 0;
    }

#elif (SPICLK_OWNER == SPICLK_OWNER_SELF) // SPI driver itself
    uint32_t N, M;
    assert(spi != NULL);
    N = spi->ext_clk->bit.DIV_SPI_CLK_N;
    M = spi->ext_clk->bit.DIV_SPI_CLK_M;
#if !VENUS_V1_MPW
    M += 1;
#endif
    assert(N != 0 && M != 0);

#if (IC_BOARD == 0)
    return (CPUFREQ() * N / M); //48MHz
#elif VENUS_V1_MPW
    // it seems that max SPI clock is 60MHz (limited by SYSPLL_USB output)
    return CRM_GetSrcFreq(CRM_IpSrcSysPllUsb) * N / M;
#else
    return CRM_GetSrcFreq(CRM_IpSrcSysPllPeri) * N / M;
#endif

#elif (SPICLK_OWNER == SPICLK_OWNER_OTHER) // constant value
    return SPICLK_CNST_FREQ;
#endif
}

<<<<<<< HEAD
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

=======
static void spi_init_common(SPI_DEV *spi, CSK_SPI_SignalEvent_t cb_event, uint32_t usr_param)
{
>>>>>>> feature/zephyr
    // initialize SPI run-time resources
    spi->info->cb_event = cb_event;
    spi->info->usr_param = usr_param;

<<<<<<< HEAD
//    spi->info->status.busy = 0U;
//    spi->info->status.data_lost = 0U;
//    spi->info->status.mode_fault = 0U;
=======
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
    spi->info->flags = SPI_FLAG_INITIALIZED;  // SPI is initialized
=======
    spi->info->flags = SPI_FLAG_INITIALIZED;


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

    spi_init_common(spi, cb_event, usr_param);
>>>>>>> feature/zephyr

    return CSK_DRIVER_OK;
}


<<<<<<< HEAD
=======
//int32_t
//SPI_Initialize2(void *spi_dev, CSK_SPI_SignalEvent_t cb_event, uint32_t usr_param, uint8_t use_flags)
int32_t
SPI_Initialize_NCS(void *spi_dev, CSK_SPI_SignalEvent_t cb_event, uint32_t usr_param, SPI_CS_SET_FUNC func_set_cs)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (spi->info->flags & SPI_FLAG_INITIALIZED)
        return CSK_DRIVER_OK;

    spi_init_common(spi, cb_event, usr_param);

    spi->info->flags |= SPI_FLAG_NO_CS;  // CS pin not connected
    spi->info->cb_set_cs = func_set_cs;

    return CSK_DRIVER_OK;
}

>>>>>>> feature/zephyr
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

    //uint32_t val;
    switch (state) {
    case CSK_POWER_OFF:
        // disable SPI IRQ
        disable_IRQ(spi->irq_num);

        // abort current TX/RX and clean state if configured
        if (spi->info->flags & SPI_FLAG_CONFIGURED)
            spi_abort_transfer(spi, false);

        // DMA uninitialize
        dma_uninitialize();

<<<<<<< HEAD
        // reset SPI and TX/RX FIFOs
        //spi->reg->CTRL |= (TXFIFORST | RXFIFORST | SPIRST);
        spi->reg->CTRL |= SPIRST;
=======
        // reset SPI and TX/RX FIFOs (clean TX/RX FIFO when closed?)
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST | SPIRST);
        //spi->reg->CTRL |= SPIRST;
>>>>>>> feature/zephyr

        // disable SPI clock here
    #if (SPICLK_OWNER == SPICLK_OWNER_SYS) // system clock manager
        if (spi->reg == CSK_SPI0) {
            __HAL_CRM_SPI0_CLK_DISABLE();
        } else if (spi->reg == CSK_SPI1) {
            __HAL_CRM_SPI1_CLK_DISABLE();
        } else {
            //TODO: error prompt?
        }
    #elif (SPICLK_OWNER == SPICLK_OWNER_SELF) // SPI driver itself
        spi->ext_clk->all &= ~SPI_CLK_ENABLE;
    #elif (SPICLK_OWNER == SPICLK_OWNER_OTHER) // constant value
        //TODO: do something...
    #endif

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
    #if (SPICLK_OWNER == SPICLK_OWNER_SYS) // system clock manager
        if (spi->reg == CSK_SPI0) {
            __HAL_CRM_SPI0_CLK_ENABLE();
        } else if (spi->reg == CSK_SPI1) {
            __HAL_CRM_SPI1_CLK_ENABLE();
        } else {
            //TODO: error prompt?
        }
    #elif (SPICLK_OWNER == SPICLK_OWNER_SELF) // SPI driver itself
        uint16_t divM, divN;

    #if (IC_BOARD == 0)
        {
            divM = divN = 1;
        }
    #else // IC_BOARD == 1
        {
            uint32_t divNM, clk_out;
        #if VENUS_V1_MPW
            clk_out = 60000; //KHz, 60MHz
            divNM = eval_freq_divNM(SPI_CLK_N_MAX, SPI_CLK_M_MAX, CRM_GetSrcFreq(CRM_IpSrcSysPllUsb)/1000, clk_out, NULL);
        #else
            clk_out = 100000; //KHz, FIXME: 100MHz?
            divNM = eval_freq_divNM(SPI_CLK_N_MAX, SPI_CLK_M_MAX, CRM_GetSrcFreq(CRM_IpSrcSysPllPeri)/1000, clk_out, NULL);
        #endif
            assert(divNM != 0);
            divM = divNM & 0xFFFF;
            divN = (divNM >> 16) & 0xFFFF;
        }
    #endif

    #if (VENUS_V1_MPW == 0)
        divM -= 1;
    #endif

       spi->ext_clk->all = SPI_CLK_SEL_USBPLL | SPI_CLK_ENABLE | SPI_CLK_M(divM) | SPI_CLK_N(divN) | SPI_CLK_LD;
        //// SPI0:
        //HAL_CRM_SetSpi0ClkSrc(CRM_IpSrcSysPllUsb);
        //HAL_CRM_SetSpi0ClkDiv(SPI_CLK_N(divM), SPI_CLK_M(divN));
        //// SPI1:
        //HAL_CRM_SetSpi1ClkSrc(CRM_IpSrcSysPllUsb);
        //HAL_CRM_SetSpi1ClkDiv(SPI_CLK_N(divM), SPI_CLK_M(divN));

#elif (SPICLK_OWNER == SPICLK_OWNER_OTHER) // constant value
       //TODO: do something...
#endif

        #if (SPICLK_OWNER != SPICLK_OWNER_OTHER && !VENUS_V1_MPW)
        //SPI1 handshaking signals are connected to AP or CP DMAC?
        if (spi->reg == CSK_SPI1) {
            #define DMA_HS_CP_SPI1_TX   (1 << 2) // 1 = CP DMAC, 0 = AP DMAC
            #define DMA_HS_CP_SPI1_RX   (1 << 3) // 1 = CP DMAC, 0 = AP DMAC
            #ifdef PLATFORM_CP
                IP_SYSCTRL->REG_AP_CTRL5.bit.AP2CP_DMA_HS_SEL |= DMA_HS_CP_SPI1_TX | DMA_HS_CP_SPI1_RX;
            #else
                IP_SYSCTRL->REG_AP_CTRL5.bit.AP2CP_DMA_HS_SEL &= ~(DMA_HS_CP_SPI1_TX | DMA_HS_CP_SPI1_RX);
            #endif
        }
        // CPHA=0 (mode0/mode2) fast mode,  frequency ratio of spi_clk and SCLK should be larger than 2
        //NOTE: As of now (just before MP version), FAST_MODE doesn't work, so remove it!
        //spi->reg->TRANSFMT |= CPHA0_FAST_MODE;
        spi->reg->TRANSFMT &= ~CPHA0_FAST_MODE;
        #endif // !VENUS_V1_MPW

        // DMA initialize
        dma_initialize();

        // reset SPI and TX/RX FIFOs
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST | SPIRST);
        while(spi->reg->CTRL & (TXFIFORST | RXFIFORST | SPIRST));

        spi->txfifo_depth = spi_get_txfifo_size(spi);
        spi->rxfifo_depth = spi_get_rxfifo_size(spi);

<<<<<<< HEAD
        /* TODO: To be optimized. Set both tx and rx fifo depth to 1. */
        spi->reg->CTRL |= (TXTHRES(1) | RXTHRES(1));
        spi->info->tx_bsize_shift = ITEMS_TO_BSIZE(1);
        spi->info->rx_bsize_shift = ITEMS_TO_BSIZE(1);
=======
        // set TX FIFO threshold and RX FIFO threshold to half of TX/RX FIFO depth
        spi->reg->CTRL |= (TXTHRES(spi->txfifo_depth / 2) | RXTHRES(spi->rxfifo_depth / 2));
        spi->info->tx_bsize_shift = ITEMS_TO_BSIZE(spi->txfifo_depth / 2); //DMA_BSIZE_4;
        spi->info->rx_bsize_shift = ITEMS_TO_BSIZE(spi->rxfifo_depth / 2); //DMA_BSIZE_4;
>>>>>>> feature/zephyr

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


<<<<<<< HEAD
static uint32_t
spi_fill_tx_fifo(SPI_DEV *spi, uint32_t xn)
{
    uint32_t i, count, data;
    i = SPI_TXFIFO_ENTRIES(spi);
    count = 0;

    while (i < spi->txfifo_depth && spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
=======
_FAST_FUNC_RO static uint32_t
spi_fill_tx_fifo(SPI_DEV *spi, uint32_t xn)
{
    uint32_t i, count, data, limit;
    i = SPI_TXFIFO_ENTRIES(spi);
    count = 0;

    limit = spi->txfifo_depth;
    //limit = (spi->txfifo_depth >> 1) + (spi->txfifo_depth >> 2);
    while (i < limit && spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
>>>>>>> feature/zephyr
        data = 0;

        // handle the data frame format
        if (spi->info->data_bits <= 8) { // data bits = 1....8
            uint8_t *tx_buf8 = (uint8_t *)spi->info->xfer.tx_buf;
            data = tx_buf8[spi->info->xfer.tx_cnt++];
<<<<<<< HEAD
=======
        #if SUPPORT_8BIT_DATA_MERGE
            if ((spi->info->flags & SPI_FLAG_8BIT_MERGE) &&
                (spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt)) {
                data |= tx_buf8[spi->info->xfer.tx_cnt++] << 8;
                if(spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
                    data |= tx_buf8[spi->info->xfer.tx_cnt++] << 16;
                    if(spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt)
                        data |= tx_buf8[spi->info->xfer.tx_cnt++] << 24;
                }
            }
        #endif
>>>>>>> feature/zephyr
        } else if (spi->info->data_bits <= 16) { // data bits = 9....16
            uint16_t *tx_buf16 = (uint16_t *)spi->info->xfer.tx_buf;
            data = tx_buf16[spi->info->xfer.tx_cnt++];
        } else { // data bits = 17....32
            uint32_t *tx_buf32 = (uint32_t *)spi->info->xfer.tx_buf;
            data = tx_buf32[spi->info->xfer.tx_cnt++];
        }
        spi->reg->DATA = data;
        i++;
        if (++count >= xn)
            break;
    }

    return count;
}


// export SPI API function: SPI_Send
<<<<<<< HEAD
=======
//NOTE: start address (and size? NOT num!) of the buffer pointed by 'data'
//      SHOULD be aligned with 4 if SPI_FLAG_8BIT_MERGE is set!!
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
    spi->info->status.bit.busy = 1U;
    spi->info->status.bit.data_unf = 0;
    spi->info->status.bit.tx_mode = NO_IO;

=======
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
>>>>>>> feature/zephyr
    spi->info->xfer.tx_buf = data;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_SEND;

<<<<<<< HEAD
    // wait prior transfer finish
=======
    bool xfer_started = false;
    bool is_master = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER);

    // wait prior transfer finish
    if (is_master)
>>>>>>> feature/zephyr
    spi_polling_spiactive(spi);

    // set transfer mode to write only and transfer count for write data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

<<<<<<< HEAD
    // DMA mode
    bool xfer_started = false;
=======

    uint32_t intren = 0, count = num;

    // DMA mode
>>>>>>> feature/zephyr
    while ( TX_DMA(spi->info->txrx_mode) ) { // DMA TX
        bool dma_only = TX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_tx_done = 0;

        // enable TX DMA
        //spi->reg->CTRL |= TXDMAEN;

        // select some free DMA channel
<<<<<<< HEAD
=======
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY)
>>>>>>> feature/zephyr
        spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
        spi->info->tx_dyn_dma_ch = dma_channel_select(
                                        &spi->info->tx_dyn_dma_ch,
                                        spi->dma_tx.cb_event,
                                        spi->dma_tx.usr_param,
<<<<<<< HEAD
                                        DMA_CACHE_SYNC_SRC); //DMA_CACHE_SYNC_SRC
=======
                                        DMA_CACHE_SYNC_SRC);
>>>>>>> feature/zephyr
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

<<<<<<< HEAD
        uint32_t control, config_low, config_high;
=======
        int32_t stat;
        uint32_t control, config_low, config_high;

    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t width_val = spi->info->dma_width_shift;
        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            width_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;
        }

        control = DMA_CH_CTLL_DST_WIDTH(width_val) | DMA_CH_CTLL_SRC_WIDTH(width_val) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
>>>>>>> feature/zephyr
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
<<<<<<< HEAD
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel); // DMA_CH_CFGH_FIFO_MODE |
        //config_high = DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel);
=======
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel);
>>>>>>> feature/zephyr

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
<<<<<<< HEAD
                            num, control, config_low, config_high, 0, 0);
=======
                            count, control, config_low, config_high, 0, 0);
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
        spi->reg->INTREN = SPI_ENDINT;
=======
        //spi->reg->INTREN = SPI_ENDINT;
        intren = SPI_ENDINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)
<<<<<<< HEAD
        // enable interrupts
        spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);
=======
        // fill the TX FIFO if PIO mode
        spi_fill_tx_fifo(spi, spi->txfifo_depth >> 1); // /2
        //spi_fill_tx_fifo(spi, 1);

        // enable interrupts
        //spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);
        intren = (SPI_TXFIFOINT | SPI_ENDINT);
>>>>>>> feature/zephyr

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
<<<<<<< HEAD
    if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        spi->reg->INTREN |= SPI_TXFIFOURINT;
        spi->reg->SLVST |= SLVST_READY;
    }

    // fill the TX FIFO if PIO mode
    if (TX_PIO(spi->info->txrx_mode))
        spi_fill_tx_fifo(spi, spi->txfifo_depth/4);

    // trigger transfer when SPI master mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER) {
=======
    if(!is_master) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        //spi->reg->INTREN |= SPI_TXFIFOURINT;
        intren |= SPI_TXFIFOURINT;
        spi->reg->SLVST |= SLVST_READY;
    }



    // trigger transfer when SPI master mode
    if (is_master) {
        // wait until there's at least 1 data in the TX FIFO??
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi)) ;
        spi->reg->CMD = 0;
    }

    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;


    return CSK_DRIVER_OK;
}


/*
// Sending data until completed (blocked send)
int32_t
SPI_Send_Sync(void *spi_dev, const void *data, uint32_t num)
{
    int32_t ret;

    ret = SPI_Send(spi_dev, data, num);
    if (ret != CSK_DRIVER_OK)
        return ret;

    SPI_DEV *spi = safe_spi_dev(spi_dev);
    // wait until Active bit is cleared in STATUS register and ENDINT ISR is called
    while ( (spi->reg->STATUS & 0x1) || spi->info->status.bit.busy)
        ;

    return CSK_DRIVER_OK;
}
*/

// Wait for SPI operation (Send/Receive/Transfer) done...
void SPI_Wait_Done(void *spi_dev)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);

    // wait until Active bit is cleared in STATUS register and ENDINT ISR is called
    //while ( (spi->reg->STATUS & 0x1) || spi->info->status.bit.busy);

    // wait ENDINT ISR is called if ENDINT is enabled, otherwise
    // wait all TX data is fetched from RAM into TX FIFO (NOT sent out on SPI data line), or
    // wait all RX data is fetched from RX FIFO into RAM (already received in on SPI data line)
    while (spi->info->status.bit.busy);
}


// Sending data continuously without triggering ENDINT interrupt (when CS signal is raised up)
//NOTE: if SPI_FLAG_8BIT_MERGE is set,
//      start address (and size? NOT num!) of the buffer pointed by 'data' SHOULD be aligned with 4,
//      and the parameter 'num' SHOULD be also aligned with 4 if (SPI slave + no CS connected)
int32_t
SPI_Send_NEnd(void *spi_dev, const void *data, uint32_t num)
{
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

    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);

#if SUPPORT_8BIT_DATA_MERGE
    if (is_slave && no_cs && (spi->info->flags & SPI_FLAG_8BIT_MERGE)) {
        if (num & 0x3) {
            CLOGE("[ERROR, 8bit-MERGE]: slave TX ONLY 4N bytes when no CS! cur = %d\r\n", num);
            return CSK_DRIVER_ERROR_PARAMETER;
        }
    }
#endif

    // We cannot know when exactly TX operation is completed in the case of slave mode and no-CS line connected,
    // for there is no raised CS signal to notify the SPI slave when the last data is shifted out.
    // But we can know exactly when the simultaneous RX is done, and at that time TX should be completed too.
    if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
        return SPI_Transfer_NEnd(spi_dev, data, NULL, num);

    // set busy flag
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
    spi->info->status.bit.no_endint = 1; // No ENDINT interrupt!!

    spi->info->xfer.tx_buf = data;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_SEND;

//    // wait prior transfer finish
//    spi_polling_spiactive(spi);

//    // set transfer mode to write only and transfer count for write data
//    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num) | // 512
//                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    bool xfer_started = false;
    uint32_t intren = 0, count = num;
    uint32_t thres = spi->txfifo_depth >> 1;

    // DMA mode
    while ( TX_DMA(spi->info->txrx_mode) ) { // DMA TX

        bool dma_only = TX_DMA_ONLY(spi->info->txrx_mode);
//        bool recount = false;

        // initial the dma done flag
        spi->info->xfer.dma_tx_done = 0;

        // select some free DMA channel
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY)
            spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
        spi->info->tx_dyn_dma_ch = dma_channel_select(
                                        &spi->info->tx_dyn_dma_ch,
                                        spi->dma_tx.cb_event,
                                        spi->dma_tx.usr_param,
                                        DMA_CACHE_SYNC_SRC); //TODO: DMA_CACHE_SYNC_NOP?
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

        int32_t stat;
        uint32_t control, config_low, config_high;

    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t width_val = spi->info->dma_width_shift;
        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            width_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

//            //FIXME: special treatment for DATA_MERGE, the odd is less than 1 word
//            // is there any deficiency in the SPI IP?
//            recount = (is_slave && (num % (thres << 2) < 4) && count > thres); //FIXME:
        }

        control = DMA_CH_CTLL_DST_WIDTH(width_val) | DMA_CH_CTLL_SRC_WIDTH(width_val) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel); //

//        recount = recount || (is_slave && no_cs && count > thres);
//        if (recount)
//            count = count / thres * thres;

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
                            count, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->tx_dyn_dma_ch, false);
            spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

//        // enable TX DMA
//        spi->reg->CTRL |= TXDMAEN;

        // enable interrupts
        //intren = SPI_ENDINT;

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;

        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)

//    #if SUPPORT_8BIT_DATA_MERGE
//        // count is divided by 4 if 8bit DATA MERGE
//        if(spi->info->flags & SPI_FLAG_8BIT_MERGE) {
//            count = num >> 2;  // 8bit => 32bit
//        }
//    #endif

        // fill the TX FIFO if PIO mode
        spi_fill_tx_fifo(spi, spi->txfifo_depth >> 1); // /2
        //spi_fill_tx_fifo(spi, 1);

        // enable interrupts
        //intren = (SPI_TXFIFOINT | SPI_ENDINT);
        intren = (SPI_TXFIFOINT);

        spi->info->status.bit.tx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA nor PIO is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // wait prior transfer finish
    if (!is_slave) // || !no_cs || spi->info->cb_set_cs != NULL
        spi_polling_spiactive(spi);

//    if (no_cs)
//        spi_set_tx_fifo_threshold(spi, spi->txfifo_depth >> 1);

    // enable TX DMA
    if (spi->info->status.bit.tx_mode == DMA_IO)
        spi->reg->CTRL |= TXDMAEN;
    else
        spi->reg->CTRL &= ~TXDMAEN;

    // enable TX FIFO underrun interrupt when slave mode
    // set the Ready bit in the SPI Slave Status Register
    if(is_slave) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        //spi->reg->INTREN |= SPI_TXFIFOURINT;
        intren |= SPI_TXFIFOURINT;
        spi->reg->SLVST |= SLVST_READY;
    }

    // set transfer mode to write only and transfer count for write data
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRONLY | WR_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));


    if (is_slave) {
        if (no_cs && spi->info->cb_set_cs != NULL)
            spi->info->cb_set_cs(spi->spi_idx, 0);
    } else { // trigger transfer when SPI master mode
>>>>>>> feature/zephyr
        // wait until there's at least 1 data in the TX FIFO??
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi)) ;
        spi->reg->CMD = 0;
    }

<<<<<<< HEAD
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
=======
    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Receive
//NOTE: start address and size (NOT num!) of the buffer pointed by 'data'
//      SHOULD be aligned with 4 if SPI_FLAG_8BIT_MERGE is set!!
int32_t
SPI_Receive(void *spi_dev, void *data, uint32_t num)
{
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
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
    spi->info->xfer.rx_buf = data;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.cur_op = SPI_RECEIVE;

    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool xfer_started = false;

    // wait prior transfer finish
    if (!is_slave)
    spi_polling_spiactive(spi);

    // set transfer mode to read only and transfer count for read data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    uint32_t intren = 0, count = num;

    // DMA mode
    while ( RX_DMA(spi->info->txrx_mode) ) { // DMA RX
        bool dma_only = RX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

        // enable RX DMA
        //spi->reg->CTRL |= RXDMAEN;

        // select some free DMA channel
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY)
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

        int32_t stat;
        uint32_t control, config_low, config_high;

    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t swidth_val, dwidth_val, dbsize_val;
        swidth_val = dwidth_val = spi->info->dma_width_shift;
        dbsize_val = spi->info->rx_bsize_shift;

        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            swidth_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

            if (((uint32_t)data & 0x3) == 0)
                dwidth_val = DMA_WIDTH_WORD;
            else if (((uint32_t)data & 0x1) == 0) {
                dwidth_val = DMA_WIDTH_HALFWORD;
                dbsize_val <<= 1;
            }
        }

        control = DMA_CH_CTLL_DST_WIDTH(dwidth_val) | DMA_CH_CTLL_SRC_WIDTH(swidth_val) |
                DMA_CH_CTLL_DST_BSIZE(dbsize_val) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); //

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
                            (uint32_t) spi->info->xfer.rx_buf,
                            count, control, config_low, config_high, 0, 0);
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
        intren = SPI_ENDINT;

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)
        // enable interrupts
        //spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);
        intren = (SPI_RXFIFOINT | SPI_ENDINT);

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
	if(is_slave) {
	    CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
	    //spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
	    intren |= SPI_RXFIFOORINT | SPI_SLVCMD;
	} else { // trigger transfer when SPI master mode
        spi->reg->CMD = 0;
	}

    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;

    return CSK_DRIVER_OK;
}


// Receiving data continuously without triggering ENDINT interrupt (when CS signal is raised up)
//NOTE: if SPI_FLAG_8BIT_MERGE is set,
//      start address and size (NOT num!) of the buffer pointed by 'data' SHOULD be aligned with 4,
//      and the parameter 'num' SHOULD be also aligned with 4 if (SPI slave + no CS connected)
int32_t
SPI_Receive_NEnd(void *spi_dev, void *data, uint32_t num)
{
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

    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);

#if SUPPORT_8BIT_DATA_MERGE
    if (is_slave && no_cs && (spi->info->flags & SPI_FLAG_8BIT_MERGE)) {
        if (num & 0x3) {
            CLOGE("[ERROR, 8bit-MERGE]: slave RX ONLY 4N bytes when no CS! cur = %d\r\n", num);
            return CSK_DRIVER_ERROR_PARAMETER;
        }
    }
#endif

    // set busy flag
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
    spi->info->status.bit.no_endint = 1; // No ENDINT interrupt!!
    spi->info->xfer.rx_buf = data;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.cur_op = SPI_RECEIVE;

//    // wait prior transfer finish
//    spi_polling_spiactive(spi);

//    // set transfer mode to read only and transfer count for read data
//    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num));
//    spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num) |
//                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    bool xfer_started = false;
    uint32_t intren = 0, count = num, thres = spi->rxfifo_depth >> 1;

    // DMA mode
    while (RX_DMA(spi->info->txrx_mode)) { // DMA RX
        bool dma_only = RX_DMA_ONLY(spi->info->txrx_mode);
        bool recount = false;

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

        // select some free DMA channel
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY)
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

        int32_t stat;
        uint32_t control, config_low, config_high;

    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t swidth_val, dwidth_val, dbsize_val;
        swidth_val = dwidth_val = spi->info->dma_width_shift;
        dbsize_val = spi->info->rx_bsize_shift;

        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            count = (count + 3) >> 2; // 8bit => 32bit
            swidth_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

            if (((uint32_t)data & 0x3) == 0) {
                dwidth_val = DMA_WIDTH_WORD;
            } else if (((uint32_t)data & 0x1) == 0) {
                dwidth_val = DMA_WIDTH_HALFWORD;
                dbsize_val <<= 1;
            }

            //FIXME: special treatment for DATA_MERGE, the odd is less than 1 word
            // is there any deficiency in the SPI IP?
            recount = (is_slave && (num % (thres << 2) < 4) && count > thres); //FIXME:
        }

        control = DMA_CH_CTLL_DST_WIDTH(dwidth_val) | DMA_CH_CTLL_SRC_WIDTH(swidth_val) |
                DMA_CH_CTLL_DST_BSIZE(dbsize_val) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); //

        recount = recount || (is_slave && no_cs && count > thres);
        if (recount)
            count = count / thres * thres;

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
                            (uint32_t) spi->info->xfer.rx_buf,
                            count, control, config_low, config_high, 0, 0);
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->rx_dyn_dma_ch, false);
            spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

//        // enable RX DMA
//        spi->reg->CTRL |= RXDMAEN;

        // enable interrupts
        //intren = SPI_ENDINT;

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;

        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)

    #if SUPPORT_8BIT_DATA_MERGE
        // count is divided by 4 if 8bit DATA MERGE
        if(spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            count = num >> 2;  // 8bit => 32bit
        }
    #endif

        // enable interrupts
        //intren = (SPI_RXFIFOINT | SPI_ENDINT);
        intren = (SPI_RXFIFOINT);

        spi->info->status.bit.rx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA nor PIO is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // wait prior transfer finish
    if (!is_slave) // || !no_cs || spi->info->cb_set_cs != NULL
        spi_polling_spiactive(spi);

    // set new RX FIFO threshold if necessary
//    if (no_cs || spi->info->status.bit.rx_mode == PIO_IO)
//        spi_set_rx_fifo_threshold(spi, count < thres ? 1 : thres);
    if (count < thres) {
        if (no_cs || spi->info->status.bit.rx_mode == PIO_IO)
            thres = 1;
    #if SUPPORT_8BIT_DATA_MERGE
        //FIXME: special treatment for DATA_MERGE, less than 1 word,
        // is there any deficiency in the SPI IP?
        else if ((spi->info->flags & SPI_FLAG_8BIT_MERGE) && count == 1)
            thres = 1;
    #endif
    }
    spi_set_rx_fifo_threshold(spi, thres);

    // enable RX DMA
    if (spi->info->status.bit.rx_mode == DMA_IO)
        spi->reg->CTRL |= RXDMAEN;
    else
        spi->reg->CTRL &= ~RXDMAEN;

    // enable RX FIFO overrun interrupt when slave mode
    // set slave cmd interrupt
    if(is_slave) {
        CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
        //spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
        intren |= SPI_RXFIFOORINT | SPI_SLVCMD;
    }

    // set transfer mode to read only and transfer count for read data
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_RDONLY | RD_TRANCNT(num) |
                (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    // trigger transfer when SPI master mode
    if (is_slave) {
        if (no_cs && spi->info->cb_set_cs != NULL)
            spi->info->cb_set_cs(spi->spi_idx, 0);
    } else {
        spi->reg->CMD = 0;
    }

    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;

    return CSK_DRIVER_OK;
}


// export SPI API function: SPI_Transfer
//NOTE: start address and size (NOT num!) of the buffer pointed by 'data_in' and
//      start address (and size? NOT num!) of the buffer pointed by 'data_out'
//      SHOULD be aligned with 4 if SPI_FLAG_8BIT_MERGE is set!!
int32_t
SPI_Transfer(void *spi_dev, const void *data_out, void *data_in, uint32_t num)
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((data_out == NULL) || (data_in == NULL) || (num == 0U) || (num > MAX_TRANCNT))
        return CSK_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return CSK_DRIVER_ERROR;

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    // set busy flag
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
    spi->info->status.bit.tx_mode = spi->info->status.bit.rx_mode = NUM_IO;
    spi->info->xfer.rx_buf = data_in;
    spi->info->xfer.tx_buf = data_out;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_TRANSFER;

    bool xfer_started, dma_only;
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);

    // wait prior transfer finish
    if (!is_slave)
    spi_polling_spiactive(spi);

    // set transfer mode to write and read at the same time and transfer count for write/read data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num) |
                        (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    int32_t stat;
    uint32_t control, config_low, config_high, count;
    uint32_t intren = 0;

    // DMA mode
    xfer_started = false;
    count = num;
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

    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t width_val = spi->info->dma_width_shift;
        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            width_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;
        }

        control = DMA_CH_CTLL_DST_WIDTH(width_val) | DMA_CH_CTLL_SRC_WIDTH(width_val) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    #endif
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel); //

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
                            count, control, config_low, config_high, 0, 0);
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
        //spi->reg->INTREN = SPI_ENDINT;
        intren = SPI_ENDINT;

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)
        // fill the TX FIFO if PIO mode
        spi_fill_tx_fifo(spi, spi->txfifo_depth >> 1); // /2
        //spi_fill_tx_fifo(spi, 1);

        // enable interrupts
        //spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);
        intren = (SPI_TXFIFOINT | SPI_ENDINT);

        spi->info->status.bit.tx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA TX nor PIO TX is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // enable TX FIFO underrun interrupt when slave mode
    if (is_slave) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        //spi->reg->INTREN |= SPI_TXFIFOURINT;
        intren |= SPI_TXFIFOURINT;
    }


    xfer_started = false;
    count = num;
    while ( RX_DMA(spi->info->txrx_mode) ) { // DMA RX
        dma_only = RX_DMA_ONLY(spi->info->txrx_mode);
>>>>>>> feature/zephyr

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

        // enable RX DMA
        //spi->reg->CTRL |= RXDMAEN;

        // select some free DMA channel
<<<<<<< HEAD
        spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        spi->info->rx_dyn_dma_ch = dma_channel_select(
                                        &spi->info->rx_dyn_dma_ch,
                                        spi->dma_rx.cb_event,
                                        spi->dma_rx.usr_param,
                                        DMA_CACHE_SYNC_DST);
=======
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY)
        spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        dma_channel_select(&spi->info->rx_dyn_dma_ch,
                            spi->dma_rx.cb_event,
                            spi->dma_rx.usr_param,
                            DMA_CACHE_SYNC_DST);
>>>>>>> feature/zephyr
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

<<<<<<< HEAD
        uint32_t control, config_low, config_high;
=======
    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t swidth_val, dwidth_val, dbsize_val;
        swidth_val = dwidth_val = spi->info->dma_width_shift;
        dbsize_val = spi->info->rx_bsize_shift;

        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            swidth_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

            if (((uint32_t)data_in & 0x3) == 0)
                dwidth_val = DMA_WIDTH_WORD;
            else if (((uint32_t)data_in & 0x1) == 0) {
                dwidth_val = DMA_WIDTH_HALFWORD;
                dbsize_val <<= 1;
            }
        }

        control = DMA_CH_CTLL_DST_WIDTH(dwidth_val) | DMA_CH_CTLL_SRC_WIDTH(swidth_val) |
                DMA_CH_CTLL_DST_BSIZE(dbsize_val) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
>>>>>>> feature/zephyr
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
<<<<<<< HEAD
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); // DMA_CH_CFGH_FIFO_MODE |
=======
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); //
>>>>>>> feature/zephyr

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
                            (uint32_t) spi->info->xfer.rx_buf,
<<<<<<< HEAD
                            num, control, config_low, config_high, 0, 0);
=======
                            count, control, config_low, config_high, 0, 0);
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
        spi->reg->INTREN = SPI_ENDINT;
=======
        //spi->reg->INTREN = SPI_ENDINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)
        // enable interrupts
<<<<<<< HEAD
        spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);
=======
        //spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);
        //spi->reg->INTREN |= SPI_RXFIFOINT;
        intren |= SPI_RXFIFOINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.rx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
<<<<<<< HEAD
        LOGD("%s: Neither DMA nor PIO is started!!\r\n", __func__);
=======
        LOGD("%s: Neither DMA RX nor PIO RX is started!!\r\n", __func__);
>>>>>>> feature/zephyr
        return CSK_DRIVER_ERROR;
    }

    // enable RX FIFO overrun interrupt when slave mode
<<<<<<< HEAD
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

=======
    // set the Ready bit in the SPI Slave Status Register
    if(is_slave) {
        CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
        //spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
        intren |= SPI_RXFIFOORINT | SPI_SLVCMD;
        spi->reg->SLVST |= SLVST_READY;
    } else { // trigger transfer when SPI master mode
        // wait until there's at least 1 data in the TX FIFO?
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi));
        spi->reg->CMD = 0;
    }

    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;

>>>>>>> feature/zephyr
    return CSK_DRIVER_OK;
}


<<<<<<< HEAD
// export SPI API function: SPI_Transfer
int32_t
SPI_Transfer(void *spi_dev, const void *data_out, void *data_in, uint32_t num)
=======
// Duplex-Transfering data continuously without triggering ENDINT interrupt (when CS signal is raised up)
//NOTE: if SPI_FLAG_8BIT_MERGE is set,
//      start address and size (NOT num!) of the buffer pointed by 'data_in' SHOULD be aligned with 4, and
//      start address (and size? NOT num!) of the buffer pointed by 'data_out' SHOULD be aligned with 4, and
//      and the parameter 'num' SHOULD be also aligned with 4 if (SPI slave + no CS connected)
int32_t
SPI_Transfer_NEnd(void *spi_dev, const void *data_out, void *data_in, uint32_t num)
>>>>>>> feature/zephyr
{
    SPI_DEV *spi = safe_spi_dev(spi_dev);
    if (spi == NULL) {
        LOGD("%s: invalid SPI device (0x%08x)!", __func__, spi_dev);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

<<<<<<< HEAD
    int32_t stat;

    if ((data_out == NULL) || (data_in == NULL) || (num == 0U) || (num > MAX_TRANCNT))
=======
    if ((data_out == NULL) || (num == 0U) || (num > MAX_TRANCNT)) //  || (data_in == NULL)
>>>>>>> feature/zephyr
        return CSK_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return CSK_DRIVER_ERROR;

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

<<<<<<< HEAD
    // set busy flag
    spi->info->status.bit.busy = 1U;
    spi->info->status.bit.data_unf = 0U;
    spi->info->status.bit.data_ovf = 0U;
    spi->info->status.bit.tx_mode = NO_IO;
    spi->info->status.bit.rx_mode = NO_IO;
=======
    bool rx_sync_only = (data_in == NULL);
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);
    bool xfer_started, dma_only, recount;

#if SUPPORT_8BIT_DATA_MERGE
    if (is_slave && no_cs && (spi->info->flags & SPI_FLAG_8BIT_MERGE)) {
        if (num & 0x3) {
            CLOGE("[ERROR, 8bit-MERGE]: slave RX ONLY 4N bytes when no CS! cur = %d\r\n", num);
            return CSK_DRIVER_ERROR_PARAMETER;
        }
    }
#endif

    // set busy flag
    spi->info->status.all = 0;
    spi->info->status.bit.busy = 1;
    spi->info->status.bit.tx_mode = spi->info->status.bit.rx_mode = NUM_IO;
    spi->info->status.bit.no_endint = 1; // No ENDINT interrupt!!
    spi->info->status.bit.rx_sync = (rx_sync_only? 1: 0);
>>>>>>> feature/zephyr

    spi->info->xfer.rx_buf = data_in;
    spi->info->xfer.tx_buf = data_out;
    spi->info->xfer.rx_cnt = 0U;
    spi->info->xfer.tx_cnt = 0U;
    spi->info->xfer.req_rx_cnt = num;
    spi->info->xfer.req_tx_cnt = num;
    spi->info->xfer.cur_op = SPI_TRANSFER;

<<<<<<< HEAD
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
=======
//    // wait prior transfer finish
//    spi_polling_spiactive(spi);

//    // set transfer mode to write and read at the same time and transfer count for write/read data
//    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num));
//    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num) |
//                        (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));

    uint32_t intren = 0, thres, count;
    uint32_t control, config_low, config_high;
    int32_t stat;

    // DMA mode
    xfer_started = false;
//    recount = false;
    count = num;
>>>>>>> feature/zephyr
    while ( TX_DMA(spi->info->txrx_mode) ) { // DMA TX
        dma_only = TX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_tx_done = 0;

<<<<<<< HEAD
        // enable TX DMA
        //spi->reg->CTRL |= TXDMAEN;

        // select some free DMA channel
        spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
=======
        // select some free DMA channel
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY)
            spi->info->tx_dyn_dma_ch = spi->dma_tx.channel;
>>>>>>> feature/zephyr
        dma_channel_select(&spi->info->tx_dyn_dma_ch,
                            spi->dma_tx.cb_event,
                            spi->dma_tx.usr_param,
                            DMA_CACHE_SYNC_SRC);
        if (spi->info->tx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

<<<<<<< HEAD
=======
    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t width_val = spi->info->dma_width_shift;
        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            //count >>= 2; // 8bit => 32bit
            count = (count + 3) >> 2; // 8bit => 32bit
            width_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

//            //FIXME: special treatment for DATA_MERGE, the odd is less than 1 word
//            // is there any deficiency in the SPI IP?
//            recount = (is_slave && (num % (thres << 2) < 4) && count > thres); //FIXME:
        }

        control = DMA_CH_CTLL_DST_WIDTH(width_val) | DMA_CH_CTLL_SRC_WIDTH(width_val) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else

>>>>>>> feature/zephyr
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->tx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->tx_bsize_shift) |
                DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
<<<<<<< HEAD
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel); // DMA_CH_CFGH_FIFO_MODE |
=======
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(spi->dma_tx.reqsel); //

//        recount = recount || (is_slave && no_cs && count > thres);
//        if (recount)
//            count = count / thres * thres;
>>>>>>> feature/zephyr

        // configure DMA channel
        stat = dma_channel_configure (spi->info->tx_dyn_dma_ch,
                            (uint32_t) spi->info->xfer.tx_buf,
                            (uint32_t) (&(spi->reg->DATA)),
<<<<<<< HEAD
                            num, control, config_low, config_high, 0, 0);
=======
                            count, control, config_low, config_high, 0, 0);
>>>>>>> feature/zephyr
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->tx_dyn_dma_ch, false);
            spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA TX
        }

<<<<<<< HEAD
        // enable TX DMA
        spi->reg->CTRL |= TXDMAEN;

        // enable interrupts
        spi->reg->INTREN = SPI_ENDINT;
=======
//        // enable TX DMA
//        spi->reg->CTRL |= TXDMAEN;

        // enable interrupts
        //spi->reg->INTREN = SPI_ENDINT;
        //intren = SPI_ENDINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.tx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && TX_PIO(spi->info->txrx_mode)) { // PIO TX (interrupt mode)
<<<<<<< HEAD
        // enable interrupts
        spi->reg->INTREN = (SPI_TXFIFOINT | SPI_ENDINT);
=======

//    #if SUPPORT_8BIT_DATA_MERGE
//        // count is divided by 4 if 8bit DATA MERGE
//        if(spi->info->flags & SPI_FLAG_8BIT_MERGE) {
//            count = num >> 2;  // 8bit => 32bit
//        }
//    #endif

        // fill the TX FIFO if PIO mode
        spi_fill_tx_fifo(spi, spi->txfifo_depth >> 1); // /2
        //spi_fill_tx_fifo(spi, 1);

        // enable interrupts
        //intren = (SPI_TXFIFOINT | SPI_ENDINT);
        intren = (SPI_TXFIFOINT);
>>>>>>> feature/zephyr

        spi->info->status.bit.tx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA TX nor PIO TX is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

<<<<<<< HEAD
    // enable TX FIFO underrun interrupt when slave mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
        spi->reg->INTREN |= SPI_TXFIFOURINT;
    }

    xfer_started = false;
    while ( RX_DMA(spi->info->txrx_mode) ) { // DMA RX
=======
//    // enable TX FIFO underrun interrupt when slave mode
//    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
//        CLR_SLV_WRCNT(spi); // clear WCnt on updated IP
//        //spi->reg->INTREN |= SPI_TXFIFOURINT;
//        intren |= SPI_TXFIFOURINT;
//    }

    xfer_started = false;
    recount = false;
    count = num;
    thres = spi->rxfifo_depth >> 1;
    while ( RX_DMA(spi->info->txrx_mode) || rx_sync_only ) { // DMA RX
>>>>>>> feature/zephyr
        dma_only = RX_DMA_ONLY(spi->info->txrx_mode);

        // initial the dma done flag
        spi->info->xfer.dma_rx_done = 0;

<<<<<<< HEAD
        // enable RX DMA
        //spi->reg->CTRL |= RXDMAEN;

        // select some free DMA channel
        spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        dma_channel_select(&spi->info->rx_dyn_dma_ch,
                            spi->dma_rx.cb_event,
                            spi->dma_rx.usr_param,
                            DMA_CACHE_SYNC_DST);
=======
        // select some free DMA channel
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY)
            spi->info->rx_dyn_dma_ch = spi->dma_rx.channel;
        dma_channel_select(&spi->info->rx_dyn_dma_ch,
                            spi->dma_rx.cb_event,
                            spi->dma_rx.usr_param,
                            (rx_sync_only ? DMA_CACHE_SYNC_NOP : DMA_CACHE_SYNC_DST));
>>>>>>> feature/zephyr
        if (spi->info->rx_dyn_dma_ch == DMA_CHANNEL_ANY) {
            LOGD("%s: NO free DMA channel!!\r\n", __func__);
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

<<<<<<< HEAD
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); // DMA_CH_CFGH_FIFO_MODE |
=======
    #if SUPPORT_8BIT_DATA_MERGE
        uint32_t swidth_val, dwidth_val, dbsize_val;
        swidth_val = dwidth_val = spi->info->dma_width_shift;
        dbsize_val = spi->info->rx_bsize_shift;

        if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            count = (count + 3) >> 2; // 8bit => 32bit
            swidth_val = DMA_WIDTH_WORD;
            //spi->info->status.bit.dma_merge = 1;

            if (((uint32_t)data_in & 0x3) == 0) {
                dwidth_val = DMA_WIDTH_WORD;
            } else if (((uint32_t)data_in & 0x1) == 0) {
                dwidth_val = DMA_WIDTH_HALFWORD;
                dbsize_val <<= 1;
            }

            //FIXME: special treatment for DATA_MERGE, the odd is less than 1 word
            // is there any deficiency in the SPI IP?
            recount = (is_slave && (num % (thres << 2) < 4) && count > thres); //FIXME:
        }

        control = DMA_CH_CTLL_DST_WIDTH(dwidth_val) | DMA_CH_CTLL_SRC_WIDTH(swidth_val) |
                DMA_CH_CTLL_DST_BSIZE(dbsize_val) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                (rx_sync_only ? DMA_CH_CTLL_DST_FIX : DMA_CH_CTLL_DST_INC) | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;

    #else
        control = DMA_CH_CTLL_DST_WIDTH(spi->info->dma_width_shift) | DMA_CH_CTLL_SRC_WIDTH(spi->info->dma_width_shift) |
                DMA_CH_CTLL_DST_BSIZE(spi->info->rx_bsize_shift) | DMA_CH_CTLL_SRC_BSIZE(spi->info->rx_bsize_shift) |
                (rx_sync_only ? DMA_CH_CTLL_DST_FIX : DMA_CH_CTLL_DST_INC) | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |
                DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    #endif

        config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
        config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(spi->dma_rx.reqsel); //

        recount = recount || (is_slave && no_cs && count > thres);
        if (recount)
            count = count / thres * thres;
>>>>>>> feature/zephyr

        // configure DMA channel
        stat = dma_channel_configure (spi->info->rx_dyn_dma_ch,
                            (uint32_t) (&(spi->reg->DATA)),
<<<<<<< HEAD
                            (uint32_t) spi->info->xfer.rx_buf,
                            num, control, config_low, config_high, 0, 0);
=======
                            (uint32_t)(rx_sync_only ? &spi->info->xfer.rx_data : spi->info->xfer.rx_buf),
                            count, control, config_low, config_high, 0, 0);
>>>>>>> feature/zephyr
        if (stat == -1) {
            LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
            dma_channel_disable(spi->info->rx_dyn_dma_ch, false);
            spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
            if (dma_only)   return CSK_DRIVER_ERROR;
            break; // quit DMA RX
        }

<<<<<<< HEAD
        // enable RX DMA
        spi->reg->CTRL |= RXDMAEN;

        // enable interrupts
        //spi->reg->INTREN = SPI_ENDINT;
=======
//        // enable RX DMA
//        spi->reg->CTRL |= RXDMAEN;

        // enable interrupts
        //intern = SPI_ENDINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.rx_mode = DMA_IO;
        xfer_started = true;
        break;
    } // end while

    if (!xfer_started && RX_PIO(spi->info->txrx_mode)) { // PIO RX (interrupt mode)
<<<<<<< HEAD
        // enable interrupts
        //spi->reg->INTREN = (SPI_RXFIFOINT | SPI_ENDINT);
        spi->reg->INTREN |= SPI_RXFIFOINT;
=======

    #if SUPPORT_8BIT_DATA_MERGE
        // count is divided by 4 if 8bit DATA MERGE
        if(spi->info->flags & SPI_FLAG_8BIT_MERGE) {
            count = num >> 2;  // 8bit => 32bit
        }
    #endif

        // enable interrupts
        //intern = (SPI_RXFIFOINT | SPI_ENDINT);
        intren |= SPI_RXFIFOINT;
>>>>>>> feature/zephyr

        spi->info->status.bit.rx_mode = PIO_IO;
        xfer_started = true;
    }

    // neither DMA nor PIO transfer is started
    if (!xfer_started) {
        LOGD("%s: Neither DMA RX nor PIO RX is started!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

<<<<<<< HEAD
    // enable RX FIFO overrun interrupt when slave mode
    // set the Ready bit in the SPI Slave Status Register
    if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
        CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
        spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
        spi->reg->SLVST |= SLVST_READY;
    }

    // fill the TX FIFO if TX PIO mode
    if (TX_PIO(spi->info->txrx_mode))
        spi_fill_tx_fifo(spi, spi->txfifo_depth/4);

    // trigger transfer when SPI master mode
    if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_MASTER) {
        // wait until there's at least 1 data in the TX FIFO?
        //while(SPI_TXFIFO_ENTRIES(spi) == 0);
        while(SPI_TXFIFO_EMPTY(spi));
        spi->reg->CMD = 0;
    }

=======
//    // enable RX FIFO overrun interrupt when slave mode
//    // set the Ready bit in the SPI Slave Status Register
//    if((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
//        CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
//        //spi->reg->INTREN |= SPI_RXFIFOORINT | SPI_SLVCMD;
//        intren |= SPI_RXFIFOORINT | SPI_SLVCMD;
//        spi->reg->SLVST |= SLVST_READY;
//    }


    // wait prior transfer finish
    if (!is_slave) // || !no_cs || spi->info->cb_set_cs != NULL
        spi_polling_spiactive(spi);

    // set new RX FIFO threshold if necessary
    //if (no_cs || spi->info->status.bit.rx_mode == PIO_IO) {
    //    //spi_set_tx_fifo_threshold(spi, spi->txfifo_depth >> 1);
    //    spi_set_rx_fifo_threshold(spi, num < thres ? 1 : thres);
    //}
    if (count < thres) {
        if (no_cs || spi->info->status.bit.rx_mode == PIO_IO)
            thres = 1;
    #if SUPPORT_8BIT_DATA_MERGE
        //FIXME: special treatment for DATA_MERGE, less than 1 word,
        // is there any deficiency in the SPI IP?
        else if ((spi->info->flags & SPI_FLAG_8BIT_MERGE) && count == 1)
            thres = 1;
    #endif
    }
    spi_set_rx_fifo_threshold(spi, thres);

    // enable RX DMA
    if (spi->info->status.bit.rx_mode == DMA_IO)
        spi->reg->CTRL |= RXDMAEN;
    else
        spi->reg->CTRL &= ~RXDMAEN;

    // enable TX DMA
    if (spi->info->status.bit.tx_mode == DMA_IO)
        spi->reg->CTRL |= TXDMAEN;
    else
        spi->reg->CTRL &= ~TXDMAEN;

    // set transfer mode to write and read at the same time and transfer count for write/read data
    //spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num));
    spi->reg->TRANSCTRL = (SPI_TRANSMODE_WRnRD | WR_TRANCNT(num) | RD_TRANCNT(num) |
                        (spi->reg->TRANSCTRL & SPI_TRANSMODE_CMD_EN));


    // enable RX FIFO overrun, TX FIFO underrun, and slave cmd interrupt when slave mode
    // set the Ready bit in the SPI Slave Status Register
    if(is_slave) {
        CLR_SLV_RDWRCNT(spi); // clear RCnt & WCnt on updated IP
        intren |= SPI_TXFIFOURINT | SPI_RXFIFOORINT | SPI_SLVCMD;
        spi->reg->SLVST |= SLVST_READY;
        if (no_cs && spi->info->cb_set_cs != NULL)
           spi->info->cb_set_cs(spi->spi_idx, 0);
    } else { // trigger transfer when SPI master mode
        // wait until there's at least 1 data in the TX FIFO?
        //while(SPI_TXFIFO_EMPTY(spi));
        spi->reg->CMD = 0;
    }

    // enable interrupts
    // (Interrupt could be triggered soon once enabled, so enable interrupts at the end!!)
    spi->reg->INTREN = intren;

//    if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
//       spi->info->cb_set_cs(spi->spi_idx, 0);

>>>>>>> feature/zephyr
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
<<<<<<< HEAD
        if (spi->info->status.bit.tx_mode == DMA_IO && !spi->info->xfer.dma_tx_done)
            spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        return (spi->info->xfer.tx_cnt);

    case SPI_RECEIVE:
        if (spi->info->status.bit.rx_mode == DMA_IO && !spi->info->xfer.dma_rx_done)
            spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
=======
        if (spi->info->status.bit.tx_mode == DMA_IO && !spi->info->xfer.dma_tx_done) {
            spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        #if SUPPORT_8BIT_DATA_MERGE
            if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                spi->info->xfer.tx_cnt <<= 2; // 8bit => 32bit
                if (spi->info->xfer.tx_cnt > spi->info->xfer.req_tx_cnt)
                    spi->info->xfer.tx_cnt = spi->info->xfer.req_tx_cnt;
            }
        #endif
        }
        return (spi->info->xfer.tx_cnt);

    case SPI_RECEIVE:
        if (spi->info->status.bit.rx_mode == DMA_IO && !spi->info->xfer.dma_rx_done) {
            spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
        #if SUPPORT_8BIT_DATA_MERGE
            if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                spi->info->xfer.rx_cnt <<= 2; // 8bit => 32bit
                if (spi->info->xfer.rx_cnt > spi->info->xfer.req_rx_cnt)
                    spi->info->xfer.rx_cnt = spi->info->xfer.req_rx_cnt;
            }
        #endif
        }
>>>>>>> feature/zephyr
        return (spi->info->xfer.rx_cnt);

    default:
        return CSK_DRIVER_OK;
    }
}


static void spi_abort_transfer(SPI_DEV *spi, bool keep_state)
{
    assert(spi != NULL);
<<<<<<< HEAD
=======
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
>>>>>>> feature/zephyr

    // if busy (Either TX or RX is ongoing), abort 1
    if (spi->info->status.bit.busy) {
        // disable SPI interrupts
        spi->reg->INTREN = 0;

        //clear the Ready bit in the SPI Slave Status Register
<<<<<<< HEAD
        if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
            spi->reg->SLVST &= ~SLVST_READY;
=======
        if (is_slave) {
            spi->reg->SLVST &= ~SLVST_READY;
            if ((spi->info->flags & SPI_FLAG_NO_CS) && spi->info->cb_set_cs != NULL)
               spi->info->cb_set_cs(spi->spi_idx, 1);
>>>>>>> feature/zephyr
        }

        if (spi->info->status.bit.tx_mode == DMA_IO) {
            if (!spi->info->xfer.dma_tx_done) {
                spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
<<<<<<< HEAD
                dma_channel_disable(spi->info->tx_dyn_dma_ch, true);
=======
                // only master can wait done, for CS & CLK is controlled by master...
                dma_channel_disable(spi->info->tx_dyn_dma_ch, !is_slave);
>>>>>>> feature/zephyr
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

    uint32_t clk, cs2sclk;
    int32_t sclk_div;
    clk = SPI_CLK(spi);

    if (clk == arg) { // output = input
        sclk_div = 0xFF;
    } else {
        sclk_div = (clk / (2 * arg)) - 1;
        if (sclk_div >= 0xFF || sclk_div < 0) {
<<<<<<< HEAD
            //CLOGW("%s: CANNOT support SCLK = %d (SPI_CLK = %d)! ", __func__, arg, clk);
=======
            CLOGW("%s: CANNOT support SCLK = %d (SPI_CLK = %d)! ", __func__, arg, clk);
>>>>>>> feature/zephyr
            return false;
        }
        clk /= (sclk_div + 1) * 2;
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
    LOGD("%s: sclk_div = %d (sclk: %d), cs2sclk = %d\r\n",
        __func__, (spi->reg->TIMING & 0xFF), clk, ((spi->reg->TIMING >> 12) & 0x3));
    return true;
}

static inline int32_t spi_get_bus_speed(SPI_DEV *spi)
{
    assert(spi != NULL);
    uint32_t sclk_div, spi_clk;
    spi_clk = SPI_CLK(spi);
    sclk_div = spi->reg->TIMING & 0xff;
    // NO divider when sclk_div is 0xFF
    return ( sclk_div == 0xFF ? spi_clk : spi_clk / ((sclk_div + 1) * 2));
}

// export SPI API function: SPI_Control
int32_t
SPI_Control(void *spi_dev, uint32_t control, uint32_t arg)
{
<<<<<<< HEAD
    uint32_t val;
=======
    uint32_t val, format;
>>>>>>> feature/zephyr
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

<<<<<<< HEAD
=======
        // reset SPI RX/TX FIFO
        case CSK_SPI_RESET_FIFO:
        {
            // clear RX/TX FIFOs
            uint32_t val = spi->reg->CTRL;
            if (arg == 0 || arg > 3)
                return CSK_DRIVER_ERROR_PARAMETER;
            if (arg & 0x1)  val |= RXFIFORST;
            if (arg & 0x2)  val |= TXFIFORST;
            spi->reg->CTRL = val;
            return CSK_DRIVER_OK;
        }

>>>>>>> feature/zephyr
        default:
            return CSK_DRIVER_ERROR_UNSUPPORTED;
    } // end exclusive MISC_OP

    if (spi->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

<<<<<<< HEAD
=======
    format = spi->reg->TRANSFMT; // original TRANSFMT value

>>>>>>> feature/zephyr
    // SPI Mode
    switch (control & CSK_SPI_MODE_Msk) {
    // Keep mode unchanged
    case CSK_SPI_MODE_UNSET:
        break;

    // SPI master (output on MOSI, input on MISO); arg = bus speed in bps
    case CSK_SPI_MODE_MASTER:
        // set master mode and disable data merge mode
<<<<<<< HEAD
        spi->reg->TRANSFMT &= ~(SPI_MERGE | SPI_SLAVE);
=======
        //spi->reg->TRANSFMT &= ~(SPI_MERGE | SPI_SLAVE);
        format &= ~(SPI_MERGE | SPI_SLAVE);
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
        spi->reg->TRANSFMT &= ~SPI_MERGE;
        spi->reg->TRANSFMT |= SPI_SLAVE;
=======
        //spi->reg->TRANSFMT &= ~SPI_MERGE;
        //spi->reg->TRANSFMT |= SPI_SLAVE;
        format &= ~SPI_MERGE;
        format |= SPI_SLAVE;
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
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
=======
        //spi->reg->TRANSFMT &= ~(SPI_CPOL | SPI_CPHA);
        format &= ~(SPI_CPOL | SPI_CPHA);
        break;
    case CSK_SPI_CPOL0_CPHA1:
        //spi->reg->TRANSFMT &= ~SPI_CPOL;
        //spi->reg->TRANSFMT |= SPI_CPHA;
        format &= ~SPI_CPOL;
        format |= SPI_CPHA;
        break;
    case CSK_SPI_CPOL1_CPHA0:
        //spi->reg->TRANSFMT |= SPI_CPOL;
        //spi->reg->TRANSFMT &= ~SPI_CPHA;
        format |= SPI_CPOL;
        format &= ~SPI_CPHA;
        break;
    case CSK_SPI_CPOL1_CPHA1:
        //spi->reg->TRANSFMT |= SPI_CPOL;
        //spi->reg->TRANSFMT |= SPI_CPHA;
        format |= SPI_CPOL;
        format |= SPI_CPHA;
>>>>>>> feature/zephyr
        break;
//    case CSK_SPI_TI_SSI:
//    case CSK_SPI_MICROWIRE:
    default:
        return CSK_SPI_ERROR_FRAME_FORMAT;
    }

    // SPI Data Bits
    val = ((control & CSK_SPI_DATA_BITS_Msk) >> CSK_SPI_DATA_BITS_Pos);
    if (val != 0) {
<<<<<<< HEAD
        if ((val < 1U) || (val > 32U))
            return CSK_SPI_ERROR_DATA_BITS;
        spi->info->data_bits = val;
        spi->reg->TRANSFMT &= ~DATA_BITS_MASK;
        spi->reg->TRANSFMT |= DATA_BITS(val);
=======
    #if SUPPORT_8BIT_DATA_MERGE
        if (val == 40U) { // means 8bit with DATA_MERGE (8bit=>32bit)
            spi->info->data_bits = 8;
            spi->info->flags |= SPI_FLAG_8BIT_MERGE;
            format |= SPI_MERGE;
        } else
    #endif
        if ((val < 1U) || (val > 32U)) {
            return CSK_SPI_ERROR_DATA_BITS;
        } else {
        spi->info->data_bits = val;
            spi->info->flags &= ~SPI_FLAG_8BIT_MERGE;
            format &= ~SPI_MERGE;
        }
        //spi->reg->TRANSFMT &= ~DATA_BITS_MASK;
        //spi->reg->TRANSFMT |= DATA_BITS(val);
        format &= ~DATA_BITS_MASK;
        format |= DATA_BITS(val);
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
        spi->reg->TRANSFMT |= SPI_LSB;
        break;
    case CSK_SPI_MSB_LSB:
        spi->reg->TRANSFMT &= ~SPI_LSB;
=======
        //spi->reg->TRANSFMT |= SPI_LSB;
        format |= SPI_LSB;
        break;
    case CSK_SPI_MSB_LSB:
        //spi->reg->TRANSFMT &= ~SPI_LSB;
        format &= ~SPI_LSB;
>>>>>>> feature/zephyr
        break;
    default:
        return CSK_SPI_ERROR_BIT_ORDER;
    }

<<<<<<< HEAD
=======
    // save changed TRANSFMT value
    spi->reg->TRANSFMT = format;

>>>>>>> feature/zephyr
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
<<<<<<< HEAD

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.tx_cnt = xfer_bytes;
        spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        spi->info->xfer.dma_tx_done = 1;
        //spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
=======
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);
//    CLOGD("%d t op = %d, %d\n", spi->spi_idx, spi->info->xfer.cur_op, spi->info->status.bit.rx_mode);

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        if (!spi->info->xfer.dma_tx_done) {
            spi->reg->CTRL &= ~TXDMAEN;
            spi->reg->INTREN &= ~SPI_TXFIFOURINT;

        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.tx_cnt = xfer_bytes;
        spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
        #if SUPPORT_8BIT_DATA_MERGE
            if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                spi->info->xfer.tx_cnt <<= 2; // 8bit => 32bit
                if (spi->info->xfer.tx_cnt > spi->info->xfer.req_tx_cnt)
                    spi->info->xfer.tx_cnt = spi->info->xfer.req_tx_cnt;
            }
        #endif

        //spi->info->tx_dyn_dma_ch = DMA_CHANNEL_ANY;
            spi->info->xfer.dma_tx_done = 1;
            spi->info->status.bit.tx_mode = NO_IO;
        }

        // in the case of no ENDINT interrupt
        if (spi->info->status.bit.no_endint) {
            // current SEND or XFER (with RECV is done)
            if (spi->info->xfer.cur_op == SPI_SEND ||
               (spi->info->xfer.cur_op == SPI_TRANSFER &&
                spi->info->status.bit.rx_mode == NO_IO)) {
                // disable SPI interrupts
                spi->reg->INTREN = 0;
                spi->info->status.bit.busy = 0U;
                // clear interrupt status
                spi->reg->INTRST = SPI_INT_MASK;

                if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
                    spi->info->cb_set_cs(spi->spi_idx, 1);

                if (spi->info->cb_event != NULL)
                    spi->info->cb_event(CSK_SPI_EVENT_TRANSFER_COMPLETE, spi->info->usr_param);
            }
        }
>>>>>>> feature/zephyr

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
<<<<<<< HEAD
    uint8_t event_type = event_info & 0xFF;
    SPI_DEV *spi = (SPI_DEV *)usr_param;

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.rx_cnt = xfer_bytes;
        spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
        spi->info->xfer.dma_rx_done = 1;
        //spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
=======
    SPI_DEV *spi = (SPI_DEV *)usr_param;
    uint8_t event_type = event_info & 0xFF;
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);
//    CLOGD("%d r cur_op = %d\n", spi->spi_idx, spi->info->xfer.cur_op);

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        if (!spi->info->xfer.dma_rx_done) {
            int32_t delta, thres;
            spi->reg->CTRL &= ~RXDMAEN;

        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        //spi->info->xfer.rx_cnt = xfer_bytes;
        spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
        #if SUPPORT_8BIT_DATA_MERGE
            if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                spi->info->xfer.rx_cnt <<= 2; // 8bit => 32bit
                if (spi->info->xfer.rx_cnt > spi->info->xfer.req_rx_cnt)
                    spi->info->xfer.rx_cnt = spi->info->xfer.req_rx_cnt;
                // the odd data should be received in PIO mode if no CS
                delta = (spi->info->xfer.req_rx_cnt - spi->info->xfer.rx_cnt + 3) >> 2;
                //delta = (spi->info->xfer.req_rx_cnt - spi->info->xfer.rx_cnt) >> 2;
            } else
        #endif
            // the odd data should be received in PIO mode if no CS
            delta = spi->info->xfer.req_rx_cnt - spi->info->xfer.rx_cnt;

            //spi->info->rx_dyn_dma_ch = DMA_CHANNEL_ANY;
        spi->info->xfer.dma_rx_done = 1;

            thres = spi->rxfifo_depth >> 1;
            if (delta <= 0 || delta >= thres) {
                spi->info->status.bit.rx_mode = NO_IO;
                spi->info->status.bit.rx_sync = 0;
                spi->reg->INTREN &= ~SPI_RXFIFOORINT;
            } else { // if (is_slave && no_cs)
                spi_set_rx_fifo_threshold(spi, 1);
                spi->reg->INTREN |= SPI_RXFIFOINT;
            }
        }

        // in the case of no ENDINT interrupt and RX is done
        if (spi->info->status.bit.no_endint && spi->info->status.bit.rx_mode == NO_IO) {
            // current RECV or XFER (with SEND is done)
            if (spi->info->xfer.cur_op == SPI_RECEIVE ||
               (spi->info->xfer.cur_op == SPI_TRANSFER &&
                spi->info->status.bit.tx_mode == NO_IO)) {
                // disable SPI interrupts
                spi->reg->INTREN = 0;
                spi->info->status.bit.busy = 0;
                // clear interrupt status
                spi->reg->INTRST = SPI_INT_MASK;

                if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
                    spi->info->cb_set_cs(spi->spi_idx, 1);

                if (spi->info->cb_event != NULL)
                    spi->info->cb_event(CSK_SPI_EVENT_TRANSFER_COMPLETE, spi->info->usr_param);
            }
        }

>>>>>>> feature/zephyr
        break;

    case DMA_EVENT_ERROR:
        //TODO: do something here?
        break;

    default:
        break;
    }
}


<<<<<<< HEAD
static void
spi_irq_handler(SPI_DEV *spi)
{
    uint32_t i, status; // j
    uint32_t data = 0;
    uint32_t rx_num = 0;
    uint32_t event = 0;
=======
_FAST_FUNC_RO static void
spi_irq_handler(SPI_DEV *spi)
{
    uint32_t i, status;
    uint32_t data = 0;
    uint32_t rx_num = 0;
    uint32_t event = 0;
    bool is_slave = ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE);
    bool no_cs = (spi->info->flags & SPI_FLAG_NO_CS);
>>>>>>> feature/zephyr

    // read status register
    status = spi->reg->INTRST;

<<<<<<< HEAD
    if ((status & SPI_RXFIFOORINT) || (status & SPI_TXFIFOURINT)) {
        // TX FIFO underrun or RX FIFO overrun interrupt status
        //spi->info->status.data_lost = 1U;
        if (status & SPI_RXFIFOORINT)
            spi->info->status.bit.data_ovf = 1U;
        if (status & SPI_TXFIFOURINT)
            spi->info->status.bit.data_unf = 1U;

        event |= CSK_SPI_EVENT_DATA_LOST;
    }
=======
    // RX FIFO overrun
    if (status & SPI_RXFIFOORINT) {
            spi->info->status.bit.data_ovf = 1U;
        event |= CSK_SPI_EVENT_DATA_LOST;
    }

    // TX FIFO underrun
    if (status & SPI_TXFIFOURINT) {
            spi->info->status.bit.data_unf = 1U;
        event |= CSK_SPI_EVENT_DATA_LOST;
    }

>>>>>>> feature/zephyr
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

<<<<<<< HEAD
=======

    if (status & SPI_TXFIFOINT) {
        spi_fill_tx_fifo(spi, spi->txfifo_depth >> 1); //
        if (spi->info->xfer.tx_cnt == spi->info->xfer.req_tx_cnt) {
            spi->reg->INTREN &= ~(SPI_TXFIFOINT | SPI_TXFIFOURINT);
            spi->info->status.bit.tx_mode = NO_IO;

            // in the case of no ENDINT interrupt and NOT (slave + no_cs)
            if (spi->info->status.bit.no_endint) { // && !(is_slave && no_cs)
                // current SEND or XFER (with RECV is done)
                if (spi->info->xfer.cur_op == SPI_SEND ||
                   (spi->info->xfer.cur_op == SPI_TRANSFER &&
                    spi->info->status.bit.rx_mode == NO_IO)) {
                    spi->reg->INTREN = 0;
                    spi->info->status.bit.busy = 0;

                    if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
                        spi->info->cb_set_cs(spi->spi_idx, 1);

                    event |= CSK_SPI_EVENT_TRANSFER_COMPLETE;
                }
            }
        }
    }

>>>>>>> feature/zephyr
    if (status & SPI_RXFIFOINT) {
        // get number of valid entries in the RX FIFO
        //rx_num = (spi->reg->STATUS >> 8) & 0x1f;
        i = SPI_RXFIFO_ENTRIES(spi);

<<<<<<< HEAD
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

=======
            if (spi->info->xfer.rx_cnt >= spi->info->xfer.req_rx_cnt) {
            LOGD("BSD: Warning!! More data received than requested!\n");
            //FIXME: read and throw away the unrequested data?
            for(; i > 0; i--)
                data = spi->reg->DATA;
        } else {
            for(; i > 0; i--) {
            data = spi->reg->DATA;

            // handle the data frame format
                if (spi->info->status.bit.rx_sync) {
                    spi->info->xfer.rx_data = data;
                    spi->info->xfer.rx_cnt++;
                #if SUPPORT_8BIT_DATA_MERGE
                    if (spi->info->flags & SPI_FLAG_8BIT_MERGE)
                        spi->info->xfer.rx_cnt += 3;
                #endif
                } else if (spi->info->data_bits <= 8) {
                uint8_t *rx_buf8 = (uint8_t *)spi->info->xfer.rx_buf;
                rx_buf8[spi->info->xfer.rx_cnt++] = data & 0xff;
                #if SUPPORT_8BIT_DATA_MERGE
                    if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 8) & 0xff;
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 16) & 0xff;
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 24) & 0xff;
                    }
                #endif
>>>>>>> feature/zephyr
            } else if (spi->info->data_bits <= 16) {
                uint16_t *rx_buf16 = (uint16_t *)spi->info->xfer.rx_buf;
                rx_buf16[spi->info->xfer.rx_cnt++] = data & 0xffff;
            } else {
                uint32_t *rx_buf32 = (uint32_t *)spi->info->xfer.rx_buf;
                rx_buf32[spi->info->xfer.rx_cnt++] = data;
            }

<<<<<<< HEAD
            // auto decrease, re-evaluate i if down to 0
            if (--i == 0)
                i = SPI_RXFIFO_ENTRIES(spi);
        } // end while i
    }

    if (status & SPI_TXFIFOINT) {
//        i = SPI_TXFIFO_ENTRIES(spi);
//        while ( i < spi->txfifo_depth) {
//
//            // auto increase i, re-evaluate i if up to TXFIFO depth
//            //if (++i == spi->txfifo_depth)
//            //    i = SPI_TXFIFO_ENTRIES(spi);
//
//            data = 0;
//            if (spi->info->xfer.tx_cnt < spi->info->xfer.req_tx_cnt) {
//                // handle the data frame format
//                if (spi->info->data_bits <= 8) { // data bits = 1....8
//                    uint8_t *tx_buf8 = (uint8_t *)spi->info->xfer.tx_buf;
//                    data = tx_buf8[spi->info->xfer.tx_cnt++];
//                } else if (spi->info->data_bits <= 16) { // data bits = 9....16
//                    uint16_t *tx_buf16 = (uint16_t *)spi->info->xfer.tx_buf;
//                    data = tx_buf16[spi->info->xfer.tx_cnt++];
//                } else { // data bits = 17....32
//                    uint32_t *tx_buf32 = (uint32_t *)spi->info->xfer.tx_buf;
//                    data = tx_buf32[spi->info->xfer.tx_cnt++];
//                }
//                spi->reg->DATA = data;
//                i++; //BSD:
//
//            } else {
//                spi->reg->INTREN &= ~SPI_TXFIFOINT;
//                break;
//            }
//        }

        spi_fill_tx_fifo(spi, spi->txfifo_depth);
        if (spi->info->xfer.tx_cnt == spi->info->xfer.req_tx_cnt)
            spi->reg->INTREN &= ~SPI_TXFIFOINT;
    }


=======
                if (spi->info->xfer.rx_cnt >= spi->info->xfer.req_rx_cnt) {
                    spi->reg->INTREN &= ~(SPI_RXFIFOINT | SPI_RXFIFOORINT);
                    spi->info->status.bit.rx_mode = NO_IO;
                    spi->info->status.bit.rx_sync = 0;

                    // in the case of no ENDINT interrupt
                    if (spi->info->status.bit.no_endint) {
                        // current RECV or XFER (with SEND is done)
                        if (spi->info->xfer.cur_op == SPI_RECEIVE ||
                           (spi->info->xfer.cur_op == SPI_TRANSFER &&
                            spi->info->status.bit.tx_mode == NO_IO)) {
                            spi->reg->INTREN = 0;
                            spi->info->status.bit.busy = 0;

                            if (is_slave && no_cs && spi->info->cb_set_cs != NULL)
                                spi->info->cb_set_cs(spi->spi_idx, 1);

                            event |= CSK_SPI_EVENT_TRANSFER_COMPLETE;
    }
                    }
                    break;
                }
            } // end for i

            int32_t thres = spi->rxfifo_depth >> 1;
            if (thres > 1) { // is_slave && no_cs &&
                int32_t delta = spi->info->xfer.req_rx_cnt - spi->info->xfer.rx_cnt;
            #if SUPPORT_8BIT_DATA_MERGE
                if (spi->info->flags & SPI_FLAG_8BIT_MERGE)
                    delta = (delta + 3) >> 2;
            #endif
                if (delta > 0 && delta < thres)
                    spi_set_rx_fifo_threshold(spi, 1);
            }
        }
    }

>>>>>>> feature/zephyr
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
<<<<<<< HEAD
                spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
=======
                //spi->info->xfer.tx_cnt = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
>>>>>>> feature/zephyr
                //NOTE: when CS is changed to LOW (ENDINT interrupt is triggered then) and TX FIFO is full,
                //  the data may remain in DMA channel FIFO and therefore never drain, so DON'T wait here!!
                dma_channel_disable(spi->info->tx_dyn_dma_ch, false); // true

                //FIXME: still wait some cycles?
                //for (i=0; i<50; i++) { // 100
                //    count = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
                //}
                count = dma_channel_get_count(spi->info->tx_dyn_dma_ch);
<<<<<<< HEAD
=======
            #if SUPPORT_8BIT_DATA_MERGE
                if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                    count <<= 2; // 8bit => 32bit
                    if (count > spi->info->xfer.req_tx_cnt)
                        count = spi->info->xfer.req_tx_cnt;
                }
            #endif
>>>>>>> feature/zephyr

                LOGD("BSD: dma tx data count: %d => %d\n", spi->info->xfer.tx_cnt, count);
                spi->info->xfer.tx_cnt = count;
                spi->info->xfer.dma_tx_done = 1;
            }

            // Check if there is remaining data left in TX FIFO!!
            //i = (spi->reg->STATUS >> 16) & 0x1f;
            i = SPI_TXFIFO_ENTRIES(spi);
            if (i > 0) {
<<<<<<< HEAD
=======

            #if SUPPORT_8BIT_DATA_MERGE
                if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) // && spi->info->status.bit.dma_merge
                    i <<= 2; //FIXME: maybe 4i, 4i-1, 4i-2 or 4i-3 hasn't been set? use which one?
            #endif

>>>>>>> feature/zephyr
                // The Data remained in TX FIFO is deducted from the total xfer count
                spi->info->xfer.tx_cnt -= i;
                LOGD("%s: Warning!! %d Data items remain in TX FIFO!\n",
                        __func__, i);
            }
<<<<<<< HEAD
            //FIXME:
            if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
=======
            //FIXME: [IP bug on VENUS_V2_MP] when data_bits = 1,
            // SLV_WRCNT() always return 0 even if TX DMA COUNT > 0 !!
            if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) { // && spi->info->data_bits > 1
>>>>>>> feature/zephyr
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
<<<<<<< HEAD
                spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
=======
                //spi->info->xfer.rx_cnt = dma_channel_get_count(spi->info->rx_dyn_dma_ch);

>>>>>>> feature/zephyr
                //wait, for data may be in SPI RX FIFO or DMA channel FIFO
                dma_channel_disable(spi->info->rx_dyn_dma_ch, true);

                //FIXME: still wait some cycles?
                //for (i=0; i<50; i++) { //TODO: decrease to 20? 50? 100?
                //    count = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
                //}
                count = dma_channel_get_count(spi->info->rx_dyn_dma_ch);
<<<<<<< HEAD
=======
            #if SUPPORT_8BIT_DATA_MERGE
                if ((spi->info->flags & SPI_FLAG_8BIT_MERGE)) { // && spi->info->status.bit.dma_merge
                    count <<= 2; // 8bit => 32bit
                    if (count > spi->info->xfer.req_rx_cnt)
                        count = spi->info->xfer.req_rx_cnt;
                }
            #endif
>>>>>>> feature/zephyr

                LOGD("BSD: dma rx data count: %d => %d\n", spi->info->xfer.rx_cnt, count);
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
<<<<<<< HEAD
=======
                    spi->info->xfer.rx_cnt = spi->info->xfer.req_rx_cnt;
>>>>>>> feature/zephyr
                    break;
                }
                data = spi->reg->DATA;
                // handle the data frame format
<<<<<<< HEAD
                if (spi->info->data_bits <= 8) {
                    uint8_t *rx_buf8 = (uint8_t *)spi->info->xfer.rx_buf;
                    rx_buf8[spi->info->xfer.rx_cnt++] = data & 0xff;
=======
                if (spi->info->status.bit.rx_sync) {
                    spi->info->xfer.rx_data = data;
                    spi->info->xfer.rx_cnt++;
                #if SUPPORT_8BIT_DATA_MERGE
                    if (spi->info->flags & SPI_FLAG_8BIT_MERGE)
                        spi->info->xfer.rx_cnt += 3;
                #endif
               } else if (spi->info->data_bits <= 8) {
                    uint8_t *rx_buf8 = (uint8_t *)spi->info->xfer.rx_buf;
                    rx_buf8[spi->info->xfer.rx_cnt++] = data & 0xff;
                #if SUPPORT_8BIT_DATA_MERGE
                    if (spi->info->flags & SPI_FLAG_8BIT_MERGE) {
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 8) & 0xff;
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 16) & 0xff;
                        if(spi->info->xfer.rx_cnt < spi->info->xfer.req_rx_cnt)
                            rx_buf8[spi->info->xfer.rx_cnt++] = (data >> 24) & 0xff;
                    }
                #endif
>>>>>>> feature/zephyr
                } else if (spi->info->data_bits <= 16){
                    uint16_t *rx_buf16 = (uint16_t *)spi->info->xfer.rx_buf;
                    rx_buf16[spi->info->xfer.rx_cnt++] = data & 0xffff;
               } else if (spi->info->data_bits <= 32){
                    uint32_t *rx_buf32 = (uint32_t *)spi->info->xfer.rx_buf;
                    rx_buf32[spi->info->xfer.rx_cnt++] = data;
                }
            } //end for
<<<<<<< HEAD
            //FIXME:
            // if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) {
            //     //count = spi->reg->SLVDATACNT & 0x1FF;
            //     count = SLV_RDCNT(spi);
            //     CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
            //     LOGD("%s: SLVDATACNT.RCnt = %d Data\n", __func__, count);
            //     spi->info->xfer.rx_cnt = count;
            // }
=======
            //FIXME: [IP bug on VENUS_V2_MP] when data_bits = 1,
            // SLV_RDCNT() always return 0 even if RX DMA COUNT > 0 !!
            if ((spi->info->txrx_mode & CSK_SPI_MODE_Msk) == CSK_SPI_MODE_SLAVE) { // && spi->info->data_bits > 1
                //count = spi->reg->SLVDATACNT & 0x1FF;
                count = SLV_RDCNT(spi);
                CLR_SLV_RDCNT(spi); // clear RCnt on updated IP
                LOGD("%s: SLVDATACNT.RCnt = %d Data\n", __func__, count);
                spi->info->xfer.rx_cnt = count;
            }
>>>>>>> feature/zephyr
        }

        // clear TX/RX FIFOs
        spi->reg->CTRL &= ~(TXDMAEN | RXDMAEN);
        spi->reg->CTRL |= (TXFIFORST | RXFIFORST);

<<<<<<< HEAD
        spi->info->status.bit.busy = 0U;
=======
        spi->info->status.bit.busy = 0;
        spi->info->status.bit.rx_sync = 0;
>>>>>>> feature/zephyr
        spi->info->status.bit.tx_mode = NO_IO;
        spi->info->status.bit.rx_mode = NO_IO;
        event |= CSK_SPI_EVENT_TRANSFER_COMPLETE;
    }

    // clear interrupt status
    spi->reg->INTRST = status;
    // make sure "write 1 clear" take effect before iret
    spi->reg->INTRST;

<<<<<<< HEAD
    if ((spi->info->cb_event != NULL) && (event != 0))
        spi->info->cb_event(event, spi->info->usr_param);

=======
    if ((spi->info->cb_event != NULL) && (event != 0)) {

        // disable SPI interrupts //FIXME: remove it??
        if (event & CSK_SPI_EVENT_TRANSFER_COMPLETE)
            spi->reg->INTREN = 0;

        spi->info->cb_event(event, spi->info->usr_param);
    }
>>>>>>> feature/zephyr
}
