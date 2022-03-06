/*
 * apc_reg_venus.h
 *
 *  Created on:  Sept. 22, 2020
 *
 */

#ifndef __APC_REG_VENUS_H
#define __APC_REG_VENUS_H

#include <stdint.h>
#include "venus_cp.h"
#include "hal/apc_venus.h"
#include "hal/reg/apc_reg_venus.h" // APC Register Map

//====================== Register-related Macros & Functions =====================
#define CSK_APC_RegDef      APC_RegDef
#define CSK_APC             ((APC_RegDef *) APC_BASE)

// channel FIFO depth (RX = IN, TX = OUT)
#define APC_IN_FIFO_DEPTH_DEF   16 // 64 // preliminary design: 16
#define APC_OUT_FIFO_DEPTH_DEF  16 // 64 // preliminary design: 16
#define APC_RX_FIFO_DEPTH_DEF   APC_IN_FIFO_DEPTH_DEF
#define APC_TX_FIFO_DEPTH_DEF   APC_OUT_FIFO_DEPTH_DEF

// channel FIFO data register offset
#define APC_RX_CH0L_DATA_OFFSET     (APC_BASE + 0x110)
#define APC_RX_CH0R_DATA_OFFSET     (APC_BASE + 0x114)
#define APC_RX_CH1L_DATA_OFFSET     (APC_BASE + 0x118)
#define APC_RX_CH1R_DATA_OFFSET     (APC_BASE + 0x11C)
#define APC_RX_CH2L_DATA_OFFSET     (APC_BASE + 0x120)
#define APC_RX_CH2R_DATA_OFFSET     (APC_BASE + 0x124)

#define APC_TX_CH0L_DATA_OFFSET     (APC_BASE + 0x100)
#define APC_TX_CH0R_DATA_OFFSET     (APC_BASE + 0x104)
#define APC_TX_CH1L_DATA_OFFSET     (APC_BASE + 0x108)
#define APC_TX_CH1R_DATA_OFFSET     (APC_BASE + 0x10C)

// EQ coefficient base address
#define APC_TX_CH1_EQCOEF_BASE      (APC_BASE + 0x20)

//-------------------------------------------------------------------------------
// APC_CFG Register

#define APC_ENABLE                     BITSET_0X000_APC_ENABLE
#define APC_RX_PATH_RESET              BITSET_0X000_APC_RX_PATH_RESET
#define APC_TX_PATH_RESET              BITSET_0X000_APC_TX_PATH_RESET
#define APC_AUTO_CLK_GATING            BITSET_0X000_AUTO_CLK_GATING

//-------------------------------------------------------------------------------
// APC INTR-related Registers

#define APC_INTR_FIFO_EMPTY         (0x1 << 0)
#define APC_INTR_FIFO_FULL          (0x1 << 1)
#define APC_INTR_FIFO_UNDERFLOW     (0x1 << 2)
#define APC_INTR_FIFO_OVERFLOW      (0x1 << 3)
#define APC_INTR_FIFO_DMA_REQ       (0x1 << 4)

#define APC_INTR_FIFO_UNDERRUN      APC_INTR_FIFO_UNDERFLOW
#define APC_INTR_FIFO_OVERRUN       APC_INTR_FIFO_OVERFLOW
#define APC_INTR_READY_TO_XFER      APC_INTR_FIFO_DMA_REQ

#define APC_FIFO_INTR_MASK      ( APC_INTR_FIFO_EMPTY | APC_INTR_FIFO_FULL | \
                                  APC_INTR_FIFO_UNDERFLOW | APC_INTR_FIFO_OVERFLOW | \
                                  APC_INTR_FIFO_DMA_REQ )
#define APC_FIFO_INTR_ALL       APC_FIFO_INTR_MASK
#define APC_FIFO_INTR_CNT         5

//__inline static void apc_rx_interrupts_enable(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_RX_MSK &= ~intr_bits; }
//
//__inline static void apc_tx_interrupts_enable(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_TX_MSK &= ~intr_bits; }
//
//__inline static void apc_rx_interrupts_disable(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_RX_MSK |= intr_bits; }
//
//__inline static void apc_tx_interrupts_disable(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_TX_MSK |= intr_bits; }
//
//__inline static void apc_rx_interrupts_clear(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_RX_CLR |= intr_bits; }
//
//__inline static void apc_tx_interrupts_clear(uint32_t intr_bits)
//{ CSK_APC->REG_APC_INTR_TX_MSK |= intr_bits; }


