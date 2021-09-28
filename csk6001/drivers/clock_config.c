/*
 * clock_config.c
 *
 *  Created on: 2021年4月20日
 *      Author: USER
 */

#include "hal/reg/syspll_ctrl_reg_venus.h"
#include "hal/reg/audpll_ctrl_reg_venus.h"

#include "hal/clock_config.h"
#include "hal/ClockManager.h"
#include "hal/venus_ap.h"

#define SYSPLL_CTRL_BASE      ((SYSPLL_RegDef*)   CMN_SYSPLL_CTRL_BASE)
#define AUDPLL_CTRL_BASE      ((AUDPLL_RegDef*)   CMN_AUDPLL_CTRL_BASE)

void BootClock_Init(){
    cmn_divider_t div;

    // System PLL
    div.divider_N = BOARD_BOOTCLOCKRUN_SYSPLL_DIV_N;
    div.divider_M = BOARD_BOOTCLOCKRUN_SYSPLL_DIV_M;
    while(SYSPLL_InitPLL(div)){
        // Change Core source to XTAL
        CRM_SetIpSrc(CRM_Sysroot, CRM_IpSrcXtal);
    }

    // Audio PLL
    div.divider_N = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_N;
    div.divider_M = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_M;
    while(AUDIOPLL_InitPLL(div, BOARD_BOOTCLOCKRUN_AUDPLL_DIV_NF)){
        // Change Core source to XTAL
        CRM_SetIpSrc(CRM_Sysroot, CRM_IpSrcXtal);
    }

    // Change System PLL core clock source
    switch(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSCORE_CLK){
    case 3:
        SYSPLL_InitCore(SYSPLL_CORE_SEL_DIV3);
        break;
    case 4:
        SYSPLL_InitCore(SYSPLL_CORE_SEL_DIV4);
        break;
    case 5:
        SYSPLL_InitCore(SYSPLL_CORE_SEL_DIV5);
        break;
    case 6:
        SYSPLL_InitCore(SYSPLL_CORE_SEL_DIV6);
        break;
    }

    // Change Audio PLL core clock source
    switch(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUDCORE_CLK){
    case 3:
        AUDIOPLL_InitCore(AUDIOPLL_CORE_SEL_DIV3);
        break;
    case 4:
        AUDIOPLL_InitCore(AUDIOPLL_CORE_SEL_DIV4);
        break;
    case 5:
        AUDIOPLL_InitCore(AUDIOPLL_CORE_SEL_DIV5);
        break;
    case 6:
        AUDIOPLL_InitCore(AUDIOPLL_CORE_SEL_DIV6);
        break;
    }

    // Change Core source
    CRM_SetIpSrc(CRM_Sysroot, BOARD_BOOTCLOCKRUN_ROOT_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_ROOT_CLK_SRC), BOARD_BOOTCLOCKRUN_ROOT_CLK, &div, CRM_Sysroot);
    CRM_SetIpDiv(CRM_Sysroot, div);

    // Change CP ROOT domain
    CRM_GetIpClkDiv(CRM_GetIpFreq(CRM_Sysroot), BOARD_BOOTCLOCKRUN_CP_PCLK_CLK, &div, CRM_CpPclk);
    CRM_SetIpDiv(CRM_CpPclk, div);
    CRM_GetIpClkDiv(CRM_GetIpFreq(CRM_Sysroot), BOARD_BOOTCLOCKRUN_AP_ROOT_CLK, &div, CRM_ApRootClk);
    CRM_SetIpDiv(CRM_ApRootClk, div);

    // Change AP ROOT domain
    CRM_GetIpClkDiv(CRM_GetIpFreq(CRM_ApRootClk), BOARD_BOOTCLOCKRUN_AP_PCLK_CLK, &div, CRM_ApPclk);
    CRM_SetIpDiv(CRM_ApPclk, div);

    // Change AP PCLK ROOT domain
    CRM_GetIpClkDiv(CRM_GetIpFreq(CRM_ApPclk), BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK, &div, CRM_ApCmnPclk);
    CRM_SetIpDiv(CRM_ApCmnPclk, div);

    // Change System PLL usb clock source
    SYSPLL_InitUsb(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSUSB_CLK);

    // Change System PLL psram clock source
    SYSPLL_InitPsram(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSPSRAM_CLK);

    // Change System PLL flash clock source
    SYSPLL_InitFlash(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSFLASH_CLK);


    // Change Audio PLL audio clock source
    AUDIOPLL_InitAudio(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUD_CLK);

    // Change Audio PLL psram clock source
    AUDIOPLL_InitPsram(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUDPSRAM_CLK);

    // Change Audio PLL flash clock source
    AUDIOPLL_InitFlash(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUDFLASH_CLK);


    // Change COMMUNICATE domain
    // uart0
    CRM_SetIpSrc(CRM_Uart0, BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK, &div, CRM_Uart0);
    CRM_SetIpDiv(CRM_Uart0, div);

    // uart1
    CRM_SetIpSrc(CRM_Uart1, BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK, &div, CRM_Uart1);
    CRM_SetIpDiv(CRM_Uart1, div);

    // uart2
    CRM_SetIpSrc(CRM_Uart2, BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK, &div, CRM_Uart2);
    CRM_SetIpDiv(CRM_Uart2, div);

    // uart3
    CRM_SetIpSrc(CRM_Uart3, BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK, &div, CRM_Uart3);
    CRM_SetIpDiv(CRM_Uart3, div);

    // sdio
    CRM_SetIpSrc(CRM_Sdio, BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK, &div, CRM_Sdio);
    CRM_SetIpDiv(CRM_Sdio, div);

    // spi0
    CRM_SetIpSrc(CRM_Spi0, BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK, &div, CRM_Spi0);
    CRM_SetIpDiv(CRM_Spi0, div);

    // spi1
    CRM_SetIpSrc(CRM_Spi1, BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK, &div, CRM_Spi1);
    CRM_SetIpDiv(CRM_Spi1, div);

    // Change GPIO_GPT domain
    // gpioa
    CRM_SetIpSrc(CRM_GpioA, BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK, &div, CRM_GpioA);
    CRM_SetIpDiv(CRM_GpioA, div);

    // gpiob
    CRM_SetIpSrc(CRM_GpioB, BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK, &div, CRM_GpioB);
    CRM_SetIpDiv(CRM_GpioB, div);

    // gpts
    CRM_SetIpSrc(CRM_Gpts, BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK, &div, CRM_Gpts);
    CRM_SetIpDiv(CRM_Gpts, div);

    // gptt0
    CRM_SetIpSrc(CRM_Gptt0, BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK, &div, CRM_Gptt0);
    CRM_SetIpDiv(CRM_Gptt0, div);

    // Change PSRAM domain
    CRM_SetIpSrc(CRM_Psram, BOARD_BOOTCLOCKRUN_PSRAM_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_PSRAM_CLK_SRC), BOARD_BOOTCLOCKRUN_PSRAM_CLK, &div, CRM_Psram);
    CRM_SetIpDiv(CRM_Psram, div);

    // Change FLASH domain
    CRM_SetIpSrc(CRM_Flash, BOARD_BOOTCLOCKRUN_FLASH_CLK_SRC);
    CRM_GetIpClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_FLASH_CLK_SRC), BOARD_BOOTCLOCKRUN_FLASH_CLK, &div, CRM_Flash);
    CRM_SetIpDiv(CRM_Flash, div);

    // Change IR domain
    CRM_SetIpSrc(CRM_Ir, BOARD_BOOTCLOCKRUN_IR_CLK_SRC);

    // Change TIMER domain
    CRM_SetIpSrc(CRM_ApWdt, BOARD_BOOTCLOCKRUN_WDT_CLK_SRC);   // same as CRM_CpWdt

    // Change ADC domain
    CRM_SetIpSrc(CRM_AudioAdc, BOARD_BOOTCLOCKRUN_ADC_CLK_SRC);

    // Change DAC domain
    CRM_SetIpSrc(CRM_AudioDac, BOARD_BOOTCLOCKRUN_DAC_CLK_SRC);
}

