/*
 * ir_venus.h
 *
 *  Created on: 2020年8月12日
 *      Author: USER
 */

#ifndef MODULES_DRIVER_INCLUDE_IR_VENUS_H_
#define MODULES_DRIVER_INCLUDE_IR_VENUS_H_

#include "cmn_sysctrl_reg_venus.h"
#include "dma.h"

#include "Driver_IR.h"
#include "venus_ap.h"
#include "ir_reg_venus.h"
#include "IOMuxManager.h"

/*************NEC TX TIMING***************/
#define IR_TIME_NEC_TX_S1                              (287)
#define IR_TIME_NEC_TX_S3                              (143)
#define IR_TIME_NEC_TX_BIT_CYCLE                       (18)
#define IR_TIME_NEC_TX_TIME1                           (35)
#define IR_TIME_NEC_TX_TIME2                           (71)
/*************NEC RX TIMING***************/
#define IR_TIME_NEC_RX_S1                              (280)
#define IR_TIME_NEC_RX_S2                              (294)
#define IR_TIME_NEC_RX_S3                              (420)
#define IR_TIME_NEC_RX_S4                              (441)
#define IR_TIME_NEC_RX_S5                              (3199)
#define IR_TIME_NEC_RX_TIME1                           (10)
#define IR_TIME_NEC_RX_TIME2                           (37)
#define IR_TIME_NEC_RX_BIT_CYCLE                       (74)
/*************NEC TX REPEAT TIMING***************/
#define IR_TIME_NEC_REPEAT_S2                          (71)
#define IR_TIME_NEC_REPEAT_S4                          (17)
/*************9012 TX TIMING***************/
#define IR_TIME_9012_TX_S1                             (143)
#define IR_TIME_9012_TX_S3                             (143)
#define IR_TIME_9012_TX_BIT_CYCLE                      (18)
#define IR_TIME_9012_TX_TIME1                          (35)
#define IR_TIME_9012_TX_TIME2                          (71)
/*************9012 RX TIMING***************/
#define IR_TIME_9012_RX_S1                             (140)
#define IR_TIME_9012_RX_S2                             (148)
#define IR_TIME_9012_RX_S3                             (280)
#define IR_TIME_9012_RX_S4                             (296)
#define IR_TIME_9012_RX_S5                             (3199)
#define IR_TIME_9012_RX_TIME1                          (10)
#define IR_TIME_9012_RX_TIME2                          (37)
#define IR_TIME_9012_RX_BIT_CYCLE                      (74)
/*************RC5 TX TIMING***************/
#define IR_TIME_RC5_TX_S1                              (56)
#define IR_TIME_RC5_TX_BIT_CYCLE                       (28)
#define IR_TIME_RC5_TX_TIME1                           (56)
/*************RC5 RX TIMING***************/
#define IR_TIME_RC5_RX_S1                              (26)
#define IR_TIME_RC5_RX_S2                              (35)
#define IR_TIME_RC5_RX_S3                              (112)
#define IR_TIME_RC5_RX_S4                              (140)
#define IR_TIME_RC5_RX_S5                              (3199)
#define IR_TIME_RC5_RX_TIME1                           (24)
#define IR_TIME_RC5_RX_TIME2                           (30)
#define IR_TIME_RC5_RX_BIT_CYCLE                       (56)
/*************IR Carry***************/
#define IR_CARRY_HIGH_CONFIG_38KHZ                     (103)//(210)
#define IR_CARRY_LOW_CONFIG_38KHZ                      (55)//(104)

#define IR_CARRY_HIGH_CONFIG_36KHZ                     (109)//(220)
#define IR_CARRY_LOW_CONFIG_36KHZ                      (58)//(110)

// static macro
#define VENUS_IR                   ((IR_RegDef*) IR_BASE)

#define CSK_FLAG_INITIALIZED        (1UL << 0)
#define CSK_FLAG_POWERED            (1UL << 1)

#define CSK_IR0_TX_DMA_CH           (4)
#define CSK_IR0_TX_DMA_REQSEL       (DMA_HSID_IR_TX)
#define CSK_IR0_RX_DMA_CH           (5)
#define CSK_IR0_RX_DMA_REQSEL       (DMA_HSID_IR_RX)

//typedef const struct _IR_PINS {
//    CSK_IOMUX_PIN_ID                 *input;
//    CSK_IOMUX_PIN_ID                 *output;
//} IR_PINS;