//-------------------------------------------------------------------------------
// APC_TX/RX_CHx_CFG Register (common part)

// APC TX/RX channel SRC/DST
#define RX_CH0_SRC_ADC01    0
#define RX_CH0_SRC_I2S0     1
#define RX_CH0_SRC_VAD      2
#define RX_CH1_SRC_ADC23    0
#define RX_CH1_SRC_I2S1     1
#define RX_CH2_SRC_TX_CH1_LPBK  0
#define RX_CH2_SRC_ECHO         RX_CH2_SRC_TX_CH1_LPBK
#define RX_CH2_SRC_I2S2         1
#define RX_CH2_SRC_I2S1         2
//NO DST select option for TX channel1, fixed for I2S1 output!
#define TX_CH1_DST_DAC01        0
#define TX_CH1_DST_I2S2         1

// channel enable
#define APC_CH_DIS      0
#define APC_CH_EN       1

// dual_channel stereo mode
#define APC_DCH_MONO_MODE           0
#define APC_DCH_STEREO_MODE         1
#define APC_DCH_MIXED_MODE          APC_DCH_STEREO_MODE
#define APC_DCH_SEPARATE_MODE       APC_DCH_MONO_MODE

#define APC_DCH_R_EN                    0x10000
#define APC_DCH_L_EN                    0x1

//dual_channel configuration register bits (common part)
typedef struct _APC_DCH_CFG_REG_BITS {

    volatile uint32_t CH_L_EN                   : 1; // bit 0~0
    volatile uint32_t CH_L_MODE                 : 2; // bit 1~2
    volatile uint32_t CH_L_FIFO_FLUSH           : 1; // bit 3~3
    volatile uint32_t CH_L_FIFO_CNT             : 5; // bit 4~8
    volatile uint32_t RESV_9_15                 : 7; // bit 9~15

    volatile uint32_t CH_R_EN                   : 1; // bit 16~16
    volatile uint32_t CH_R_MODE                 : 2; // bit 17~18
    volatile uint32_t CH_R_FIFO_FLUSH           : 1; // bit 19~19
    volatile uint32_t CH_R_FIFO_CNT             : 5; // bit 20~24

    volatile uint32_t DCH_STEREO_MODE           : 1; // bit 25~25
    volatile uint32_t DCH_DMA_THD_SEL           : 2; // bit 26~27
    volatile uint32_t RESV_28_31                : 4; // bit 28~31

} APC_DCH_CFG_REG_BITS;

//dual_channel configuration register (common part)
typedef union _APC_DCH_CFG_REG {
    volatile uint32_t                     all;
    struct _APC_DCH_CFG_REG_BITS          bit;
} APC_DCH_CFG_REG;

//-------------------------------------------------------------------------------
#define APC_RX_INT_STATUS()                     (CSK_APC->REG_APC_INTR_RX_ISR.all)
#define APC_TX_INT_STATUS()                     (CSK_APC->REG_APC_INTR_TX_ISR.all)
#define APC_RX_INT_RAW_STATUS()                 (CSK_APC->REG_APC_INTR_RX_IRSR.all)
#define APC_TX_INT_RAW_STATUS()                 (CSK_APC->REG_APC_INTR_TX_IRSR.all)
#define APC_CH_INT_STATUS(rx_sta, tx_sta, ch)   ((ch >= APC_CH_IN_COUNT ? \
                                                 tx_sta >> ((ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT) : \
                                                 rx_sta >> (ch * APC_FIFO_INTR_CNT)) & APC_FIFO_INTR_MASK)

#define IS_SET_READY_TO_XFER(ch_sta)            (ch_sta & APC_INTR_READY_TO_XFER)
#define IS_SET_RX_OVERRUN(ch_sta)               (ch_sta & APC_INTR_FIFO_OVERRUN)
#define IS_SET_TX_UNDERRUN(ch_sta)              (ch_sta & APC_INTR_FIFO_UNDERRUN)
#define IS_SET_RX_FULL(ch_sta)                  (ch_sta & APC_INTR_FIFO_FULL)
#define IS_SET_TX_EMPTY(ch_sta)                 (ch_sta & APC_INTR_FIFO_EMPTY)

#endif /* __APC_REG_VENUS_H */
