/*
 * clock_config.c
 *
 *  Created on: 2021年4月20日
 *      Author: USER
 */
#include "clock_config.h"
#include "ClockManager.h"
#include "venus_ap.h"

void BootClock_Init(){
    uint8_t div_n, div_m;

#if BOARD_BOOTCLOCKRUN_SYSPLL_CLK_DEF
    // System PLL
    div_n = BOARD_BOOTCLOCKRUN_SYSPLL_DIV_N;
    div_m = BOARD_BOOTCLOCKRUN_SYSPLL_DIV_M;
    while(SYSPLL_InitPLL(div_n, div_m)){
        // Change Core source to XTAL
        HAL_CRM_SetRootClkSrc(0);
    }
#endif

#if BOARD_BOOTCLOCKRUN_AUDPLL_CLK_DEF
    // Audio PLL
    div_n = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_N;
    div_m = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_M;
    while(AUDIOPLL_InitPLL(div_n, div_m, BOARD_BOOTCLOCKRUN_AUDPLL_DIV_NF)){
        // Change Core source to XTAL
        HAL_CRM_SetRootClkSrc(0);
    }
#endif

#if BOARD_BOOTCLOCKRUN_SYSCORE_CLK_DEF
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
#endif

#if BOARD_BOOTCLOCKRUN_AUDCORE_CLK_DEF
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
#endif

#if BOARD_BOOTCLOCKRUN_ROOT_CLK_DEF
    // Change Core source
    HAL_CRM_SetRootClkSrc(BOARD_BOOTCLOCKRUN_ROOT_CLK_SRC);
    HAL_CRM_CalcRootClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_ROOT_CLK_SRC), BOARD_BOOTCLOCKRUN_ROOT_CLK, &div_n, &div_m);
    HAL_CRM_SetRootClkDiv(div_n, div_m);
#endif

    // Change CP ROOT domain
#if BOARD_BOOTCLOCKRUN_CP_PCLK_CLK_DEF
    HAL_CRM_CalcCpapbClkDiv(CRM_GetRootFreq(), BOARD_BOOTCLOCKRUN_CP_PCLK_CLK, &div_n, &div_m);
    HAL_CRM_SetCpapbClkDiv(div_n, div_m);
#endif
#if BOARD_BOOTCLOCKRUN_AP_ROOT_CLK_DEF
    HAL_CRM_CalcAprootClkDiv(CRM_GetRootFreq(), BOARD_BOOTCLOCKRUN_AP_ROOT_CLK, &div_n, &div_m);
    HAL_CRM_SetAprootClkDiv(div_n, div_m);
#endif

    // Change AP ROOT domain
#if BOARD_BOOTCLOCKRUN_AP_PCLK_CLK_DEF
    HAL_CRM_CalcApapbClkDiv(CRM_GetAprootFreq(), BOARD_BOOTCLOCKRUN_AP_PCLK_CLK, &div_n, &div_m);
    HAL_CRM_SetApapbClkDiv(div_n, div_m);
#endif

    // Change AP PCLK ROOT domain
#if BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK_DEF
    HAL_CRM_CalcApcmnapbClkDiv(CRM_GetApapbFreq(), BOARD_BOOTCLOCKRUN_AP_CMN_CFG_PCLK_CLK, &div_n, &div_m);
    HAL_CRM_SetApcmnapbClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_SYSUSB_CLK_DEF
    // Change System PLL usb clock source
    SYSPLL_InitUsb(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSUSB_CLK);
#endif

#if BOARD_BOOTCLOCKRUN_SYSPSRAM_CLK_DEF
    // Change System PLL psram clock source
    SYSPLL_InitPsram(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSPSRAM_CLK);
#endif

#if BOARD_BOOTCLOCKRUN_SYSFLASH_CLK_DEF
    // Change System PLL flash clock source
    SYSPLL_InitFlash(BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOTCLOCKRUN_SYSFLASH_CLK);
#endif

#if BOARD_BOOT_CLOCKRUN_SYSPERI_CLK_DEF
    // Change System PLL peripheral clock source
    SYSPLL_InitPeri((BOARD_BOOTCLOCKRUN_SYSPLL_CLK/BOARD_BOOT_CLOCKRUN_SYSPERI_CLK) - 10);
#endif

#if BOARD_BOOTCLOCKRUN_AUD_CLK_DEF
    // Change Audio PLL audio clock source
    AUDIOPLL_InitAudio(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUD_CLK);
#endif

#if BOARD_BOOTCLOCKRUN_AUDPSRAM_CLK_DEF
    // Change Audio PLL psram clock source
    AUDIOPLL_InitPsram(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUDPSRAM_CLK);
#endif

