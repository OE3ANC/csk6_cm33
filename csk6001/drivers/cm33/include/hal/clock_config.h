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

#define BOARD_BOOTCLOCKRUN_SYSCORE_CLK       300000000UL
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


// 该时钟初始化配置包括了 UART0 UART1 UART2 SDIO SPI0 SPI1 这些设备的上电时钟配置
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_SRC    CRM_IpSrcSysPllUsb
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK        BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK        BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK        BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK        BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_SRC     CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK         BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_SRC     CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK         BOARD_H_XTAL_SRC_FREQ  // Divider = bypass

#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_SRC     CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK         BOARD_H_XTAL_SRC_FREQ  // Divider = bypass


// 该时钟初始化配置包括了 GPIOA GPIOB GPTS GPTT0 这些设备的上电时钟配置
#define BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK        BOARD_H_XTAL_SRC_FREQ            // Divider = bypass

#define BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK        BOARD_H_XTAL_SRC_FREQ            // Divider = bypass

#define BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK_SRC     CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK         BOARD_H_XTAL_SRC_FREQ            // Divider = bypass

#define BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK_SRC    CRM_IpSrcXtal
#define BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK        BOARD_H_XTAL_SRC_FREQ            // Divider = bypass


// 该时钟初始化配置包括了 PSRAM 设备的上电时钟配置
#define BOARD_BOOTCLOCKRUN_PSRAM_CLK_SRC           CRM_IpSrcSysPllPsram
#define BOARD_BOOTCLOCKRUN_PSRAM_CLK               200000000UL  // Divider = bypass


// 该时钟初始化配置包括了 FLASH 设备的上电时钟配置
#define BOARD_BOOTCLOCKRUN_FLASH_CLK_SRC           CRM_IpSrcSysPllFlash
#define BOARD_BOOTCLOCKRUN_FLASH_CLK               BOARD_BOOTCLOCKRUN_SYSFLASH_CLK  // Divider = bypass


// 该时钟初始化配置包括了 SYS ROOT 设备的上电时钟配置
#define BOARD_BOOTCLOCKRUN_ROOT_CLK_SRC            CRM_IpSrcSysPllCore  // Core clock source
#define BOARD_BOOTCLOCKRUN_ROOT_CLK                300000000UL   // CP Core clock

#define BOARD_BOOTCLOCKRUN_CP_PCLK_CLK             100000000UL   // CP PCLK
#if BOARD_BOOTCLOCKRUN_CP_PCLK_CLK > 100000000UL || BOARD_BOOTCLOCKRUN_CP_PCLK_CLK > BOARD_BOOTCLOCKRUN_ROOT_CLK
#error "CP Pclk beyond the frequency boundary or greater than the CP core clock"
#endif

#define BOARD_BOOTCLOCKRUN_AP_ROOT_CLK             300000000UL   // AP Core clock
#if BOARD_BOOTCLOCKRUN_AP_ROOT_CLK > BOARD_BOOTCLOCKRUN_ROOT_CLK
#error "AP core clock greater than the CP core clock"
#endif

#define BOARD_BOOTCLOCKRUN_AP_PCLK_CLK             100000000UL   // AP PCLK
#if BOARD_BOOTCLOCKRUN_AP_PCLK_CLK > 100000000UL || BOARD_BOOTCLOCKRUN_AP_PCLK_CLK > BOARD_BOOTCLOCKRUN_AP_ROOT_CLK
#error "AP Pclk beyond the frequency boundary or greater than the AP core clock"
#endif

#define BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK     100000000UL   // AP CMN PCLK
#if BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK > 100000000UL || BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK > BOARD_BOOTCLOCKRUN_AP_PCLK_CLK
#error "AP CMN Pclk beyond the frequency boundary or greater than the AP pclk"
#endif


#define BOARD_BOOTCLOCKRUN_IR_CLK_SRC              CRM_IpSrcCmn32K


#define BOARD_BOOTCLOCKRUN_WDT_CLK_SRC             CRM_IpSrcCmn32K


#define BOARD_BOOTCLOCKRUN_ADC_CLK_SRC             CRM_IpSrcXtal


#define BOARD_BOOTCLOCKRUN_DAC_CLK_SRC             CRM_IpSrcXtal

#endif /* INCLUDE_BSP_CLOCK_CONFIG_H_ */
