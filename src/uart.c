/*
 * Copyright (c) 2013-2020 CSK Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uart.h"
#include "ClockManager.h"

 #define CSK_UART_DRV_VERSION    CSK_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */

 /* Driver Version */
 static const CSK_DRIVER_VERSION DriverVersion = {
     CSK_UART_API_VERSION,
     CSK_UART_DRV_VERSION
 };

#define CHECK_RESOURCES(res)  do{\
        if((res != &uart0_resources) && (res != &uart1_resources) && (res != &uart2_resources)){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)


/*
 * UART0 DEVICE
 * */
static void UART0_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART0_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART0_IRQ_Handler(void);

static UART_DMA uart0_tx_dma_info = {
        CSK_UART_TX_DMA_BSIZE_PARA,
        0,
        CSK_UART0_TX_DMA_CH,
        CSK_UART0_TX_DMA_REQSEL,
        UART0_DMA_TX_Handler,
};

static UART_DMA uart0_rx_dma_info = {
        CSK_UART_RX_DMA_BSIZE_PARA,
        0,
        CSK_UART0_RX_DMA_CH,
        CSK_UART0_RX_DMA_REQSEL,
        UART0_DMA_RX_Handler,
};

static UART_INFO uart0_info = { 0 };

static UART_RESOURCES uart0_resources = {
    IP_UART0,// REG
    IRQ_UART0_VECTOR,// IRQ NUM
    UART0_IRQ_Handler,// IRQ HANDLER
    CSK_UART0_FIFO,
    &uart0_tx_dma_info,// TX DMA INFO
    &uart0_rx_dma_info,// RX DMA INFO
    &uart0_info,// TRANS INFO
    (CSK_UART_AUTO_FLOW_EN_RTS | CSK_UART_AUTO_FLOW_EN_CTS),
};

/*
 * UART1 DEVICE
 * */

static void UART1_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART1_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART1_IRQ_Handler(void);

static UART_DMA uart1_tx_dma_info = {
        CSK_UART_TX_DMA_BSIZE_PARA,
        0,
        CSK_UART1_TX_DMA_CH,
        CSK_UART1_TX_DMA_REQSEL,
        UART1_DMA_TX_Handler,
};

static UART_DMA uart1_rx_dma_info = {
        CSK_UART_RX_DMA_BSIZE_PARA,
        0,
        CSK_UART1_RX_DMA_CH,
        CSK_UART1_RX_DMA_REQSEL,
        UART1_DMA_RX_Handler,
};

static UART_INFO uart1_info = { 0 };

static UART_RESOURCES uart1_resources = {
    IP_UART1,// REG
    IRQ_UART1_VECTOR,// IRQ NUM
    UART1_IRQ_Handler,// IRQ HANDLER
    CSK_UART1_FIFO,
    &uart1_tx_dma_info,// TX DMA INFO
    &uart1_rx_dma_info,// RX DMA INFO
    &uart1_info,// TRANS INFO
    (CSK_UART_AUTO_FLOW_EN_RTS | CSK_UART_AUTO_FLOW_EN_CTS),
};

/*
 * UART2 DEVICE
 * */

static void UART2_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART2_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static void UART2_IRQ_Handler(void);

static UART_DMA uart2_tx_dma_info = {
        CSK_UART_TX_DMA_BSIZE_PARA,
        0,
        CSK_UART2_TX_DMA_CH,
        CSK_UART2_TX_DMA_REQSEL,
        UART2_DMA_TX_Handler,
};

static UART_DMA uart2_rx_dma_info = {
        CSK_UART_RX_DMA_BSIZE_PARA,
        0,
        CSK_UART2_RX_DMA_CH,
        CSK_UART2_RX_DMA_REQSEL,
        UART2_DMA_RX_Handler,
};

static UART_INFO uart2_info = { 0 };

static UART_RESOURCES uart2_resources = {
    IP_UART2,// REG
    IRQ_UART2_VECTOR,// IRQ NUM
    UART2_IRQ_Handler,// IRQ HANDLER
    CSK_UART2_FIFO,
    &uart2_tx_dma_info,// TX DMA INFO
    &uart2_rx_dma_info,// RX DMA INFO
    &uart2_info,// TRANS INFO
    (CSK_UART_AUTO_FLOW_EN_RTS | CSK_UART_AUTO_FLOW_EN_CTS),
};

/*
 * DEVICE DESCRIPTION FUNCTION
 * */

void* UART0(void){
    return (void*)&uart0_resources;
}

void* UART1(void){
    return (void*)&uart1_resources;
}

void* UART2(void){
    return (void*)&uart2_resources;
}

