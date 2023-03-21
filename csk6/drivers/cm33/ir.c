/*
 * ir_venus.c
 *
 *  Created on: 2020年8月12日
 *      Author: USER
 */

#include "ir.h"

#define CSK_IR_DRV_VERSION    CSK_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */

 /* Driver Version */
static const CSK_DRIVER_VERSION DriverVersion = {
    CSK_IR_API_VERSION,
    CSK_IR_DRV_VERSION
};

static void
IR0_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);

static IR_DMA ir0_dma_tx_info = {
        CSK_IR0_TX_DMA_CH,
        CSK_IR0_TX_DMA_REQSEL,
        IR0_DMA_TX_Handler,
};

static void
IR0_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);

static IR_DMA ir0_dma_rx_info = {
        CSK_IR0_RX_DMA_CH,
        CSK_IR0_RX_DMA_REQSEL,
        IR0_DMA_RX_Handler,
};

static IR_SW_INFO ir_sw_info = {
        0
};

static IR_HW_NEC ir_nec_info = {
        0
};

static IR_HW_9012 ir_9012_info = {
        0
};

static IR_HW_RC5 ir_rc5_info = {
        0
};

static IR_HW_INFO ir_hw_info = {
        0,
        0,
        NULL,
        NULL,
        &ir_nec_info,
        &ir_9012_info,
        &ir_rc5_info,
};

static IR_INFO ir_info = {
        NULL,
        &ir_sw_info,
        &ir_hw_info,
        IR_MODE_NEC,
        0,
        NULL,
        0,
        0,
};

static void
IR0_IRQ_Handler();

static IR_RESOURCES ir0_resources = {
        VENUS_IR,
        IRQ_IR_VECTOR,
        IR0_IRQ_Handler,
        &ir0_dma_tx_info,
        &ir0_dma_rx_info,
        &ir_info,
};

