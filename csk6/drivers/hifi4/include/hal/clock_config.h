/*
 * clock_config.h
 *
 *  Created on: 2021年4月19日
 *      Author: USER
 */

#ifndef INCLUDE_BSP_CLOCK_CONFIG_H_
#define INCLUDE_BSP_CLOCK_CONFIG_H_

#include "ClockManager.h"

/*
 * 该 header 文件用于表示工程启动后的静态时钟属性
 * ，随着工程的运行，用户可以参考ClockManager.h内
 * 相关 function，获取当前系统的动态时钟属性。
 * 此文件的主要作用是在于如何配置工程的板级时钟
 * */

#define IC_BOARD_FPGA_FIX_FREQ               (48000000)  /// FPGA fix frequence
#define IC_BOARD_FPGA_PHR_FREQ               (60000000)  /// FPGA peripheral frequence

// 芯片板级启动晶振
#define BOARD_H_XTAL_SRC_FREQ                24000000UL
#define BOARD_L_XTAL_SRC_FREQ                32768UL

// SysPll 相关时钟源
#define BOARD_BOOTCLOCKRUN_SYSPLL_CLK        1200000000UL  // XTAL * N / (M)
#define BOARD_BOOTCLOCKRUN_SYSPLL_DIV_N      50
#define BOARD_BOOTCLOCKRUN_SYSPLL_DIV_M      1
// SysPLL的最大最小频率
#define BOARD_BOOTCLOCKRUN_SYSPLL_MIN_FREQ  (600000000UL)
#define BOARD_BOOTCLOCKRUN_SYSPLL_MAX_FREQ  (1800000000UL)
#if (BOARD_BOOTCLOCKRUN_SYSPLL_CLK < BOARD_BOOTCLOCKRUN_SYSPLL_MIN_FREQ) || (BOARD_BOOTCLOCKRUN_SYSPLL_CLK > BOARD_BOOTCLOCKRUN_SYSPLL_MAX_FREQ)
#error "System PLL frequence out of bounds"
#endif

#define BOARD_BOOTCLOCKRUN_SYSUSB_CLK        60000000UL
#if BOARD_BOOTCLOCKRUN_SYSPLL_CLK % BOARD_BOOTCLOCKRUN_SYSUSB_CLK != 0
#error "System PLL domain usb source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_SYSCORE_CLK       200000000UL
#if BOARD_BOOTCLOCKRUN_SYSPLL_CLK % BOARD_BOOTCLOCKRUN_SYSCORE_CLK != 0
#error "System PLL domain core source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_SYSPSRAM_CLK      200000000UL
#if BOARD_BOOTCLOCKRUN_SYSPLL_CLK % BOARD_BOOTCLOCKRUN_SYSPSRAM_CLK != 0
#error "System PLL domain psram source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_SYSFLASH_CLK      120000000UL
#if BOARD_BOOTCLOCKRUN_SYSPLL_CLK % BOARD_BOOTCLOCKRUN_SYSFLASH_CLK != 0
#error "System PLL domain flash source frequence trigger error"
#endif


// AudioPll 相关时钟源
#define BOARD_BOOTCLOCKRUN_AUDPLL_CLK        1190700000UL   // (XTAL * (N + NF/2^24)) / (M)
#define BOARD_BOOTCLOCKRUN_AUDPLL_DIV_N      49
#define BOARD_BOOTCLOCKRUN_AUDPLL_DIV_NF     10276045
#define BOARD_BOOTCLOCKRUN_AUDPLL_DIV_M      1
// AudPLL的最大最小频率
#define BOARD_BOOTCLOCKRUN_AUDPLL_MIN_FREQ  (600000000UL)
#define BOARD_BOOTCLOCKRUN_AUDPLL_MAX_FREQ  (1800000000UL)
#if (BOARD_BOOTCLOCKRUN_AUDPLL_CLK < BOARD_BOOTCLOCKRUN_AUDPLL_MIN_FREQ) || (BOARD_BOOTCLOCKRUN_AUDPLL_CLK > BOARD_BOOTCLOCKRUN_AUDPLL_MAX_FREQ)
#error "Audio PLL frequence out of bounds"
#endif

#define BOARD_BOOTCLOCKRUN_AUDCORE_CLK       198450000UL
#if BOARD_BOOTCLOCKRUN_AUDPLL_CLK % BOARD_BOOTCLOCKRUN_AUDCORE_CLK != 0
#error "Audio PLL domain core source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_AUDPSRAM_CLK      198450000UL
#if BOARD_BOOTCLOCKRUN_AUDPLL_CLK % BOARD_BOOTCLOCKRUN_AUDPSRAM_CLK != 0
#error "Audio PLL domain psram source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_AUDFLASH_CLK      119070000UL
#if BOARD_BOOTCLOCKRUN_AUDPLL_CLK % BOARD_BOOTCLOCKRUN_AUDFLASH_CLK != 0
#error "Audio PLL domain flash source frequence trigger error"
#endif

#define BOARD_BOOTCLOCKRUN_AUD_CLK           22050000UL
#if BOARD_BOOTCLOCKRUN_AUDPLL_CLK % BOARD_BOOTCLOCKRUN_AUD_CLK != 0
#error "Audio PLL domain audio source frequence trigger error"
#endif

#endif /* INCLUDE_BSP_CLOCK_CONFIG_H_ */
