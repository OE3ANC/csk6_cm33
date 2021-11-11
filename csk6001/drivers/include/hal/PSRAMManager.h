/*
 * PSRAMManager.h
 *
 *  Created on: 2020年11月17日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_PSRAMMANAGER_H_
#define INCLUDE_DRIVER_PSRAMMANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include "clock_config.h"

//*********************CONTROL
#define TEST_MODE                           (0)
#define PSRAM_LOG_CHECK                     (0)
#define PSRAM_ODT_ENA                       (0)

// Search dqs
#define PSRAM_RD_DQS_SEARCH                 (1)
#define PSRAM_WT_DQS_SEARCH                 (1)
// 采用默认的wt 或者 rd 的delay参数
#define DEFAULT_WT_DELAY                    (0)
#define DEFAULT_RD_DELAY                    (0)
#define PSRAM_DQS_RD_DELAY                  (0x60)
#define PSRAM_DQS_WR_DELAY                  (0x60)

#define PSRAM_SEARCH_DQS_NUM                (1024 * 4)
#define PSRAM_SIZE                          (8*1024*1024)  // bytes

// 这些值需要和PSRAM mode register的匹配

// tcph = 6 (200mhz)
//      = 4 (133mhz)
#if BOARD_BOOTCLOCKRUN_PSRAM_CLK > 166000000UL
#define CONTROLLER_TIME_CONFIG_TCPH         (0x6)// bit0-7
#else
#define CONTROLLER_TIME_CONFIG_TCPH         (0x4)// bit0-7
#endif
// tcem * clock < 4us
// PSRAM 刷新时间,固定4us，但是随着clock升高需要确定是多少个clock
// TODO ************************************
// 非常重要的提醒，现在的设置都是在-40°-80°之间的参数
// 如果温度非常温，提高到105°，则 最大的刷新时间4us会降低到1us，到时参数需要重新设计
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
// bit[N]=1: port N in group1
// bit[N]=0: port N in group0
#define CONTROLLER_PRIO_FIXED               (0x1f)
#define CONTROLLER_PRIO_ROUND_ROBIN         (0x0)
#define CONTROLLER_PRIO_SETTING             (CONTROLLER_PRIO_FIXED)

// bypass 置1时，表示DQS delay采用delay cells去延迟，反之表示，采用百分比延迟
#define PSRAM_DLL_BYPASS                    (0x1)

//******************MR0
// 0x0 Full (25Ω)
// 0x1 Half (50Ω)
// 0x2 1/4 (100Ω)
// 0x3 1/8 (200Ω default)
#define PSRAM_MR0_DRIVE_STR                 (0x0)      // bit0-1
#define PSRAM_MR0_LT                        (0x0)      // bit5

//******************MR4
#define PSRAM_MR4_PASR                      (0x0)      // bit0-2
#define PSRAM_MR4_RF                        (0x0)      // bit3


//******************MR8
#define PSRAM_MR8_BL                        (0x3)      // bit0-1       // warp 1k
#define PSRAM_MR8_BT                        (0x0)      // bit2

void PSRAM_Initialize(void);

#endif /* INCLUDE_DRIVER_PSRAMMANAGER_H_ */