//
//   Internal Functions
//
#if defined(CSK6001_MPW)
static inline uint32_t
uart_compute_div(uint32_t baudrate, uint32_t clk){
    uint32_t tmp = 16 * baudrate;
    uint32_t div = clk / tmp;
    uint32_t div2 = div + 1;
    int32_t err = clk - div * tmp;
    int32_t err2 = div2 * tmp - clk;
    if (err2 < 0) {
        err2 = err2 * (-1);
    }

    return err > err2 ? div2 : div;
}
#endif

#if defined(CSK6001)
static inline float
uart_compute_div(uint32_t baudrate, uint32_t clk){
    uint32_t tmp = 16 * baudrate;
    // 获得整数部分
    uint32_t div_i = clk / tmp;
    // 获得小数部分
    float div_f = ((float)clk / (float)tmp) - div_i;
    float div1 = div_i + (float)((uint32_t)(div_f * 16)) / 16.0;
    float div2 = div_i + (float)((uint32_t)(div_f * 16) + 1) / 16.0;

    int32_t err = clk - div1 * tmp;
    int32_t err2 = div2 * tmp - clk;
    if (err2 < 0) {
        err2 = err2 * (-1);
    }

    return err > err2 ? div2 : div1;
}
#endif
//
//   Functions
//

 CSK_DRIVER_VERSION UART_GetVersion(void)
 {
   return DriverVersion;
 }

int32_t UART_Initialize(void *res, CSK_UART_SignalEvent_t cb_event, void* workspace)
{

    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if (uart->info->flags & UART_FLAG_INITIALIZED) {
        // Driver is already initialized
        return CSK_DRIVER_OK;
    }

    // Initialize UART Run-time Resources
    uart->info->cb_event = cb_event;

    uart->info->workspace = workspace;

    uart->info->rx_status.rx_busy = 0U;
    uart->info->rx_status.rx_overflow = 0U;
    uart->info->rx_status.rx_break = 0U;
    uart->info->rx_status.rx_framing_error = 0U;
    uart->info->rx_status.rx_parity_error = 0U;

    uart->info->xfer.send_active = 0U;
    uart->info->xfer.tx_def_val = 0U;
    uart->info->xfer.rx_num = 0U;
    uart->info->xfer.tx_num = 0U;
    uart->info->xfer.rx_buf = NULL;
    uart->info->xfer.tx_buf = NULL;
    uart->info->xfer.rx_cnt = 0U;
    uart->info->xfer.tx_cnt = 0U;

    uart->info->tx_trig_lvl = CSK_UART_TX_TRIG_LVL;
    uart->info->rx_trig_lvl = CSK_UART_RX_TRIG_LVL;
    uart->info->baudrate = 0U;
    uart->info->mode = 0U;

    uart->info->timeout = 0U;
    uart->info->inter_en = 0U;

    uart->info->flags = UART_FLAG_INITIALIZED;

    return CSK_DRIVER_OK;
}

int32_t UART_Uninitialize(void *res)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if (uart->info->flags == 0) {
        // Driver is already initialized
        return CSK_DRIVER_OK;
    }

    // DMA Uninitialize
    if (!uart->info->inter_en) {
        dma_uninitialize();
    }

    // Reset UART status flags
    uart->info->cb_event = NULL;

    uart->info->workspace = NULL;

    uart->info->rx_status.rx_busy = 0U;
    uart->info->rx_status.rx_overflow = 0U;
    uart->info->rx_status.rx_break = 0U;
    uart->info->rx_status.rx_framing_error = 0U;
    uart->info->rx_status.rx_parity_error = 0U;

    uart->info->xfer.send_active = 0U;
    uart->info->xfer.tx_def_val = 0U;
    uart->info->xfer.rx_num = 0U;
    uart->info->xfer.tx_num = 0U;
    uart->info->xfer.rx_buf = NULL;
    uart->info->xfer.tx_buf = NULL;
    uart->info->xfer.rx_cnt = 0U;
    uart->info->xfer.tx_cnt = 0U;

    uart->info->tx_trig_lvl = CSK_UART_TX_TRIG_LVL;
    uart->info->rx_trig_lvl = CSK_UART_RX_TRIG_LVL;
    uart->info->baudrate = 0U;
    uart->info->mode = 0U;
    uart->info->flags = 0U;

    uart->info->timeout = 0U;
    uart->info->inter_en = 0U;

    return CSK_DRIVER_OK;
}

