/*
 * PSRAMManager.h
 *
 *  Created on: 2020年11月17日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_PSRAMMANAGER_H_
#define INCLUDE_DRIVER_PSRAMMANAGER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "clock_config.h"

#ifndef CONFIG_PSRAM_SIZE
#define CONFIG_PSRAM_SIZE 0x800000
#endif

#define PSRAM_CHIP_SIZE (CONFIG_PSRAM_SIZE / (1024 * 1024))

#define PSRAM_INNER_SEARCH_LOOP_NUM         64
#define PSRAM_SEARCH_DQS_NUM                64             // word
#if PSRAM_CHIP_SIZE == 4
#define PSRAM_SIZE                          (4*1024*1024)  // bytes
#elif PSRAM_CHIP_SIZE == 8
#define PSRAM_SIZE                          (8*1024*1024)  // bytes
#else
#error Unsupported PSRAM size
#endif
#define PSRAM_EYE_DIAGRAM_TOP               (0xB4)
#define PSRAM_EYE_DIAGRAM_BOTTOM            (0x0)

#define PSRAM_REFRESH_TIME                  (3.6)          // microsecond
#define __INNER_MICROSEC                    (1000000)

typedef enum _prefetch_level{
    _prefetch_level3 = 3,     /// check write address hit read data FIFO
    _prefetch_level2 = 2,     /// always clean read FIFO when write
    _prefetch_level1 = 1,     /// never mind write operate
    _prefetch_level0 = 0,     /// non-prefetch
} prefetch_level;

//*********************CONTROL
#define PSRAM_LOG_CHECK                     (1)
#define PSRAM_ODT_ENA                       (0)
#if defined(CSK6001)
#define PSRAM_PREFETCH_LEVEL                _prefetch_level2
#define PSRAM_PREFETCH_PRED_NUM             2
#define PSRAM_PREFETCH_FIFO_DEPTH           0x40

#define PSRAM_CS_TMO_INT_ENABLE             1
#define PSRAM_RD_TMO_INT_ENABLE             1
#endif

// Search dqs
#define PSRAM_RD_DQS_SEARCH                 (0)
#define PSRAM_WT_DQS_SEARCH                 (0)

// 采用默认的wt 或者 rd 的delay参数
#define DEFAULT_WT_DELAY                    (0x2d)
#define DEFAULT_RD_DELAY                    (0x15)

#if defined(CSK6001_MPW)
#if BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 200000000UL
#define PSRAM_DQS_RD_DELAY                  (0x11)
#define PSRAM_DQS_WR_DELAY                  (0x1a)
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 150000000UL
#define PSRAM_DQS_RD_DELAY                  (0x1a)
#define PSRAM_DQS_WR_DELAY                  (0x23)
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 100000000UL
#define PSRAM_DQS_RD_DELAY                  (0x2c)
#define PSRAM_DQS_WR_DELAY                  (0x35)
#else
#define PSRAM_DQS_RD_DELAY                  (0x5a)
#define PSRAM_DQS_WR_DELAY                  (0x5f)
#endif
#endif

// tcph = 6 (200mhz)
//      = 4 (133mhz)
#if BOARD_BOOTCLOCKRUN_PSRAM_CLK > 166000000UL
#define CONTROLLER_TIME_CONFIG_TCPH         (0x6)// bit0-7
#else
#define CONTROLLER_TIME_CONFIG_TCPH         (0x2)// bit0-7
#endif
// tcem * clock < 4us
// PSRAM 刷新时间,固定4us，但是随着clock升高需要确定是多少个clock
// TODO ************************************
// 非常重要的提醒，现在的设置都是在-40°-80°之间的参数
// 如果温度非常温，提高到105°，则 最大的刷新时间4us会降低到1us，到时参数需要重新设计
#if defined(CSK6001_MPW)
#if BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 170000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (6)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 85000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (5)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 42000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (4)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 21000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (3)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 11000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (2)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 5000000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (1)// bit8-11
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 2600000UL
#define CONTROLLER_TIME_CONFIG_TCEM         (0)// bit8-11
#endif
#endif
#define CONTROLLER_TIME_CONFIG_TCSP         (0x0)// bit12-15

#if BOARD_BOOTCLOCKRUN_PSRAM_CLK >= 166000000UL
#define CONTROLLER_MEM_R_WAIT_CYCLES        (6)
#define CONTROLLER_MEM_W_WAIT_CYCLES        (1)
#define CONTROLLER_MODE_R_WAIT_CYCLES       (6)
//******************MR0
#define PSRAM_MEM_R_WAIT_CYCLES             (0x6)      // bit2-4
//******************MR4
#define PSRAM_MEM_W_WAIT_CYCLES             (0x4)      // bit 5-7      // WL0 = 0;   WL2 = 0x4
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK > 133000000UL
#define CONTROLLER_MEM_R_WAIT_CYCLES        (5)
#define CONTROLLER_MEM_W_WAIT_CYCLES        (0)
#define CONTROLLER_MODE_R_WAIT_CYCLES       (5)
//******************MR0
#define PSRAM_MEM_R_WAIT_CYCLES             (0x5)      // bit2-4
//******************MR4
#define PSRAM_MEM_W_WAIT_CYCLES             (0x0)      // bit 5-7      // WL0 = 0;   WL2 = 0x4
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK > 109000000UL
#define CONTROLLER_MEM_R_WAIT_CYCLES        (4)
#define CONTROLLER_MEM_W_WAIT_CYCLES        (0)
#define CONTROLLER_MODE_R_WAIT_CYCLES       (4)
//******************MR0
#define PSRAM_MEM_R_WAIT_CYCLES             (0x4)      // bit2-4
//******************MR4
#define PSRAM_MEM_W_WAIT_CYCLES             (0x0)      // bit 5-7      // WL0 = 0;   WL2 = 0x4
#elif BOARD_BOOTCLOCKRUN_PSRAM_CLK > 66000000UL
#define CONTROLLER_MEM_R_WAIT_CYCLES        (3)
#define CONTROLLER_MEM_W_WAIT_CYCLES        (0)
#define CONTROLLER_MODE_R_WAIT_CYCLES       (3)
//******************MR0
#define PSRAM_MEM_R_WAIT_CYCLES             (0x3)      // bit2-4
//******************MR4
#define PSRAM_MEM_W_WAIT_CYCLES             (0x0)      // bit 5-7      // WL0 = 0;   WL2 = 0x4
#else
#define CONTROLLER_MEM_R_WAIT_CYCLES        (2)
#define CONTROLLER_MEM_W_WAIT_CYCLES        (0)
#define CONTROLLER_MODE_R_WAIT_CYCLES       (2)
//******************MR0
#define PSRAM_MEM_R_WAIT_CYCLES             (0x2)      // bit2-4
//******************MR4
#define PSRAM_MEM_W_WAIT_CYCLES             (0x0)      // bit 5-7      // WL0 = 0;   WL2 = 0x4
#endif

// the ports in group1 have high priority than the ports in group0
// port0 HIFI4 AHB MASTER
// port1 CP DMA MASTER
// port2 NPU INST DMA
// port3 NPU DATA DMA
// port4 AP-TO-CP MASTER
// port5 DVP
// bit[N]=1: port N in group1
// bit[N]=0: port N in group0
#define CONTROLLER_PRIO_FIXED               (0x01)
#define CONTROLLER_PRIO_ROUND_ROBIN         (0x3f)
#define CONTROLLER_PRIO_SETTING             (CONTROLLER_PRIO_FIXED)

// bypass 置1时，表示DQS delay采用delay cells去延迟，反之表示，采用百分比延迟
#define PSRAM_DLL_BYPASS                    (0x1)

//******************MR0
#if PSRAM_CHIP_SIZE == 4
// For 4M PSRAM
// 0x0 1/16
// 0x1 Half
// 0x2 1/4
// 0x3 1/8
#define PSRAM_MR0_DRIVE_STR                 (0x1)      // bit0-1
#elif PSRAM_CHIP_SIZE == 8
// 0x0 Full (25Ω)
// 0x1 Half (50Ω)
// 0x2 1/4 (100Ω)
// 0x3 1/8 (200Ω default)
#define PSRAM_MR0_DRIVE_STR                 (0x0)      // bit0-1
#endif
#define PSRAM_MR0_LT                        (0x1)      // bit5

//******************MR1-MR2
#define PSRAM_MR1_VENDOR_ID_OFFSET          (0x0)
#define PSRAM_MR2_DEV_ID_OFFSET             (0x3)
#define PSRAM_MR2_GB_OFFSET                 (0x7)

//******************MR3
// SRF-> 0   mean: Slow Refresh
//    -> 1   mean: Fast Refresh
#define PSRAM_MR3_SRF                       (0x1)      // bit5
#define PSRAM_MR3_VCC                       (0x0)      // bit6

//******************MR4
#define PSRAM_MR4_PASR                      (0x0)      // bit0-2
// RF-> 0   mean: Fast Refresh
//   -> 1   mean: Enables Slow Refresh when temperature allows
#define PSRAM_MR4_RF                        (0x0)      // bit3
// Diff in chip die of 4M and 8M
#if PSRAM_CHIP_SIZE == 4
#define PSRAM_MR4_WL_MASK                   (0x1)
#define PSRAM_MR4_WL_POS                    (7)
#define PSRAM_MR4_WL_SHIFT                  (2)
#elif PSRAM_CHIP_SIZE == 8
#define PSRAM_MR4_WL_MASK                   (0x7)
#define PSRAM_MR4_WL_POS                    (5)
#define PSRAM_MR4_WL_SHIFT                  (0)
#endif

//******************MR8
#define PSRAM_MR8_BL                        (0x3)      // bit0-1       // warp 1k
#define PSRAM_MR8_BT                        (0x0)      // bit2

int32_t PSRAM_Initialize(void);

// when using sleep mode or deep sleep mode,
// must avoid other master call psram
void PSRAM_HalfSleep_Mode(void);
void PSRAM_Exit_HalfSleep_Mode(void);
void PSRAM_DeepSleep_Mode(void);
void PSRAM_Exit_DeepSleep_Mode(void);

#endif /* INCLUDE_DRIVER_PSRAMMANAGER_H_ */