#define CHECK_RESOURCES(res)  do{\
        if(res != &ir0_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

void* IR0(void){
    return (void*)&ir0_resources;
}

CSK_DRIVER_VERSION CSK_IR_GetVersion(void){
    return DriverVersion;
}

int32_t IR_Initialize(void *res, CSK_IR_SignalEvent_t cb_event, void* workspace){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if (ir->info->flags & CSK_FLAG_INITIALIZED){
        return CSK_DRIVER_OK;
    }

    ir->info->cb_event = cb_event;
    ir->info->workspace = workspace;
    ir->info->tx_rx_c = 0x0;
    ir->info->busy = 0x0;
    ir->info->mode = IR_MODE_NEC;

    // configure all hardware parameter
    __builtin_memset(ir->info->sw_info, 0, sizeof(IR_SW_INFO));

    ir->info->sw_info->thres = 0x800;

    ir->info->hw_info->rx_address = NULL;
    ir->info->hw_info->rx_command = 0x0;
    ir->info->hw_info->tx_address = 0x0;
    ir->info->hw_info->tx_command = 0x0;

    // nec
    ir->info->hw_info->_nec_info->tx_head_pulse_s1 = IR_TIME_NEC_TX_S1;
    ir->info->hw_info->_nec_info->tx_head_space_s3 = IR_TIME_NEC_TX_S3;
    ir->info->hw_info->_nec_info->tx_logic0_total_t1 = IR_TIME_NEC_TX_TIME1;
    ir->info->hw_info->_nec_info->tx_logic1_total_t2 = IR_TIME_NEC_TX_TIME2;
    ir->info->hw_info->_nec_info->tx_logic_pulse_bc = IR_TIME_NEC_TX_BIT_CYCLE;

    ir->info->hw_info->_nec_info->rx_head_pulse_top_margin_s1 = IR_TIME_NEC_RX_S1;
    ir->info->hw_info->_nec_info->rx_head_pulse_bottom_margin_s2 = IR_TIME_NEC_RX_S2;
    ir->info->hw_info->_nec_info->rx_head_space_top_margin_s3 = IR_TIME_NEC_RX_S3;
    ir->info->hw_info->_nec_info->rx_head_space_bottom_margin_s4 = IR_TIME_NEC_RX_S4;
    ir->info->hw_info->_nec_info->rx_total_time_s5 = IR_TIME_NEC_RX_S5;
    ir->info->hw_info->_nec_info->rx_filter_glitch_t1 = IR_TIME_NEC_RX_TIME1;
    ir->info->hw_info->_nec_info->rx_bit0_bit1_detect_t2 = IR_TIME_NEC_RX_TIME2;
    ir->info->hw_info->_nec_info->rx_bit_timeout_bc = IR_TIME_NEC_RX_BIT_CYCLE;

    ir->info->hw_info->_nec_info->tx_repeat_total_s2 = IR_TIME_NEC_REPEAT_S2;
    ir->info->hw_info->_nec_info->tx_repeat_pulse_s4 = IR_TIME_NEC_REPEAT_S4;

    // 9012
    ir->info->hw_info->_9012_info->tx_head_pulse_s1 = IR_TIME_9012_TX_S1;
    ir->info->hw_info->_9012_info->tx_head_space_s3 = IR_TIME_9012_TX_S3;
    ir->info->hw_info->_9012_info->tx_logic0_total_t1 = IR_TIME_9012_TX_TIME1;
    ir->info->hw_info->_9012_info->tx_logic1_total_t2 = IR_TIME_9012_TX_TIME2;
    ir->info->hw_info->_9012_info->tx_logic_pulse_bc = IR_TIME_9012_TX_BIT_CYCLE;

    ir->info->hw_info->_9012_info->rx_head_pulse_top_margin_s1 = IR_TIME_9012_RX_S1;
    ir->info->hw_info->_9012_info->rx_head_pulse_bottom_margin_s2 = IR_TIME_9012_RX_S2;
    ir->info->hw_info->_9012_info->rx_head_space_top_margin_s3 = IR_TIME_9012_RX_S3;
    ir->info->hw_info->_9012_info->rx_head_space_bottom_margin_s4 = IR_TIME_9012_RX_S4;
    ir->info->hw_info->_9012_info->rx_total_time_s5 = IR_TIME_9012_RX_S5;
    ir->info->hw_info->_9012_info->rx_filter_glitch_t1 = IR_TIME_9012_RX_TIME1;
    ir->info->hw_info->_9012_info->rx_bit0_bit1_detect_t2 = IR_TIME_9012_RX_TIME2;
    ir->info->hw_info->_9012_info->rx_bit_timeout_bc = IR_TIME_9012_RX_BIT_CYCLE;

    // rc5
    ir->info->hw_info->_rc5_info->tx_stage_cycle_s1 = IR_TIME_RC5_TX_S1;
    ir->info->hw_info->_rc5_info->tx_half_cycle_bc = IR_TIME_RC5_TX_BIT_CYCLE;
    ir->info->hw_info->_rc5_info->tx_logic_total_t1 = IR_TIME_RC5_TX_TIME1;

    ir->info->hw_info->_rc5_info->rx_head_pulse_top_margin_s1 = IR_TIME_RC5_RX_S1;
    ir->info->hw_info->_rc5_info->rx_head_pulse_bottom_margin_s2 = IR_TIME_RC5_RX_S2;
    ir->info->hw_info->_rc5_info->rx_head_space_top_margin_s3 = IR_TIME_RC5_RX_S3;
    ir->info->hw_info->_rc5_info->rx_head_space_bottom_margin_s4 = IR_TIME_RC5_RX_S4;
    ir->info->hw_info->_rc5_info->rx_total_time_s5 = IR_TIME_RC5_RX_S5;
    ir->info->hw_info->_rc5_info->rx_lower_logic_t1 = IR_TIME_RC5_RX_TIME1;
    ir->info->hw_info->_rc5_info->rx_upper_logic_t2 = IR_TIME_RC5_RX_TIME2;
    ir->info->hw_info->_rc5_info->rx_bit_timeout_bc = IR_TIME_RC5_RX_BIT_CYCLE;

    ir->info->flags = CSK_FLAG_INITIALIZED;

    return CSK_DRIVER_OK;
}

int32_t IR_Uninitialize(void* res){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    ir->info->cb_event = NULL;
    ir->info->workspace = NULL;
    ir->info->tx_rx_c = 0x0;
    ir->info->busy = 0x0;
    ir->info->mode = IR_MODE_NEC;

    // configure all hardware parameter
    __builtin_memset(ir->info->sw_info, 0, sizeof(IR_SW_INFO));

    ir->info->hw_info->rx_address = NULL;
    ir->info->hw_info->rx_command = 0x0;
    ir->info->hw_info->tx_address = 0x0;
    ir->info->hw_info->tx_command = 0x0;

    ir->info->flags = 0;

    return CSK_DRIVER_OK;
}

int32_t IR_PowerControl(void *res, CSK_POWER_STATE state){

    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;
    switch (state)
    {
    case CSK_POWER_OFF:
        if((ir->info->flags & CSK_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }

        // disable DMA
        ir->reg->REG_IR_DMA_CONFIG.bit.TX_DMA_ENABLE = 0x0;

        if((ir->info->busy) && (ir->info->mode == IR_MODE_CUSTOM)){
            if(ir->info->tx_rx_c){
                dma_channel_disable(ir->dma_tx->channel, 0x1);
            }else{
                dma_channel_disable(ir->dma_rx->channel, 0x1);
            }
        }

        // reset ir
        IP_SYSCTRL->REG_SW_RESET2.bit.IR_RESET = 0x1;
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = 0x0;
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_IR_CLK = 0x0;

        // clear ir soft ware fifo
        ir->reg->REG_IR_FIFO_CONFIG.all = 0xf;

        disable_IRQ(ir->irq_num);

        register_ISR(ir->irq_num, NULL, NULL);

        ir->info->flags &= (~CSK_FLAG_POWERED);

        break;
    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    case CSK_POWER_FULL:
        if((ir->info->flags & CSK_FLAG_INITIALIZED) == 0U){
            return CSK_DRIVER_ERROR;
        }

        if((ir->info->flags & CSK_FLAG_POWERED) != 0U){
            return CSK_DRIVER_OK;
        }

        // reset ir
        IP_SYSCTRL->REG_SW_RESET2.bit.IR_RESET = 0x1;
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = 0x1;
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_IR_CLK = 0x1;

        // clear interrupt status
        ir->reg->REG_IR_CLEAR_STATUS.all = 0x7f;

        // clear ir soft ware fifo
        ir->reg->REG_IR_FIFO_CONFIG.all = 0xf;

        // disable ir interrupt
        ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;
        // disable ir
        ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

        dma_initialize();

        ir->info->flags = CSK_FLAG_INITIALIZED | CSK_FLAG_POWERED;

         // Configure interrupt handle
        register_ISR(ir->irq_num, ir->irq_handler, NULL);
        enable_IRQ(ir->irq_num);

        break;
    }

    return CSK_DRIVER_OK;
}

int32_t IR_Control(void *res, uint32_t control, uint32_t arg){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if ((ir->info->flags & CSK_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    switch (control & CSK_IR_CONTROL_Msk){

    case CSK_IR_CONTROL_SW_TIMEOUT_THRES:
        if (arg == 0){
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        ir->info->sw_info->thres = arg;
        return CSK_DRIVER_OK;

    case CSK_IR_CONTROL_ABORT:
        // sw mode
        if(ir->info->mode == IR_MODE_CUSTOM){
            // tx
            if(ir->info->tx_rx_c){
                dma_channel_disable(ir->dma_tx->channel, 0x1);
                ir->info->sw_info->tx_cnt = dma_channel_get_count(ir->dma_tx->channel);
                // disable DMA
                ir->reg->REG_IR_DMA_CONFIG.bit.TX_DMA_ENABLE = 0x0;
            }
            // rx
            else{
                dma_channel_disable(ir->dma_rx->channel, 0x1);
                ir->info->sw_info->rx_cnt = dma_channel_get_count(ir->dma_rx->channel);
                // disable DMA
                ir->reg->REG_IR_DMA_CONFIG.bit.RX_DMA_ENABLE = 0x0;
            }

            // disable ir
            ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

            // clear fifo
            ir->reg->REG_IR_FIFO_CONFIG.all |= (BITSET_0X038_RX_FIFO_WR_CLR |\
                                                BITSET_0X038_RX_FIFO_RD_CLR |\
                                                BITSET_0X038_TX_FIFO_WR_CLR |\
                                                BITSET_0X038_TX_FIFO_RD_CLR);
        }

        // hw mode
        else{
            // ignore
        }

        ir->info->busy = 0x0;

        return CSK_DRIVER_OK;

    case CSK_IR_CONTROL_TX_REPEAT:
        // busy?
        if (ir->info->busy){
            return CSK_DRIVER_ERROR_BUSY;
        }

        // only support NEC mode and tx mode
        if ((ir->info->mode != IR_MODE_NEC) || (ir->info->tx_rx_c != 0x1)){
            return CSK_DRIVER_ERROR;
        }

        // set time parameter
        ir->reg->REG_IR_TIME_2.all = ir->info->hw_info->_nec_info->tx_repeat_total_s2;
        ir->reg->REG_IR_TIME_4.all = ir->info->hw_info->_nec_info->tx_repeat_pulse_s4;

        // configure control register
        // hardware mode
        ir->reg->REG_IR_CTRL.bit.SW_HW_MODE = 0x0;
        // set to tx mode
        ir->reg->REG_IR_CTRL.bit.TXRX_MODE = 0x1;
        // enable repeat mode
        ir->reg->REG_IR_CTRL.bit.TX_REPEAT_MODE = 0x1;
        // enable ir interrupt
        ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x1;
        // enable ir
        ir->reg->REG_IR_CTRL.bit.IR_EN = 0x1;
        // start send message
        ir->reg->REG_IR_TX_CONFIG.bit.TX_START = 0x1;

        return CSK_DRIVER_OK;

    case CSK_IR_CONTROL_CLEAR_TX_FIFO:
        ir->reg->REG_IR_FIFO_CONFIG.all |= 0x3;
        while(ir->reg->REG_IR_FSM.bit.IR_TX_FIFO_USEDW);

        return CSK_DRIVER_OK;

    case CSK_IR_CONTROL_CLEAR_RX_FIFO:
        ir->reg->REG_IR_FIFO_CONFIG.all |= 0xc;
        while(ir->reg->REG_IR_FSM.bit.IR_RX_FIFO_USEDW);

        return CSK_DRIVER_OK;
    }

    switch (control & CSK_IR_SW_CARRY_Msk){
    case CSK_IR_SW_CARRY_CONFIG:
    {
        uint32_t x;
        // high period : low period = 2 : 1
        x = 12000000/(arg*3);
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_HIGH = (2*x) & 0xff;
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_LOW = x & 0xff;

        ir->info->sw_info->h_carry = (2 * x) & 0xff;
        ir->info->sw_info->l_carry = x & 0xff;
    }
        break;
    }

    switch (control & CSK_IR_HW_PARAMETER_Msk){
    // nec tx
    case IR_HW_PARAMETER_NEC_TX_S1:
        ir->info->hw_info->_nec_info->tx_head_pulse_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_TX_S3:
        ir->info->hw_info->_nec_info->tx_head_space_s3 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_TX_BIT_CYCLE:
        ir->info->hw_info->_nec_info->tx_logic_pulse_bc = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_TX_TIME1:
        ir->info->hw_info->_nec_info->tx_logic1_total_t2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_TX_TIME2:
        ir->info->hw_info->_nec_info->tx_logic0_total_t1 = arg;
        return CSK_DRIVER_OK;


    // nec rx
    case IR_HW_PARAMETER_NEC_RX_S1:
        ir->info->hw_info->_nec_info->rx_head_pulse_top_margin_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_S2:
        ir->info->hw_info->_nec_info->rx_head_pulse_bottom_margin_s2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_S3:
        ir->info->hw_info->_nec_info->rx_head_space_top_margin_s3 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_S4:
        ir->info->hw_info->_nec_info->rx_head_space_bottom_margin_s4 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_S5:
        ir->info->hw_info->_nec_info->rx_total_time_s5 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_TIME1:
        ir->info->hw_info->_nec_info->rx_filter_glitch_t1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_TIME2:
        ir->info->hw_info->_nec_info->rx_bit0_bit1_detect_t2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_RX_BIT_CYCLE:
        ir->info->hw_info->_nec_info->rx_bit_timeout_bc = arg;
        return CSK_DRIVER_OK;


    // nec repeat
    case IR_HW_PARAMETER_NEC_REPEAT_S2:
        ir->info->hw_info->_nec_info->tx_repeat_total_s2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_NEC_REPEAT_S4:
        ir->info->hw_info->_nec_info->tx_repeat_pulse_s4 = arg;
        return CSK_DRIVER_OK;


    // 9012 tx
    case IR_HW_PARAMETER_9012_TX_S1:
        ir->info->hw_info->_9012_info->tx_head_pulse_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_TX_S3:
        ir->info->hw_info->_9012_info->tx_head_space_s3 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_TX_BIT_CYCLE:
        ir->info->hw_info->_9012_info->tx_logic_pulse_bc = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_TX_TIME1:
        ir->info->hw_info->_9012_info->tx_logic0_total_t1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_TX_TIME2:
        ir->info->hw_info->_9012_info->tx_logic1_total_t2 = arg;
        return CSK_DRIVER_OK;


    // 9012 rx
    case IR_HW_PARAMETER_9012_RX_S1:
        ir->info->hw_info->_9012_info->rx_head_pulse_top_margin_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_S2:
        ir->info->hw_info->_9012_info->rx_head_pulse_bottom_margin_s2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_S3:
        ir->info->hw_info->_9012_info->rx_head_space_top_margin_s3 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_S4:
        ir->info->hw_info->_9012_info->rx_head_space_bottom_margin_s4 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_S5:
        ir->info->hw_info->_9012_info->rx_total_time_s5 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_TIME1:
        ir->info->hw_info->_9012_info->rx_filter_glitch_t1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_TIME2:
        ir->info->hw_info->_9012_info->rx_bit0_bit1_detect_t2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_9012_RX_BIT_CYCLE:
        ir->info->hw_info->_9012_info->rx_bit_timeout_bc = arg;
        return CSK_DRIVER_OK;


    // rc5 tx
    case IR_HW_PARAMETER_RC5_TX_S1:
        ir->info->hw_info->_rc5_info->tx_stage_cycle_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_TX_BIT_CYCLE:
        ir->info->hw_info->_rc5_info->tx_half_cycle_bc = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_TX_TIME1:
        ir->info->hw_info->_rc5_info->tx_logic_total_t1 = arg;
        return CSK_DRIVER_OK;


    // rc5 rx
    case IR_HW_PARAMETER_RC5_RX_S1:
        ir->info->hw_info->_rc5_info->rx_head_pulse_top_margin_s1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_S2:
        ir->info->hw_info->_rc5_info->rx_head_pulse_bottom_margin_s2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_S3:
        ir->info->hw_info->_rc5_info->rx_head_space_top_margin_s3 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_S4:
        ir->info->hw_info->_rc5_info->rx_head_space_bottom_margin_s4 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_S5:
        ir->info->hw_info->_rc5_info->rx_total_time_s5 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_TIME1:
        ir->info->hw_info->_rc5_info->rx_lower_logic_t1 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_TIME2:
        ir->info->hw_info->_rc5_info->rx_upper_logic_t2 = arg;
        return CSK_DRIVER_OK;
    case IR_HW_PARAMETER_RC5_RX_BIT_CYCLE:
        ir->info->hw_info->_rc5_info->rx_bit_timeout_bc = arg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_OK;
}

int32_t  IR_HW_Send(void *res, IR_MODE mode, uint16_t address, uint16_t command){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if(!(ir->info->flags & CSK_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    if(ir->info->busy){
        return CSK_DRIVER_ERROR_BUSY;
    }

    // disable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

    //********************software information configure
    ir->info->busy = 0x1;
    ir->info->hw_info->tx_address = address;
    ir->info->hw_info->tx_command = command;
    ir->info->mode = mode;
    ir->info->tx_rx_c = 1;

    // configure the mode timing
    switch (ir->info->mode){
    case IR_MODE_CUSTOM:
        return CSK_DRIVER_ERROR_PARAMETER;
    case IR_MODE_NEC:
//      ir->reg->REG_IR_TX_CONFIG.bit.IR_TX_POL = 0x1;
        // nec mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x0;
        // 38khz carry
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_HIGH = IR_CARRY_HIGH_CONFIG_38KHZ;
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_LOW = IR_CARRY_LOW_CONFIG_38KHZ;
        // hardware parameter
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_nec_info->tx_head_pulse_s1;
        ir->reg->REG_IR_TIME_3.all = ir->info->hw_info->_nec_info->tx_head_space_s3;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_nec_info->tx_logic0_total_t1;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_2 = ir->info->hw_info->_nec_info->tx_logic1_total_t2;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_nec_info->tx_logic_pulse_bc;
        break;
    case IR_MODE_TOSHIBA_9012:
//      ir->reg->REG_IR_TX_CONFIG.bit.IR_TX_POL = 0x1;
        // 9012 mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x1;
        // 38khz carry
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_HIGH = IR_CARRY_HIGH_CONFIG_38KHZ;
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_LOW = IR_CARRY_LOW_CONFIG_38KHZ;
        // hardware parameter
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_9012_info->tx_head_pulse_s1;
        ir->reg->REG_IR_TIME_3.all = ir->info->hw_info->_9012_info->tx_head_space_s3;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_9012_info->tx_logic0_total_t1;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_2 = ir->info->hw_info->_9012_info->tx_logic1_total_t2;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_9012_info->tx_logic_pulse_bc;
        break;
    case IR_MODE_PHILIPS_RC5:
//      ir->reg->REG_IR_TX_CONFIG.bit.IR_TX_POL = 0x0;
        // rc5 mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x2;
        // 36khz carry
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_HIGH = IR_CARRY_HIGH_CONFIG_36KHZ;
        ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_LOW = IR_CARRY_LOW_CONFIG_36KHZ;
        // hardware parameter
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_rc5_info->tx_stage_cycle_s1;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_rc5_info->tx_half_cycle_bc;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_rc5_info->tx_logic_total_t1;
        break;
    }
    // hardware mode
    ir->reg->REG_IR_CTRL.bit.SW_HW_MODE = 0x0;
    ir->reg->REG_IR_TX_CODE.bit.IR_TX_USERCODE = address;
    ir->reg->REG_IR_TX_CODE.bit.IR_TX_DATACODE = command;
    // set to tx mode
    ir->reg->REG_IR_CTRL.bit.TXRX_MODE = 0x1;
    // enable ir interrupt
    ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x1;
    // enable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x1;
    // start send message
    ir->reg->REG_IR_TX_CONFIG.bit.TX_START = 0x1;

    return CSK_DRIVER_OK;
}


int32_t  IR_HW_Receive(void *res, IR_MODE mode, uint16_t *address, uint16_t *command){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if(!(ir->info->flags & CSK_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    if(ir->info->busy){
        return CSK_DRIVER_ERROR_BUSY;
    }

    // disable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

    //********************software information configure
    ir->info->busy = 0x1;
    ir->info->hw_info->rx_address = address;
    ir->info->hw_info->rx_command = command;
    ir->info->tx_rx_c = 0;
    ir->info->mode = mode;

    //*********************register configure
    switch (ir->info->mode){
    case IR_MODE_CUSTOM:
        return CSK_DRIVER_ERROR_PARAMETER;
    case IR_MODE_NEC:
        // nec mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x0;
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_nec_info->rx_head_pulse_top_margin_s1;
        ir->reg->REG_IR_TIME_2.all = ir->info->hw_info->_nec_info->rx_head_pulse_bottom_margin_s2;
        ir->reg->REG_IR_TIME_3.all = ir->info->hw_info->_nec_info->rx_head_space_top_margin_s3;
        ir->reg->REG_IR_TIME_4.all = ir->info->hw_info->_nec_info->rx_head_space_bottom_margin_s4;
        ir->reg->REG_IR_TIME_5.all = ir->info->hw_info->_nec_info->rx_total_time_s5;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_nec_info->rx_filter_glitch_t1;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_2 = ir->info->hw_info->_nec_info->rx_bit0_bit1_detect_t2;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_nec_info->rx_bit_timeout_bc;
        break;
    case IR_MODE_TOSHIBA_9012:
        // 9012 mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x1;
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_9012_info->rx_head_pulse_top_margin_s1;
        ir->reg->REG_IR_TIME_2.all = ir->info->hw_info->_9012_info->rx_head_pulse_bottom_margin_s2;
        ir->reg->REG_IR_TIME_3.all = ir->info->hw_info->_9012_info->rx_head_space_top_margin_s3;
        ir->reg->REG_IR_TIME_4.all = ir->info->hw_info->_9012_info->rx_head_space_bottom_margin_s4;
        ir->reg->REG_IR_TIME_5.all = ir->info->hw_info->_9012_info->rx_total_time_s5;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_9012_info->rx_filter_glitch_t1;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_2 = ir->info->hw_info->_9012_info->rx_bit0_bit1_detect_t2;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_9012_info->rx_bit_timeout_bc;
        break;
    case IR_MODE_PHILIPS_RC5:
        // rc5 mode
        ir->reg->REG_IR_CTRL.bit.IR_MODE = 0x2;
        ir->reg->REG_IR_TIME_1.all = ir->info->hw_info->_rc5_info->rx_head_pulse_top_margin_s1;
        ir->reg->REG_IR_TIME_2.all = ir->info->hw_info->_rc5_info->rx_head_pulse_bottom_margin_s2;
        ir->reg->REG_IR_TIME_3.all = ir->info->hw_info->_rc5_info->rx_head_space_top_margin_s3;
        ir->reg->REG_IR_TIME_4.all = ir->info->hw_info->_rc5_info->rx_head_space_bottom_margin_s4;
        ir->reg->REG_IR_TIME_5.all = ir->info->hw_info->_rc5_info->rx_total_time_s5;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_1 = ir->info->hw_info->_rc5_info->rx_lower_logic_t1;
        ir->reg->REG_IR_CTRL.bit.BIT_TIME_2 = ir->info->hw_info->_rc5_info->rx_upper_logic_t2;
        ir->reg->REG_IR_CTRL.bit.IR_BIT_CYCLE = ir->info->hw_info->_rc5_info->rx_bit_timeout_bc;
        break;
    }
    // hardware mode
    ir->reg->REG_IR_CTRL.bit.SW_HW_MODE = 0x0;
    // enable ir interrupt
    ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x1;
    // set to rx mode
    ir->reg->REG_IR_CTRL.bit.TXRX_MODE = 0x0;
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x1;

    return CSK_DRIVER_OK;
}


int32_t
IR_SW_Send(void *res, const int32_t *data, uint32_t num){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if(!(ir->info->flags & CSK_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    if(ir->info->busy){
        return CSK_DRIVER_ERROR_BUSY;
    }

    // disable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

    //********************software information configure
    // set busy
    ir->info->busy = 0x1;
    ir->info->tx_rx_c = 0x1;

    ir->info->mode = IR_MODE_CUSTOM;

    ir->info->sw_info->tx_buf = data;
    ir->info->sw_info->tx_num = num;
    ir->info->sw_info->tx_cnt = 0;

    //*********************register configure
    // software mode
    ir->reg->REG_IR_CTRL.bit.SW_HW_MODE = 0x1;
    // carry
    ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_HIGH = ir->info->sw_info->h_carry;
    ir->reg->REG_IR_CARRY_CONFIG.bit.IR_CARRY_LOW = ir->info->sw_info->l_carry;
    // tx mode
    ir->reg->REG_IR_CTRL.bit.TXRX_MODE = 0x1;
    // enable tx dma
    // 8-fifo_use >= threshold
    ir->reg->REG_IR_DMA_CONFIG.bit.TX_DMA_THRES_SEL = 0x7;
    ir->reg->REG_IR_DMA_CONFIG.bit.TX_DMA_ENABLE = 0x1;

    //*********************DMA configure
    {
        int32_t stat;

        dma_channel_select(
            &ir->dma_tx->channel,
            ir->dma_tx->cb_event,
            0,
            DMA_CACHE_SYNC_SRC);
        if (ir->dma_tx->channel == DMA_CHANNEL_ANY) {
            return CSK_DRIVER_ERROR;
        }

        stat = dma_channel_configure (ir->dma_tx->channel,
                    (uint32_t) ir->info->sw_info->tx_buf,
                    (uint32_t) (&(ir->reg->REG_IR_TX_FIFO.all)),
                    ir->info->sw_info->tx_num,
                    DMA_CH_CTLL_DST_WIDTH(DMA_WIDTH_WORD) | DMA_CH_CTLL_SRC_WIDTH(DMA_WIDTH_WORD) |\
                  DMA_CH_CTLL_DST_BSIZE(DMA_BSIZE_1) | DMA_CH_CTLL_SRC_BSIZE(DMA_BSIZE_1) |\
                  DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |\
                  DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                  DMA_CH_CFGL_CH_PRIOR(1), // config_low
                  DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(ir->dma_tx->reqsel), // config_high
                  0, 0);

        if(stat == -1){
            return CSK_DRIVER_ERROR;
        }
    }

    // enable ir interrupt
    ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x1;
    // enable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x1;

    return CSK_DRIVER_OK;
}


int32_t
IR_SW_Receive(void *res, int32_t *data, uint32_t num){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    if(!(ir->info->flags & CSK_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    if(ir->info->busy){
        return CSK_DRIVER_ERROR_BUSY;
    }

    // disable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x0;

    //********************software information configure
    // set busy
    ir->info->busy = 0x1;
    ir->info->tx_rx_c = 0x0;

    ir->info->mode = IR_MODE_CUSTOM;

    ir->info->sw_info->rx_buf = data;
    ir->info->sw_info->rx_num = num;
    ir->info->sw_info->rx_cnt = 0;

    //*********************register configure
    // software mode
    ir->reg->REG_IR_CTRL.bit.SW_HW_MODE = 0x1;
    // rx mode
    ir->reg->REG_IR_CTRL.bit.TXRX_MODE = 0x0;
    // rx time out
    ir->reg->REG_IR_IDLE_THRES.all = ir->info->sw_info->thres;
    // enable rx dma
    // fifo_use >= threshold
    ir->reg->REG_IR_DMA_CONFIG.bit.RX_DMA_THRES_SEL = 0x1;
    ir->reg->REG_IR_DMA_CONFIG.bit.RX_DMA_ENABLE = 0x1;

    //*********************DMA configure
    {
        int32_t stat;

        dma_channel_select(
            &ir->dma_rx->channel,
            ir->dma_rx->cb_event,
            0,
            DMA_CACHE_SYNC_DST);
        if (ir->dma_rx->channel == DMA_CHANNEL_ANY) {
            return CSK_DRIVER_ERROR;
        }

        stat = dma_channel_configure (ir->dma_rx->channel,
                    (uint32_t) (&(ir->reg->REG_IR_RX_FIFO)),
                    (uint32_t) ir->info->sw_info->rx_buf,
                    ir->info->sw_info->rx_num,
                    DMA_CH_CTLL_DST_WIDTH(DMA_WIDTH_WORD) | DMA_CH_CTLL_SRC_WIDTH(DMA_WIDTH_WORD) |\
                  DMA_CH_CTLL_DST_BSIZE(DMA_BSIZE_1) | DMA_CH_CTLL_SRC_BSIZE(DMA_BSIZE_1) |\
                  DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |\
                  DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                  DMA_CH_CFGL_CH_PRIOR(1), // config_low
                  DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(ir->dma_rx->reqsel), // config_high
                  0, 0);

        if(stat == -1){
            return CSK_DRIVER_ERROR;
        }
    }

    // enable ir interrupt
    ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x1;
    // enable ir
    ir->reg->REG_IR_CTRL.bit.IR_EN = 0x1;

    return CSK_DRIVER_OK;
}

uint32_t IR_GetTxCount(void *res){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    return ir->info->sw_info->tx_cnt;
}

uint32_t IR_GetRxCount(void *res){
    CHECK_RESOURCES(res);

    IR_RESOURCES *ir = (IR_RESOURCES*)res;

    return ir->info->sw_info->rx_cnt;
}

void IR_IRQ_Handler(IR_RESOURCES *ir){
    uint32_t c_status = 0;
    uint32_t event = 0;
    c_status = ir->reg->REG_IR_FSM.all;

    // TODO clear pending

    // clear status
    ir->reg->REG_IR_CLEAR_STATUS.all = 0x7f;
    // can't disable ir interrupt, because the device need to
    // receive repeat and send repeat signal

    // in nec receive mode
    if ((ir->info->mode == IR_MODE_NEC) && (ir->info->tx_rx_c == 0)){
        // check repeat mode
        if (c_status & BITSET_0X028_IR_REPEAT){
            event |= CSK_IR_EVENT_HW_RX_REPEAT_TRIGGER;
        }

        // receive ok and busy
        if ((c_status & BITSET_0X028_IR_RECEIVED_OK) && (ir->info->busy == 1)){
            ir->info->busy = 0;

            *(ir->info->hw_info->rx_address) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_USERCODE & 0xffff);
            *(ir->info->hw_info->rx_command) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_DATACODE & 0xffff);

            event |= CSK_IR_EVENT_HW_RECEIVE_COMPLETE;
        }
    }

    // in nec transmit mode
    if ((ir->info->mode == IR_MODE_NEC) && (ir->info->tx_rx_c == 1)){
        // check tx repeat mode
        if (c_status & BITSET_0X028_IR_TX_REPEAT){
            event |= CSK_IR_EVENT_HW_TX_REPEAT_COMPLETE;

            // disable repeat mode
            ir->reg->REG_IR_CTRL.bit.TX_REPEAT_MODE = 0x0;
        }

        // transmit ok and busy
        if ((c_status & BITSET_0X028_IR_TRANSMIT_OK) && (ir->info->busy == 1)){
            event |= CSK_IR_EVENT_HW_SEND_COMPLETE;
            ir->info->busy = 0;
        }
    }

    // in 9012 receive mode
    if ((ir->info->mode == IR_MODE_TOSHIBA_9012) && (ir->info->tx_rx_c == 0)){

        // receive ok and busy
        if ((c_status & BITSET_0X028_IR_RECEIVED_OK) && (ir->info->busy == 1)){
            ir->info->busy = 0;

            *(ir->info->hw_info->rx_address) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_USERCODE & 0xffff);
            *(ir->info->hw_info->rx_command) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_DATACODE & 0xffff);

            ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;

            event |= CSK_IR_EVENT_HW_RECEIVE_COMPLETE;
        }
    }

    // in 9012 transmit mode
    if ((ir->info->mode == IR_MODE_TOSHIBA_9012) && (ir->info->tx_rx_c == 1)){

        // transmit ok and busy
        if ((c_status & BITSET_0X028_IR_TRANSMIT_OK) && (ir->info->busy == 1)){

            event |= CSK_IR_EVENT_HW_SEND_COMPLETE;
            ir->info->busy = 0;

            ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;
        }
    }

    // in rc5 receive mode
    if ((ir->info->mode == IR_MODE_PHILIPS_RC5) && (ir->info->tx_rx_c == 0)){

        // receive ok and busy
        if ((c_status & BITSET_0X028_IR_RECEIVED_OK) && (ir->info->busy == 1)){
            ir->info->busy = 0;

            *(ir->info->hw_info->rx_address) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_USERCODE & 0xffff);
            *(ir->info->hw_info->rx_command) = (uint16_t)(ir->reg->REG_IR_RX_CODE.bit.IR_RX_DATACODE & 0xffff);

            ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;

            event |= CSK_IR_EVENT_HW_RECEIVE_COMPLETE;
        }
    }

    // in rc5 transmit mode
    if ((ir->info->mode == IR_MODE_PHILIPS_RC5) && (ir->info->tx_rx_c == 1)){

        // transmit ok and busy
        if ((c_status & BITSET_0X028_IR_TRANSMIT_OK) && (ir->info->busy == 1)){

            event |= CSK_IR_EVENT_HW_SEND_COMPLETE;
            ir->info->busy = 0;

            ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;
        }
    }


    // because ir timeout , interrupt should ignore the first
    // timeout interrupt
    // in custom receive mode
    if ((ir->info->mode == IR_MODE_CUSTOM) && (ir->info->tx_rx_c == 0)){
        // get DMA receive count
        ir->info->sw_info->rx_cnt = dma_channel_get_count(ir->dma_rx->channel);

        if ((c_status & BITSET_0X028_IR_RECEIVED_OK) && (ir->info->busy == 1)){
            // TIME OUT
            if (ir->info->sw_info->rx_cnt != ir->info->sw_info->rx_num){
                while(!ir->reg->REG_IR_FSM.bit.IR_RX_FIFO_EMPTY);

                dma_channel_disable(ir->dma_rx->channel, 0x1);

                // in power up the first receive data equal 0x80000000, it's wrong
                ir->info->sw_info->rx_buf[0] += 4;

                ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;
                ir->reg->REG_IR_DMA_CONFIG.bit.RX_DMA_ENABLE = 0x0;

                event |= CSK_IR_EVENT_SW_RECEIVE_TIMEOUT;

                ir->info->busy = 0x0;
            }
            // ignore
            else {
                return;
            }
        }
    }

    // in custom transmit mode
    if ((ir->info->mode == IR_MODE_CUSTOM) && (ir->info->tx_rx_c == 1)){

        if((c_status & BITSET_0X028_IR_TRANSMIT_OK) && (ir->info->busy == 1)){
            while(!ir->reg->REG_IR_FSM.bit.IR_TX_FIFO_EMPTY);

            ir->info->sw_info->tx_cnt = dma_channel_get_count(ir->dma_tx->channel);
            // if tx count is't equal to the tx number, maybe user tx buffer have zero

            event |= CSK_IR_EVENT_SW_SEND_COMPLETE;

            ir->reg->REG_IR_CTRL.bit.IR_INT_EN = 0x0;

            ir->info->busy = 0;
        }

    }

    if (event == 0){
        return;
    }

    if(ir->info->cb_event){
        ir->info->cb_event(event, ir->info->workspace);
    }
}

static void
IR_DMA_TX_Handler(uint32_t event, IR_RESOURCES *ir){
    switch (event){
    case DMA_EVENT_TRANSFER_COMPLETE:
        // wait tx fifo not full
        while(ir->reg->REG_IR_FSM.bit.IR_TX_FIFO_FULL);
        ir->reg->REG_IR_TX_FIFO.all = 0x80000000;
        break;
    case DMA_EVENT_ERROR:
        default:
        break;
    }
}

static void
IR_DMA_RX_Handler(uint32_t event, IR_RESOURCES *ir){
    switch (event){
    case DMA_EVENT_TRANSFER_COMPLETE:
        ir->info->busy = 0x0;
        ir->info->sw_info->rx_buf[0] += 4;
        if(ir->info->cb_event){
            ir->info->cb_event(CSK_IR_EVENT_SW_RECEIVE_COMPLETE, ir->info->workspace);
        }
        break;
    case DMA_EVENT_ERROR:
        default:
        break;
    }
}


/* IR0 static function*/
static void
IR0_IRQ_Handler(){
    IR_IRQ_Handler(&ir0_resources);
}

static void
IR0_DMA_TX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    IR_DMA_TX_Handler((event & 0xFF), &ir0_resources);
}

static void
IR0_DMA_RX_Handler(uint32_t event, uint32_t xfer_bytes, uint32_t usr_param){
    IR_DMA_RX_Handler((event & 0xFF), &ir0_resources);
}