int32_t UART_PowerControl(void *res, CSK_POWER_STATE state)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    switch (state)
    {
    case CSK_POWER_OFF:
        if ((uart->info->flags & UART_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }

        //需要考虑当用户立刻停止，需要让当前数据发送完毕才能重新reset uart
        //所以这里存在的问题在于，如果当前只是发送，而接收缓存中又存在数据，则会导致
        //wait busy无法通过，程序会卡死在这里

        {
            uint32_t i = 0;

            if(uart->info->baudrate){
                i = ((uart->fifo_d * 10) / uart->info->baudrate) * CRM_GetApFreq();
            } else{
                i = 0;
            }

            do{
                if (uart->reg->REG_USR.bit.BUSY == 0){
                    break;
                }
            }while(i--);
        }

        uart->reg->REG_FCR.all |= 0x06;

        while((uart->reg->REG_USR.all & 0x1) == 0x1);

        // Disable UART IRQ
        disable_IRQ(uart->irq_num);

        if (!(uart->info->inter_en) && (uart->info->xfer.send_active != 0U)){
            dma_channel_disable(uart->dma_tx->channel, 0);
        }

        if (!(uart->info->inter_en) && (uart->info->rx_status.rx_busy)){
            dma_channel_disable(uart->dma_rx->channel, 0);
        }

        if (uart == &uart0_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.UART0_RESET = 0x1;
            __HAL_CRM_UART0_CLK_DISABLE();
        }else if (uart == &uart1_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.UART1_RESET = 0x1;
            __HAL_CRM_UART1_CLK_DISABLE();
        }else if (uart == &uart2_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.UART2_RESET = 0x1;
            __HAL_CRM_UART2_CLK_DISABLE();
        }else {
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        // Clear driver variables
        uart->info->rx_status.rx_busy = 0U;
        uart->info->rx_status.rx_overflow = 0U;
        uart->info->rx_status.rx_break = 0U;
        uart->info->rx_status.rx_framing_error = 0U;
        uart->info->rx_status.rx_parity_error = 0U;
        uart->info->xfer.send_active = 0U;

        uart->info->flags &= ~UART_FLAG_POWERED;

        // Uninstall IRQ Handler
        register_ISR(uart->irq_num, NULL, NULL);

        break;

    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if ((uart->info->flags & UART_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }
        if ((uart->info->flags & UART_FLAG_POWERED) != 0U) {
            return CSK_DRIVER_OK;
        }

        // TODO
        // Power and clock manager
        // Reset current peripheral
        if (uart == &uart0_resources){
            __HAL_CRM_UART0_CLK_ENABLE();

            IP_SYSCTRL->REG_SW_RESET2.bit.UART0_RESET = 0x1;
        }else if (uart == &uart1_resources){
            __HAL_CRM_UART1_CLK_ENABLE();

            IP_SYSCTRL->REG_SW_RESET2.bit.UART1_RESET = 0x1;
        }else if (uart == &uart2_resources){
            __HAL_CRM_UART2_CLK_ENABLE();

            IP_SYSCTRL->REG_SW_RESET2.bit.UART2_RESET = 0x1;
        }else {
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        // Disable interrupts
        uart->reg->REG_IER.all = UARTC_IER_DISABLE;

        // Enable modem interrupt
        if (uart->modem){
            uart->reg->REG_IER.all |= UARTC_IER_MS;
        }        

        uart->info->flags = UART_FLAG_POWERED | UART_FLAG_INITIALIZED;

        break;
    }
    return CSK_DRIVER_OK;
}

int32_t UART_Send(void *res, const void *data, uint32_t num)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if ((data == NULL) || (num == 0U)) {
        // Invalid parameters
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((uart->info->flags & UART_FLAG_CONFIGURED) == 0U) {
        // UART is not configured (mode not selected)
        return CSK_DRIVER_ERROR;
    }

    if (uart->info->xfer.send_active != 0U) {
        // Send is not completed yet
        return CSK_DRIVER_ERROR_BUSY;
    }

    // Set Send active flag
    uart->info->xfer.send_active = 1U;

    // Save transmit buffer info
    uart->info->xfer.tx_buf = (uint8_t *) data;
    uart->info->xfer.tx_num = num;
    uart->info->xfer.tx_cnt = 0U;

    // DMA transmit
    if (!uart->info->inter_en){
        int32_t stat;

        dma_channel_select(
            &uart->dma_tx->channel,
            uart->dma_tx->cb_event,
            0,
            DMA_CACHE_SYNC_SRC);
        if (uart->dma_tx->channel == DMA_CHANNEL_ANY) {
            return CSK_DRIVER_ERROR;
        }

        stat = dma_channel_configure (uart->dma_tx->channel,
                    (uint32_t) uart->info->xfer.tx_buf,
                    (uint32_t) (&(uart->reg->REG_THR.all)),
                    uart->info->xfer.tx_num,
                    DMA_CH_CTLL_DST_WIDTH(CSK_UART_TX_DMA_WIDTH_PARA) | DMA_CH_CTLL_SRC_WIDTH(CSK_UART_TX_DMA_WIDTH_PARA) |\
                  DMA_CH_CTLL_DST_BSIZE(uart->dma_tx->bsize_para) | DMA_CH_CTLL_SRC_BSIZE(uart->dma_tx->bsize_para) |\
                  DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |\
                  DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                  DMA_CH_CFGL_CH_PRIOR(1), // config_low
                  DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(uart->dma_tx->reqsel), // config_high
                  0, 0);

        uart->dma_tx->en = 1;

        if(stat == -1){
            return CSK_DRIVER_ERROR;
        }

    } else{
        uint32_t val;
        // Fill TX FIFO
        if (uart->reg->REG_LSR.all & UARTC_LSR_THRE) {

            // TODO depend on FIFO depth and threashold
            val = uart->fifo_d;

            while ((val--)&& (uart->info->xfer.tx_cnt != uart->info->xfer.tx_num)) {
                uart->reg->REG_THR.all =uart->info->xfer.tx_buf[uart->info->xfer.tx_cnt++];
            }
        }

        // Enable transmit holding register empty interrupt
        uart->reg->REG_IER.all |= UARTC_IER_THRE;
    }

    return CSK_DRIVER_OK;
}

int32_t UART_Receive(void *res, void *data, uint32_t num)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if ((data == NULL) || (num == 0U)) {
        // Invalid parameters
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((uart->info->flags & UART_FLAG_CONFIGURED) == 0U) {
        // UART is not configured (mode not selected)
        return CSK_DRIVER_ERROR;
    }

    // Check if receiver is busy
    if (uart->info->rx_status.rx_busy == 1U) {
        return CSK_DRIVER_ERROR_BUSY;
    }

    // Set RX busy flag
    uart->info->rx_status.rx_busy = 1U;

    // Save number of data to be received
    uart->info->xfer.rx_num = num;

    // Clear RX statuses
    uart->info->rx_status.rx_break = 0U;
    uart->info->rx_status.rx_framing_error = 0U;
    uart->info->rx_status.rx_overflow = 0U;
    uart->info->rx_status.rx_parity_error = 0U;

    // Save receive buffer info
    uart->info->xfer.rx_buf = (uint8_t *) data;
    uart->info->xfer.rx_cnt = 0U;

    // DMA transmit
    if (!uart->info->inter_en){
        int32_t stat;

        dma_channel_select(
            &uart->dma_rx->channel,
            uart->dma_rx->cb_event,
            0,
            DMA_CACHE_SYNC_DST);
        if (uart->dma_rx->channel == DMA_CHANNEL_ANY) {
            return CSK_DRIVER_ERROR;
        }

        stat = dma_channel_configure (uart->dma_rx->channel,
                    (uint32_t) (&(uart->reg->REG_RBR.all)),
                    (uint32_t) uart->info->xfer.rx_buf,
                    uart->info->xfer.rx_num,
                    DMA_CH_CTLL_DST_WIDTH(CSK_UART_RX_DMA_WIDTH_PARA) | DMA_CH_CTLL_SRC_WIDTH(CSK_UART_RX_DMA_WIDTH_PARA) |\
                  DMA_CH_CTLL_DST_BSIZE(uart->dma_rx->bsize_para) | DMA_CH_CTLL_SRC_BSIZE(uart->dma_rx->bsize_para) |\
                  DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |\
                  DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                  DMA_CH_CFGL_CH_PRIOR(1), // config_low
                  DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(uart->dma_rx->reqsel), // config_high
                  0, 0);

        uart->dma_rx->en = 1;

        if (uart->info->timeout){
            uart->reg->REG_IER.bit.ECHT = 0x1;
        }

        if(stat == -1){
            return CSK_DRIVER_ERROR;
        }
    } else{
        // Enable receive data available interrupt
        uart->reg->REG_IER.all |= UARTC_IER_RDR;
#if defined(CSK6001)
        // avoid a small amount of data that does not reach the threshold
        uart->reg->REG_IER.bit.ECHT = 0x1;
#endif
    }

    return CSK_DRIVER_OK;
}

uint32_t UART_GetTxCount(void *res)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if ((!uart->info->inter_en) && uart->dma_tx && uart->info->xfer.send_active && uart->dma_tx->en) {
        uart->info->xfer.tx_cnt = dma_channel_get_count(uart->dma_tx->channel);
    }

    return uart->info->xfer.tx_cnt;
}

uint32_t UART_GetRxCount(void *res)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if ((!uart->info->inter_en) && uart->dma_rx && uart->info->rx_status.rx_busy && uart->dma_rx->en) {
        uart->info->xfer.rx_cnt = dma_channel_get_count(uart->dma_rx->channel);
    }

    return uart->info->xfer.rx_cnt;
}

int32_t UART_Control(void *res, uint32_t control, uint32_t arg)
{    
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    if ((uart->info->flags & UART_FLAG_POWERED) == 0U) {
        // UART not powered
        return CSK_DRIVER_ERROR;
    }
    // looptest
    // data on the sout line is held high, while serial data output is looped back to the sin line, internally

    // Set uart modem
    switch (control & CSK_UART_CONTROL_Msk) {
    // Control TX
    case CSK_UART_CONTROL_TX:
        
        if (arg) {
            uart->info->flags |= UART_FLAG_TX_ENABLED;
        } else {
            uart->info->flags &= ~UART_FLAG_TX_ENABLED;
        }

        return CSK_DRIVER_OK;

    // Control RX
    case CSK_UART_CONTROL_RX:

        if (arg) {
            uart->info->flags |= UART_FLAG_RX_ENABLED;
        } else {
            uart->info->flags &= ~UART_FLAG_RX_ENABLED;
        }

        return CSK_DRIVER_OK;

        // Abort Send
    case CSK_UART_ABORT_SEND:
        // Disable transmit holding register empty interrupt
        uart->reg->REG_IER.all &= ~UARTC_IER_THRE;

        // If DMA mode - disable DMA channel
        if ((!uart->info->inter_en) && (uart->info->xfer.send_active != 0U) && uart->dma_tx->en) {
            uart->info->xfer.tx_cnt = dma_channel_get_count(uart->dma_tx->channel);

            //dma_channel_disable(uart->dma_tx->channel);
            dma_channel_disable(uart->dma_tx->channel, 1);
            uart->dma_tx->en = 0;
        }

        uart->info->xfer.tx_cnt = uart->info->xfer.tx_cnt > uart->reg->REG_TFL.all ?\
                uart->info->xfer.tx_cnt - uart->reg->REG_TFL.all : 0;
        // Transmit FIFO reset
        uart->reg->REG_FCR.all |= 0x04;

        // Clear Send active flag
        uart->info->xfer.send_active = 0U;

        return CSK_DRIVER_OK;

        // Abort receive
    case CSK_UART_ABORT_RECEIVE:
        // Disable receive data available interrupt
        uart->reg->REG_IER.all &= ~UARTC_IER_RDR;
        uart->reg->REG_IER.bit.ECHT = 0x0;

        // Receive FIFO reset
        uart->reg->REG_FCR.all |= 0x02;

        // If DMA mode - disable DMA channel
        if ((!uart->info->inter_en) && (uart->info->rx_status.rx_busy) && uart->dma_rx->en) {
            uart->info->xfer.rx_cnt = dma_channel_get_count(uart->dma_rx->channel);

            //dma_channel_disable(uart->dma_rx->channel);
            dma_channel_disable(uart->dma_rx->channel, 1);
            uart->dma_rx->en = 0;
        }

        // Clear RX busy status
        uart->info->rx_status.rx_busy = 0U;

        return CSK_DRIVER_OK;

    default:
        break;
    }

    //**********
    // Set FCR
    // (optional) reset FIFO
    // Receive trigger level
    // Transmit trigger level
    // Enable FIFO
    // Configure FIFO
    {
        uint32_t val = 0;

        val = CSK_UART_TX_TRIG_LVL_PARA | UARTC_FCR_RFIFO_RESET |\
                UARTC_FCR_TFIFO_RESET | UARTC_FCR_FIFO_EN;

        switch (control & CSK_UART_Function_CONTROL_Msk) {
            case CSK_UART_Function_CONTROL_Dma:
                val |= UARTC_FCR_DMA_EN;
                uart->info->inter_en = 0;
                // DMA initialize
                dma_initialize();
                break;
            case CSK_UART_Function_CONTROL_Int:
                uart->info->inter_en = 1;
                break;
        }

        uint32_t mode = 0;
        switch (control & CSK_UART_CONTROL_Msk) {
        case CSK_UART_MODE_ASYNCHRONOUS:
            mode = CSK_UART_MODE_ASYNCHRONOUS;
            uart->info->timeout = 0;
            // Enable DMA
            if (!uart->info->inter_en){
                // DMA transmit must set threshold to 1 character
                uart->info->rx_trig_lvl = CSK_UART_RX_TRIG_LVL_0;
                val |= UARTC_FCR_RFIFO_TRGL0;
                uart->dma_rx->bsize_para = DMA_BSIZE_1;
            } else {
                val |= CSK_UART_RX_TRIG_LVL_PARA;
            }
            break;

        case CSK_UART_MODE_ASYNCHRONOUS_TIMEOUT:
            mode = CSK_UART_MODE_ASYNCHRONOUS_TIMEOUT;
            uart->info->timeout = 1;
            uart->info->rx_trig_lvl = CSK_UART_RX_TRIG_LVL;
            val |= CSK_UART_RX_TRIG_LVL_PARA;
            uart->dma_rx->bsize_para = CSK_UART_RX_DMA_BSIZE_PARA;
            break;

            // Default TX value
        case CSK_UART_SET_DEFAULT_TX_VALUE:
            uart->info->xfer.tx_def_val = arg;
            mode = CSK_UART_SET_DEFAULT_TX_VALUE;
            uart->info->mode = mode;
            return CSK_DRIVER_OK;
        default:
            return CSK_DRIVER_ERROR_UNSUPPORTED;
        }

        // FIFO configure
        // Default enable FIFO mode and set tx/rx trigger level,
        // the DMA mode depend if support it
        uart->reg->REG_FCR.all = val;

    }

    // **************************************************
    // Below configure in LCR[6:0]
    // SP & EPS & PEN
    // Parity check
    // Stop bit
    // Data length
    // UART Data bits
    
    uint32_t lcr = 0;

    switch (control & CSK_UART_DATA_BITS_Msk) {
    case CSK_UART_DATA_BITS_5:
        lcr |= UARTC_LCR_BITS5;
        break;
    case CSK_UART_DATA_BITS_6:
        lcr |= UARTC_LCR_BITS6;
        break;
    case CSK_UART_DATA_BITS_7:
        lcr |= UARTC_LCR_BITS7;
        break;
    // default
    case CSK_UART_DATA_BITS_8:
        lcr |= UARTC_LCR_BITS8;
        break;
    default:
        return CSK_UART_ERROR_DATA_BITS;
    }

    // UART Parity
    switch (control & CSK_UART_PARITY_Msk) {
    // default
    case CSK_UART_PARITY_NONE:
        break;
    case CSK_UART_PARITY_EVEN:
        lcr |= UARTC_LCR_PARITY_EVEN;
        break;
    case CSK_UART_PARITY_ODD:
        lcr |= UARTC_LCR_PARITY_ODD;
        break;
    default:
        return (CSK_UART_ERROR_PARITY);
    }

    // UART Stop bits
    switch (control & CSK_UART_STOP_BITS_Msk) {
    // default
    case CSK_UART_STOP_BITS_1:
        uart->reg->REG_LCR.bit.STOP = 0x0;
        break;
    case CSK_UART_STOP_BITS_2:
        // TODO 1.5 and 2
        uart->reg->REG_LCR.bit.STOP = 0x1;
        break;
    case CSK_UART_STOP_BITS_1_5:
        // 1.5 stop bit transmit when only data bit = 5
        if(control & CSK_UART_DATA_BITS_5){
            uart->reg->REG_LCR.bit.STOP = 0x1;
        }else{
            return CSK_UART_ERROR_STOP_BITS;
        }
        break;
    default:
        return CSK_UART_ERROR_STOP_BITS;
    }
    

    // **************************************************
    // set baud rate
    // LCR[7] = 1
    // (optional) DLF for fractional
    // set DLL & DLH
    // LCR[7] = 0
    {        
#if defined(CSK6001_MPW)
        uint32_t div;
#else
        float div, div_f;
        uint32_t div_i;
#endif
        // compute the baudrate div
        // optional can compute the fractional part
        if (uart == &uart0_resources){
            div = uart_compute_div(arg, CRM_GetUart0Freq());
        }else if (uart == &uart1_resources){
            div = uart_compute_div(arg, CRM_GetUart1Freq());
        }else if (uart == &uart2_resources){
            div = uart_compute_div(arg, CRM_GetUart2Freq());
        }else {
            return CSK_DRIVER_ERROR_PARAMETER;
        }
#if defined(CSK6001_MPW)
        uart->reg->REG_LCR.all |= UARTC_LCR_DLAB;

        uart->reg->REG_DLL.all = (div >> 0) & 0xff;
        uart->reg->REG_DLH.all = (div >> 8) & 0xff;
        // Reset DLAB bit
        uart->reg->REG_LCR.all &= (~UARTC_LCR_DLAB);

        uart->info->baudrate = arg;
#else
        div_i = (uint32_t)div;
        div_f = (div - div_i) * 16;

        uart->reg->REG_LCR.all |= UARTC_LCR_DLAB;

        uart->reg->REG_DLL.all = (div_i >> 0) & 0xff;
        uart->reg->REG_DLH.all = (div_i >> 8) & 0xff;
        uart->reg->REG_DLF.bit.DLF = div_f;
        // Reset DLAB bit
        uart->reg->REG_LCR.all &= (~UARTC_LCR_DLAB);

        uart->info->baudrate = arg;
#endif
    }

    {
        uint32_t mcr = 0;
        // auto flow
        mcr = uart->reg->REG_MCR.all & ~UARTC_MCR_AFE;
        switch (control & CSK_UART_FLOW_CONTROL_Msk) {
        case CSK_UART_FLOW_CONTROL_NONE:
            break;
        case CSK_UART_FLOW_CONTROL_RTS:
            if (uart->modem & CSK_UART_AUTO_FLOW_EN_RTS) {

                mcr |= UARTC_MCR_AFE | UARTC_MCR_RTS;
            }
            else {
                return CSK_UART_ERROR_FLOW_CONTROL;
            }
            break;
        case CSK_UART_FLOW_CONTROL_CTS:
            if (uart->modem & CSK_UART_AUTO_FLOW_EN_CTS) {

                mcr |= UARTC_MCR_AFE;
            } else {
                return CSK_UART_ERROR_FLOW_CONTROL;
            }
            break;
        case CSK_UART_FLOW_CONTROL_RTS_CTS:
            if (uart->modem & (CSK_UART_AUTO_FLOW_EN_RTS | CSK_UART_AUTO_FLOW_EN_CTS)) {

                mcr |= UARTC_MCR_AFE | UARTC_MCR_RTS;
            } else {
                return CSK_UART_ERROR_FLOW_CONTROL;
            }
            break;
        default:
            {
                return CSK_UART_ERROR_FLOW_CONTROL;
            }
        }

        // Configure MCR register
        uart->reg->REG_MCR.all = (uart->reg->REG_MCR.all & ~UARTC_MCR_AFE) | mcr;
    }

    // Configure Line control register
    uart->reg->REG_LCR.all = ((uart->reg->REG_LCR.all & (UARTC_LCR_SETBREAK | UARTC_LCR_DLAB)) | lcr);

    // Set configured flag
    uart->info->flags |= UART_FLAG_CONFIGURED;

    return CSK_DRIVER_OK;
}

int32_t UART_GetStatus(void *res, CSK_UART_STATUS* stat)
{
    CHECK_RESOURCES(res);

    UART_RESOURCES* uart = (UART_RESOURCES*)res;

    stat->tx_busy = (uart->reg->REG_LSR.all & UARTC_LSR_TEMT ? (0U) : (1U));
    stat->rx_busy = uart->info->rx_status.rx_busy;
    stat->tx_underflow = 0U;
    stat->rx_overflow = uart->info->rx_status.rx_overflow;
    stat->rx_break = uart->info->rx_status.rx_break;
    stat->rx_framing_error = uart->info->rx_status.rx_framing_error;
    stat->rx_parity_error = uart->info->rx_status.rx_parity_error;

    return CSK_DRIVER_OK;
}

static uint32_t
uart_rxline_irq_handler(UART_RESOURCES *uart)
{
    uint32_t lsr, event;

#define UARTC_LSR_LINE_INT      (UARTC_LSR_OE | UARTC_LSR_PE | UARTC_LSR_FE | UARTC_LSR_BI)

    event = 0U;

    lsr = uart->reg->REG_LSR.all & UARTC_LSR_LINE_INT;

    // OverRun error
    if (lsr & UARTC_LSR_OE) {
        uart->info->rx_status.rx_overflow = 1U;
        event |= CSK_UART_EVENT_RX_OVERFLOW;
    }

    // Parity error
    if (lsr & UARTC_LSR_PE) {
        uart->info->rx_status.rx_parity_error = 1U;
        event |= CSK_UART_EVENT_RX_PARITY_ERROR;
    }

    // Break detected
    if (lsr & UARTC_LSR_BI) {
        uart->info->rx_status.rx_break = 1U;
        event |= CSK_UART_EVENT_RX_BREAK;
    }

    // Framing error
    else {
        if (lsr & UARTC_LSR_FE) {
            uart->info->rx_status.rx_framing_error = 1U;
            event |= CSK_UART_EVENT_RX_FRAMING_ERROR;
        }
    }

    return event;
}


static void UART_IRQ_Handler(UART_RESOURCES *uart)
{
    uint32_t iir, event, val, i;

    event = 0U;
    // Get interrupt status
    iir = uart->reg->REG_IIR.all;

    // 无中断产生
    if ((iir & UARTC_IIR_NONE) == 0U) {

        // Transmit holding register empty
        // It depend on threshold
        if ((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_THRE) {

            // TODO This is very importance
            // Get the residue FIFO data
            // Fill data in FIFO until full
            val = uart->fifo_d - uart->reg->REG_TFL.all;

            // 传输FIFO residue 个字符，并确保是否传输完成
            while ((val--) && (uart->info->xfer.tx_num != uart->info->xfer.tx_cnt)) {
                // Write data to Tx FIFO
                uart->reg->REG_THR.all =
                        uart->info->xfer.tx_buf[uart->info->xfer.tx_cnt];
                uart->info->xfer.tx_cnt++;
            }

            // Check if all data is transmitted
            if (uart->info->xfer.tx_num == uart->info->xfer.tx_cnt) {
                // Disable THRE interrupt
                uart->reg->REG_IER.all &= ~UARTC_IER_THRE;

                // Clear TX busy flag
                uart->info->xfer.send_active = 0U;

                // Set send complete event
                event |= CSK_UART_EVENT_SEND_COMPLETE;
            }
        }

        // 先不做这些操作
        // 奇偶校验
        // Receive line status
        // Only trigger line status
        if ((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_RLS) {
            event |= uart_rxline_irq_handler(uart);
        }

        // Receive data available and Character time-out indicator interrupt
        // 即使出现了TIME OUT 也需要把剩下的数据拷贝出来
        if (((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_RDA) ||
                ((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_RTO)) {

#if defined(CSK6001)
            // for dma mode
            if (uart->info->inter_en == 0){
                uart->info->xfer.rx_cnt = dma_channel_get_count(uart->dma_rx->channel);

                // abort dma receive
                dma_channel_disable(uart->dma_rx->channel, 1);
                uart->dma_rx->en = 0;
            }
#endif

            // Read RX FIFO residue
            // This step is very importance, because it can promise to trigger uart time-out interrupt
            i = uart->info->rx_trig_lvl == 1 ? uart->info->rx_trig_lvl : (uart->info->rx_trig_lvl - 1);

            // Get available data from RX FIFO
            while ((uart->reg->REG_LSR.all & UARTC_LSR_RDR) && (i--)) {
                // Check RX line interrupt for errors
                event |= uart_rxline_irq_handler(uart);

                // Read data from RX FIFO into receive buffer
                uart->info->xfer.rx_buf[uart->info->xfer.rx_cnt] =
                        (uint8_t)(uart->reg->REG_RBR.all & 0xFF);

                uart->info->xfer.rx_cnt++;

                // Check if requested amount of data is received
                if (uart->info->xfer.rx_cnt == uart->info->xfer.rx_num) {
                    // Disable RDA interrupt
                    uart->reg->REG_IER.all &= ~UARTC_IER_RDR;
#if defined(CSK6001)
                    uart->reg->REG_IER.bit.ECHT = 0x0;
#endif

                    // Clear RX busy flag and set receive transfer complete event
                    uart->info->rx_status.rx_busy = 0U;
                    event |= CSK_UART_EVENT_RECEIVE_COMPLETE;

                    break;
                }
            }
        }

        // Character time-out indicator
        // 最后再传递TIME OUT 事件
        if(uart->info->timeout){
            if ((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_RTO) {
                // Signal RX Time-out event, if not all requested data received
                if (uart->info->xfer.rx_cnt != uart->info->xfer.rx_num) {
                    event |= CSK_UART_EVENT_RX_TIMEOUT;
                }
            }
        }

        // Modem interrupt
         if ((iir & UARTC_IIR_INT_MASK) == UARTC_IIR_MODEM) {
             // Save modem status register
             val = uart->reg->REG_MSR.all;

             // CTS state changed
             if (val & UARTC_MSR_DCTS) {
                 event |= CSK_UART_EVENT_CTS;
             }
         }
    }

    if ((uart->info->cb_event != NULL) && (event != 0U)) {
        uart->info->cb_event(event, uart->info->workspace);
    }
}

static void
UART_DMA_TX_Handler(uint32_t event, UART_RESOURCES *uart)
{
    switch (event) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        uart->info->xfer.tx_cnt = uart->info->xfer.tx_num;
        // Clear TX busy flag
        uart->info->xfer.send_active = 0U;
        uart->dma_tx->en = 0;

        // Set Send Complete event for asynchronous transfers
        if (uart->info->cb_event) {
            uart->info->cb_event(CSK_UART_EVENT_SEND_COMPLETE, uart->info->workspace);
        }
        break;
    case DMA_EVENT_ERROR:
        default:
        break;
    }
}

static void
UART_DMA_RX_Handler(uint32_t event, UART_RESOURCES *uart)
{
    switch (event ) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        uart->info->xfer.rx_cnt = uart->info->xfer.rx_num;
        uart->info->rx_status.rx_busy = 0U;
        uart->dma_rx->en = 0;
        if (uart->info->cb_event) {
            uart->info->cb_event(CSK_UART_EVENT_RECEIVE_COMPLETE, uart->info->workspace);
        }
        break;
    case DMA_EVENT_ERROR:
        default:
        break;
    }
}


/* UART0 static function */
static void UART0_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_TX_Handler((event & 0xFF), &uart0_resources);
}

static void UART0_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_RX_Handler((event & 0xFF), &uart0_resources);
}

static void UART0_IRQ_Handler(void)
{
    UART_IRQ_Handler(&uart0_resources);
}

/* UART1 static function */
static void UART1_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_TX_Handler((event & 0xFF), &uart1_resources);
}
static void UART1_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_RX_Handler((event & 0xFF), &uart1_resources);
}
static void UART1_IRQ_Handler(void)
{
    UART_IRQ_Handler(&uart1_resources);
}

/* UART2 static function */
static void UART2_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_TX_Handler((event & 0xFF), &uart2_resources);
}
static void UART2_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    UART_DMA_RX_Handler((event & 0xFF), &uart2_resources);
}
static void UART2_IRQ_Handler(void){
    UART_IRQ_Handler(&uart2_resources);;
}