#if BOARD_BOOTCLOCKRUN_AUDFLASH_CLK_DEF
    // Change Audio PLL flash clock source
    AUDIOPLL_InitFlash(BOARD_BOOTCLOCKRUN_AUDPLL_CLK/BOARD_BOOTCLOCKRUN_AUDFLASH_CLK);
#endif

    // Change COMMUNICATE domain
#if BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_DEF
    // uart0
    HAL_CRM_SetUart0ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_SRC);
    HAL_CRM_CalcUart0ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART0_CLK, &div_n, &div_m);
    HAL_CRM_SetUart0ClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_DEF
    // uart1
    HAL_CRM_SetUart1ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_SRC);
    HAL_CRM_CalcUart1ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART1_CLK, &div_n, &div_m);
    HAL_CRM_SetUart1ClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_DEF
    // uart2
    HAL_CRM_SetUart2ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_SRC);
    HAL_CRM_CalcUart2ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART2_CLK, &div_n, &div_m);
    HAL_CRM_SetUart2ClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_DEF
    // uart3
    HAL_CRM_SetUart3ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_SRC);
    HAL_CRM_CalcUart3ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_UART3_CLK, &div_n, &div_m);
    HAL_CRM_SetUart3ClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_DEF
    // sdio
    HAL_CRM_SetSdioClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_SRC);
    HAL_CRM_CalcSdioClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SDIO_CLK, &div_n, &div_m);
    HAL_CRM_SetSdioClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_DEF
    // spi0
    HAL_CRM_SetSpi0ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_SRC);
    HAL_CRM_CalcSpi0ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI0_CLK, &div_n, &div_m);
    HAL_CRM_SetSpi0ClkDiv(div_n, div_m);
#endif

#if BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_DEF
    // spi1
    HAL_CRM_SetSpi1ClkSrc(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_SRC);
    HAL_CRM_CalcSpi1ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK_SRC), BOARD_BOOTCLOCKRUN_COMMUNICATE_SPI1_CLK, &div_n, &div_m);
    HAL_CRM_SetSpi1ClkDiv(div_n, div_m);
#endif

    // Change GPIO_GPT domain
#if BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK_DEF
    // gpioa
    HAL_CRM_CalcGpioaClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPIOA_CLK, &div_n, &div_m);
    HAL_CRM_SetGpioaClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK_DEF
    // gpiob
    HAL_CRM_CalcGpiobClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPIOB_CLK, &div_n, &div_m);
    HAL_CRM_SetGpiobClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK_DEF
    // gpts
    HAL_CRM_CalcGptsClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPTS_CLK, &div_n, &div_m);
    HAL_CRM_SetGptsClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK_DEF
    // gptt0
    HAL_CRM_CalcGptt0ClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK_SRC), BOARD_BOOTCLOCKRUN_IO_GPT_GPTT0_CLK, &div_n, &div_m);
    HAL_CRM_SetGptt0ClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_PSRAM_CLK_DEF
    // Change PSRAM domain
    HAL_CRM_SetPsramClkSrc(BOARD_BOOTCLOCKRUN_PSRAM_CLK_SRC);
    HAL_CRM_CalcPsramClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_PSRAM_CLK_SRC), BOARD_BOOTCLOCKRUN_PSRAM_CLK, &div_n, &div_m);
    HAL_CRM_SetPsramClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_FLASH_CLK_DEF
    // Change FLASH domain
    HAL_CRM_SetFlashClkSrc(BOARD_BOOTCLOCKRUN_FLASH_CLK_SRC);
    HAL_CRM_CalcFlashClkDiv(CRM_GetSrcFreq(BOARD_BOOTCLOCKRUN_FLASH_CLK_SRC), BOARD_BOOTCLOCKRUN_FLASH_CLK, &div_n, &div_m);
    HAL_CRM_SetFlashClkDiv(div_m);
#endif

#if BOARD_BOOTCLOCKRUN_IR_CLK_DEF
    // Change IR domain
    HAL_CRM_SetIrClkSrc(BOARD_BOOTCLOCKRUN_IR_CLK_SRC);
#endif

    // Change TIMER domain
//    CRM_SetIpSrc(BOARD_BOOTCLOCKRUN_WDT_CLK_SRC);   // same as CRM_CpWdt

#if BOARD_BOOTCLOCKRUN_ADC_CLK_DEF
    // Change ADC domain
    HAL_CRM_SetAdcClkSrc(BOARD_BOOTCLOCKRUN_ADC_CLK_SRC);
#endif

#if BOARD_BOOTCLOCKRUN_DAC_CLK_DEF
    // Change DAC domain
    HAL_CRM_SetDacClkSrc(BOARD_BOOTCLOCKRUN_DAC_CLK_SRC);
#endif
}