typedef struct _IR_SW_INFO {
    uint32_t tx_num;
    uint32_t rx_num;
    const int32_t *tx_buf;
    int32_t *rx_buf;
    uint32_t tx_cnt;
    uint32_t rx_cnt;
    uint32_t thres;
    uint8_t h_carry;
    uint8_t l_carry;
} IR_SW_INFO;

typedef struct _IR_HW_NEC {
    uint32_t tx_head_pulse_s1;                      // header 9ms
    uint32_t tx_head_space_s3;                      // header 4.5ms
    uint32_t tx_logic_pulse_bc;                     // bit 0.56ms
    uint32_t tx_logic0_total_t1;                    // logic 0: 1.12ms
    uint32_t tx_logic1_total_t2;                    // logic 1: 2.25ms

    uint32_t rx_head_pulse_top_margin_s1;
    uint32_t rx_head_pulse_bottom_margin_s2;
    uint32_t rx_head_space_top_margin_s3;
    uint32_t rx_head_space_bottom_margin_s4;
    uint32_t rx_total_time_s5;
    uint32_t rx_filter_glitch_t1;
    uint32_t rx_bit0_bit1_detect_t2;
    uint32_t rx_bit_timeout_bc;

    uint32_t tx_repeat_total_s2;
    uint32_t tx_repeat_pulse_s4;
} IR_HW_NEC;

typedef struct _IR_HW_9012 {
    uint32_t tx_head_pulse_s1;                      // header 4.5ms
    uint32_t tx_head_space_s3;                      // header 4.5ms
    uint32_t tx_logic_pulse_bc;                     // bit 0.56ms
    uint32_t tx_logic0_total_t1;                    // logic 0: 1.12ms
    uint32_t tx_logic1_total_t2;                    // logic 1: 2.25ms

    uint32_t rx_head_pulse_top_margin_s1;
    uint32_t rx_head_pulse_bottom_margin_s2;
    uint32_t rx_head_space_top_margin_s3;
    uint32_t rx_head_space_bottom_margin_s4;
    uint32_t rx_total_time_s5;
    uint32_t rx_filter_glitch_t1;
    uint32_t rx_bit0_bit1_detect_t2;
    uint32_t rx_bit_timeout_bc;
} IR_HW_9012;

typedef struct _IR_HW_RC5 {
    uint32_t tx_stage_cycle_s1;
    uint32_t tx_half_cycle_bc;
    uint32_t tx_logic_total_t1;

    uint32_t rx_head_pulse_top_margin_s1;
    uint32_t rx_head_pulse_bottom_margin_s2;
    uint32_t rx_head_space_top_margin_s3;
    uint32_t rx_head_space_bottom_margin_s4;
    uint32_t rx_total_time_s5;
    uint32_t rx_lower_logic_t1;
    uint32_t rx_upper_logic_t2;
    uint32_t rx_bit_timeout_bc;
} IR_HW_RC5;

typedef struct _IR_HW_INFO {
    uint16_t tx_address;
    uint16_t tx_command;
    uint16_t *rx_address;
    uint16_t *rx_command;
    IR_HW_NEC * const _nec_info;
    IR_HW_9012 * const _9012_info;
    IR_HW_RC5 * const _rc5_info;
} IR_HW_INFO;

typedef struct _IR_INFO {
    CSK_IR_SignalEvent_t cb_event;
    IR_SW_INFO * const sw_info;
    IR_HW_INFO * const hw_info;
    IR_MODE mode;
    uint32_t flags;
    void* workspace;
    uint8_t busy;                          // HW T/R or SW T/R busy flag
    uint8_t tx_rx_c;                       // 0 for rx
                                           // 1 for tx
} IR_INFO;

typedef struct _IR_DMA {
    uint8_t channel;                       // DMA Channel
    uint8_t reqsel;                        // DMA request selection
    DMA_SignalEvent_t cb_event;            // DMA Event callback
} IR_DMA;

typedef const struct _IR_RESOURCES {
    IR_RegDef* reg;
    uint32_t irq_num;
    void (*irq_handler)(void);
//    IR_PINS *pins;
    IR_DMA *dma_tx;
    IR_DMA *dma_rx;
    IR_INFO *info;
} IR_RESOURCES;

#endif /* MODULES_DRIVER_INCLUDE_IR_VENUS_H_ */
