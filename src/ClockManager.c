/**
  ******************************************************************************
  * @file    ClockManager.c
  * @author  ListenAI Application Team
  * @brief   Clock Manager HAL module driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 ListenAI.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ListenAI under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "ClockManager.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @cond
 */
#ifdef PLATFORM_AP
#define MemBarrier()          __COMPILER_BARRIER()
#elif defined PLATFORM_CP
#include <xtensa/tie/xt_core.h>
extern void XT_ISYNC(void);
#define MemBarrier()          XT_ISYNC()
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif
/** @endcond
 */
/** @defgroup CRM
  * @brief
  * @{
  */
/** @addtogroup CRM_Private_Define
  * @{
  */
/** @defgroup SysPll_XtalMask
  * @{
  */
#define SYSPLL_XTAL_REFM_DIV_MASK    (0xF)
#define SYSPLL_XTAL_REFN_DIV_MASK    (0x3F)
/**
  * @}
  */
/** @defgroup AudPll_XtalMask
  * @{
  */
#define AUDIOPLL_XTAL_REFM_DIV_MASK    (0xF)
#define AUDIOPLL_XTAL_REFN_DIV_MASK    (0x3F)
/**
  * @}
  */
/**
  * @}
  */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @defgroup CRM_PLL_FUNC
  * @{
  */
/** @defgroup CRM_SYSPLL
  * @{
  */
#define WAITTIME_CLOCK100_DIV   10000  //100us
#define WAITTIME_CLOCK10_DIV    100000 //10us
int32_t SYSPLL_InitPLL(uint8_t div_n, uint8_t div_m){
    uint32_t sys_freq, rdata, i = 0;
    uint8_t div_n_g, div_m_g;
    clock_src_name_t src;
    /// Parameter check
    if (div_n > SYSPLL_XTAL_REFN_DIV_MASK || div_n == 0 || \
            div_m > SYSPLL_XTAL_REFM_DIV_MASK || div_m == 0){
        return CSK_DRIVER_ERROR_PARAMETER;
    }
    /// SysPLL check
    if (IP_SYSPLL->REG_0X014.bit.FBDIV_N == div_n && \
            IP_SYSPLL->REG_0X014.bit.REFDIV_N == (div_m - 1)&& \
            IP_ANACORE->REG_0X008.bit.SYSPLL_LOCK == 0x1){
        return CSK_DRIVER_OK;
    }
    /// 检查当前系统是否使用SYS PLL，避免系统跑飞
    HAL_CRM_GetRootClkConfig(&src, &div_n_g, &div_m_g);
    {
        if (src == CRM_IpSrcSysPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }
    /// 获得当前系统时钟
#ifdef PLATFORM_AP
    sys_freq = CRM_GetApFreq();
#elif defined PLATFORM_CP
    sys_freq = CRM_GetCpFreq();
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif
    uint32_t wait_time = sys_freq / WAITTIME_CLOCK10_DIV;
    IP_ANA->REG_0X000.all = 0x0;
    IP_SYSPLL->REG_0X000.all = 0x0;
    do{
        i++;
        rdata = IP_SYSPLL->REG_0X000.all;
    } while(rdata == 0 && i < wait_time);
    wait_time = sys_freq / WAITTIME_CLOCK100_DIV;
    IP_SYSPLL->REG_0X014.bit.FBDIV_N = div_n;
    IP_SYSPLL->REG_0X014.bit.REFDIV_N = (div_m - 1);
    /// Reset. (do this in ROM already, so do nothing here)
    IP_SYSPLL->REG_0X004.all = 0x0;
    /// Wait lock
    do{
        i++;
        rdata = IP_ANACORE->REG_0X008.bit.SYSPLL_LOCK;
    } while(rdata == 0 && i < wait_time);
    // 如果lock超时，手动配置为clock
    if (i >= wait_time){
        IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIS_SYS_PLL_UNLOCK = 0x1;
    }
    return CSK_DRIVER_OK;
}
int32_t SYSPLL_DeinitPLL(){
    uint8_t div_n_g, div_m_g;
    clock_src_name_t src;
    /// 检查当前系统是否使用SYS PLL，避免系统跑飞
    HAL_CRM_GetRootClkConfig(&src, &div_n_g, &div_m_g);
    {
        if (src == CRM_IpSrcSysPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }
    /// Power down
    IP_SYSPLL->REG_0X000.all = 0x7f;
    return CSK_DRIVER_OK;
}
int32_t SYSPLL_InitUsb(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 30) && div % 2 == 0){
=======
    if ((div >= 2 && div <= 30) && div % 2 == 0){
>>>>>>> feature/zephyr
        div_reg = div/2;
        IP_SYSPLL->REG_0X018.bit.POSTDIV_USBCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
#if defined(CSK6001)
int32_t SYSPLL_InitPeri(syspll_peri_sel_t div){
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_AP_PERI = div;
    return CSK_DRIVER_OK;
}
#endif
int32_t SYSPLL_InitCore(syspll_core_sel_t div){
    IP_SYSPLL->REG_0X018.bit.POSTDIV_CORECLK_SEL = div;
    return CSK_DRIVER_OK;
}
int32_t SYSPLL_InitFlash(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 30) && div % 2 == 0){
=======
    if ((div >= 2 && div <= 30) && div % 2 == 0){
>>>>>>> feature/zephyr
        div_reg = div/2;
        IP_SYSPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
int32_t SYSPLL_InitPsram(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 30) && div % 2 == 0){
=======
    if ((div >= 2 && div <= 30) && div % 2 == 0){
>>>>>>> feature/zephyr
        div_reg = div/2;
        IP_SYSPLL->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
/**
  * @}
  */
static uint32_t SYSPLL_GetPLLFreq(void){
    uint32_t freq;
    freq = BOARD_H_XTAL_SRC_FREQ * (IP_SYSPLL->REG_0X014.bit.FBDIV_N) / (IP_SYSPLL->REG_0X014.bit.REFDIV_N + 1);
    return freq;
}
static uint32_t SYSPLL_GetPeriFreq(void){
#if defined(CSK6001)
    uint32_t div;
    div = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_AP_PERI + 10;  // default div = 12
    return SYSPLL_GetPLLFreq()/div;
#endif
#if defined(CSK6001_MPW)
    return SYSPLL_GetPLLFreq()/(IP_SYSPLL->REG_0X018.bit.POSTDIV_USBCLK_SEL * 2);
#endif
}
static uint32_t SYSPLL_GetUsbFreq(void){
    return SYSPLL_GetPLLFreq()/(IP_SYSPLL->REG_0X018.bit.POSTDIV_USBCLK_SEL * 2);
}
static uint32_t SYSPLL_GetPsramFreq(void){
    return SYSPLL_GetPLLFreq()/(IP_SYSPLL->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL * 2);
}
static uint32_t SYSPLL_GetFlashFreq(void){
    return SYSPLL_GetPLLFreq()/(IP_SYSPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL * 2);
}
static uint32_t SYSPLL_GetCoreFreq(void){
    uint8_t div = IP_SYSPLL->REG_0X018.bit.POSTDIV_CORECLK_SEL + 3;
    return SYSPLL_GetPLLFreq()/div;
}
/** @defgroup CRM_AUDPLL
  * @{
  */
int32_t AUDIOPLL_InitPLL(uint8_t div_n, uint8_t div_m, uint32_t nf){
    uint32_t sys_freq, rdata, i = 0;
    uint8_t div_n_g, div_m_g;
    clock_src_name_t src;
    /// Parameter check
    if (div_n > AUDIOPLL_XTAL_REFN_DIV_MASK || div_n == 0 || \
            div_m > AUDIOPLL_XTAL_REFM_DIV_MASK || div_m == 0){
        return CSK_DRIVER_ERROR_PARAMETER;
    }
    /// AudPLL check
    if (IP_AUDPLL->REG_0X020.bit.FBDIV_N == div_n && \
            IP_AUDPLL->REG_0X014.bit.REFDIV_N == (div_m - 1) && \
            IP_AUDPLL->REG_0X020.bit.FBSDM_N == nf && \
            IP_ANACORE->REG_0X008.bit.AUDPLL_LOCK == 0x1){
        return CSK_DRIVER_OK;
    }
    /// 检查当前系统是否使用AUD PLL，避免系统跑飞
    HAL_CRM_GetRootClkConfig(&src, &div_n_g, &div_m_g);
    {
        if (src == CRM_IpSrcAudPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }
    /// 获得当前系统时钟
#ifdef PLATFORM_AP
    sys_freq = CRM_GetApFreq();
#elif defined PLATFORM_CP
    sys_freq = CRM_GetCpFreq();
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif
    uint32_t wait_time = sys_freq / WAITTIME_CLOCK10_DIV;
    /// Reset
    IP_ANA->REG_0X000.all = 0x0;
    IP_AUDPLL->REG_0X000.all = 0x0;
    IP_ANACORE->REG_0X000.all = 0x4;
    do{
        i++;
        rdata = IP_AUDPLL->REG_0X000.all;
    } while(rdata == 0 && i < wait_time);
<<<<<<< HEAD
=======

    /// Reset
    IP_AUDPLL->REG_0X004.all = 0x0;

>>>>>>> feature/zephyr
    wait_time = sys_freq / WAITTIME_CLOCK100_DIV;
    IP_AUDPLL->REG_0X020.bit.FBDIV_N = div_n;
    IP_AUDPLL->REG_0X014.bit.REFDIV_N = (div_m - 1);
    IP_AUDPLL->REG_0X020.bit.FBSDM_N = nf;
    IP_AUDPLL->REG_0X024.bit.FB_DIV_APPLY=1; //write the REFDIV_N/FBSDM_N

<<<<<<< HEAD
    /// Reset
    IP_AUDPLL->REG_0X004.all = 0x0;
=======
//    /// Reset
//    IP_AUDPLL->REG_0X004.all = 0x0;
>>>>>>> feature/zephyr
    /// Wait lock
    do{
        i++;
        rdata = IP_ANACORE->REG_0X008.bit.AUDPLL_LOCK;
    } while(rdata == 0 && i < wait_time);
    /// 如果lock超时，手动配置为clock
    if (i >= wait_time){
        IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIS_AUD_PLL_UNLOCK = 0x1;
    }
    return CSK_DRIVER_OK;
}
#undef WAITTIME_CLOCK100_DIV
#undef WAITTIME_CLOCK10_DIV
int32_t AUDIOPLL_InitPsram(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        IP_AUDPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
=======
    if ((div >= 2 && div <= 30) && div % 2 == 0){
        div_reg = div/2;
        IP_AUDPLL->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL = div_reg;
>>>>>>> feature/zephyr
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
int32_t AUDIOPLL_InitFlash(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 30) && div % 2 == 0){
=======
    if ((div >= 2 && div <= 30) && div % 2 == 0){
>>>>>>> feature/zephyr
        div_reg = div/2;
        IP_AUDPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
int32_t AUDIOPLL_InitCore(audiopll_core_sel_t div){
    IP_AUDPLL->REG_0X018.bit.POSTDIV_CORECLK_SEL = div;
    return CSK_DRIVER_OK;
}
int32_t AUDIOPLL_InitAudio(uint32_t div){
    uint32_t div_reg;
<<<<<<< HEAD
    if ((div >= 2 || div <= 62) && div % 2 == 0){
        div_reg = div/2;
        IP_AUDPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
=======
    if ((div >= 2 && div <= 62) && div % 2 == 0){
        div_reg = div/2;
        IP_AUDPLL->REG_0X018.bit.POSTDIV_AUDCLK_SEL = div_reg;
>>>>>>> feature/zephyr
        return CSK_DRIVER_OK;
    }
    return CSK_DRIVER_ERROR_PARAMETER;
}
/**
  * @}
  */
/// XTAL * (N + NF/2^24) / M
static uint32_t AUDIOPLL_GetPLLFreq(void){
<<<<<<< HEAD
    double nf = ((double)IP_AUDPLL->REG_0X020.bit.FBSDM_N) / (1 << 24);
=======
    double nf = ((double)IP_AUDPLL->REG_0X020.bit.FBSDM_N) / (1 << 23);
>>>>>>> feature/zephyr
    double n_nf = (double)IP_AUDPLL->REG_0X020.bit.FBDIV_N + nf;
    return (uint32_t)(BOARD_H_XTAL_SRC_FREQ * n_nf / (IP_AUDPLL->REG_0X014.bit.REFDIV_N + 1));
}
static uint32_t AUDIOPLL_GetPsramFreq(void){
    return AUDIOPLL_GetPLLFreq() / (IP_AUDPLL->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL * 2);
}
static uint32_t AUDIOPLL_GetFlashFreq(void){
    return AUDIOPLL_GetPLLFreq() / (IP_AUDPLL->REG_0X018.bit.POSTDIV_FLASHCLK_SEL * 2);
}
static uint32_t AUDIOPLL_GetCoreFreq(void){
    uint8_t div = IP_AUDPLL->REG_0X018.bit.POSTDIV_CORECLK_SEL + 3;
    return AUDIOPLL_GetPLLFreq() / div;
}
static uint32_t AUDIOPLL_GetAudioFreq(void){
    return AUDIOPLL_GetPLLFreq() / (IP_AUDPLL->REG_0X018.bit.POSTDIV_AUDCLK_SEL * 2);
}
static inline uint32_t CRM_CLK_XTAL_24MHZ(void){
    return BOARD_H_XTAL_SRC_FREQ;
}
static inline uint32_t CRM_CLK_CMN_32KHZ(void){
    return A2D32K_FIXDIV_SRC_FREQ;
}
static inline uint32_t CRM_CLK_AON_32KHZ(void){
    return (uint32_t)(AON32K_CLK_SRC_FREQ);
}
uint32_t CRM_GetSrcFreq(clock_src_name_t src){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    switch (src){
    case CRM_IpSrcSysPllPeri:
        return SYSPLL_GetPeriFreq();
    case CRM_IpSrcSysPllPsram:
        return SYSPLL_GetPsramFreq();
    case CRM_IpSrcSysPllFlash:
        return SYSPLL_GetFlashFreq();
    case CRM_IpSrcSysPllCore:
        return SYSPLL_GetCoreFreq();
    case CRM_IpSrcAudPllPsram:
        return AUDIOPLL_GetPsramFreq();
    case CRM_IpSrcAudPllFlash:
        return AUDIOPLL_GetFlashFreq();
    case CRM_IpSrcAudPllCore:
        return AUDIOPLL_GetCoreFreq();
    case CRM_IpSrcAudPllAudio:
        return AUDIOPLL_GetAudioFreq();
    case CRM_IpSrcXtal:
        return CRM_CLK_XTAL_24MHZ();
    case CRM_IpSrcCmn32K:
        return CRM_CLK_CMN_32KHZ();
    case CRM_IpSrcAon32K:
        return CRM_CLK_AON_32KHZ();
    case CRM_IpSrcSysPllUsb:
        return SYSPLL_GetUsbFreq();
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }
#endif
}
/**
  * @}
  */
/** @defgroup CRM_IP_CLK_FUNC CRM_IP_CLK_FUNC
  * @brief
  * @{
  */
void HAL_CRM_SetUart0ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetUart0ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcUart0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetUart0Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Uart0ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetUart0ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetUart1ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetUart1ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART1_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART1_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcUart1ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetUart1Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Uart1ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetUart1ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetUart2ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetUart2ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART2_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART2_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcUart2ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetUart2Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Uart2ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetUart2ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetUart3ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_N = div_n;
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_N = div_n;
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetUart3ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcUart3ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetUart3Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Uart3ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetUart3ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetSdioClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_N = div_n;
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetSdioClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcSdioClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetSdioFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_SdioClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetSdioClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetGpioaClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK = div_m;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK_LD = 0x1;
}
int32_t HAL_CRM_CalcGpioaClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    ref = ref / 2;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetGpioaFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ/2;
#else



    uint8_t div_m;
    HAL_CRM_GetGpioaClkConfig(&div_m);
    return CRM_GetSrcFreq(CRM_IpSrcXtal) / div_m / 2;

#endif
}
void HAL_CRM_SetGpiobClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK = div_m;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK_LD = 0x1;
}
int32_t HAL_CRM_CalcGpiobClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    ref = ref / 2;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetGpiobFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ/2;
#else



    uint8_t div_m;
    HAL_CRM_GetGpiobClkConfig(&div_m);
    return CRM_GetSrcFreq(CRM_IpSrcXtal) / div_m / 2;

#endif
}
void HAL_CRM_SetGptsClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS = div_m;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS_LD = 0x1;
}
int32_t HAL_CRM_CalcGptsClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    ref = ref / 2;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetGptsFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ/2;
#else



    uint8_t div_m;
    HAL_CRM_GetGptsClkConfig(&div_m);
    return CRM_GetSrcFreq(CRM_IpSrcXtal) / div_m / 2;

#endif
}
void HAL_CRM_SetGptt0ClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0 = div_m;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0 = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0_LD = 0x1;
}
int32_t HAL_CRM_CalcGptt0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    ref = ref / 2;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetGptt0Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ/2;
#else



    uint8_t div_m;
    HAL_CRM_GetGptt0ClkConfig(&div_m);
    return CRM_GetSrcFreq(CRM_IpSrcXtal) / div_m / 2;

#endif
}
void HAL_CRM_SetSpi0ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_N = div_n;
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_N = div_n;
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetSpi0ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcSpi0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetSpi0Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Spi0ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetSpi0ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
void HAL_CRM_SetSpi1ClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_N = div_n;
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_N = div_n;
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetSpi1ClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPeri){
        IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcSpi1ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetSpi1Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_PHR_FREQ/2;
#else
    if (!HAL_CRM_Spi1ClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetSpi1ClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
uint32_t HAL_CRM_SetIrClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcAon32K){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = 0;
    }

    if (src == CRM_IpSrcCmn32K){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = 1;
    }

    return CSK_DRIVER_OK;
}
uint32_t CRM_GetIrFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_IrClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;


    HAL_CRM_GetIrClkConfig(&src);
    return CRM_GetSrcFreq(src);

#endif
}
uint32_t CRM_GetApcFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_ApcClkIsEnabled()){
        return 0;
    }




    return CRM_GetCpahbFreq();

#endif
}
uint32_t HAL_CRM_SetDacClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = 0;
    }

    if (src == CRM_IpSrcAudPllAudio){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = 1;
    }

    return CSK_DRIVER_OK;
}
uint32_t CRM_GetDacFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_DacClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;


    HAL_CRM_GetDacClkConfig(&src);
    return CRM_GetSrcFreq(src);

#endif
}
uint32_t HAL_CRM_SetAdcClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = 0;
    }

    if (src == CRM_IpSrcAudPllAudio){
        IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = 1;
    }

    return CSK_DRIVER_OK;
}
uint32_t CRM_GetAdcFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_AdcClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;


    HAL_CRM_GetAdcClkConfig(&src);
    return CRM_GetSrcFreq(src);

#endif
}
uint32_t CRM_GetUsbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_UsbClkIsEnabled()){
        return 0;
    }




    return CRM_GetSrcFreq(CRM_IpSrcSysPllUsb);

#endif
}
uint32_t CRM_GetI2c1Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_I2c1ClkIsEnabled()){
        return 0;
    }




    return CRM_GetApperiapbFreq();

#endif
}
uint32_t CRM_GetI2c0Freq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_I2c0ClkIsEnabled()){
        return 0;
    }




    return CRM_GetApperiapbFreq();

#endif
}
uint32_t CRM_GetTrngFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetApperiapbFreq();

#endif
}
uint32_t CRM_GetCryptoFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetApahbFreq();

#endif
}
uint32_t CRM_GetApdmacFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetApahbFreq();

#endif
}
uint32_t CRM_GetCpdmacFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetCpahbFreq();

#endif
}
void HAL_CRM_SetFlashClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK = div_m - 1;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetFlashClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllFlash){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = 1;
    }

    if (src == CRM_IpSrcAudPllFlash){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = 2;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcFlashClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 4; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 4; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetFlashFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_FlashClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;

    uint8_t div_m;
    HAL_CRM_GetFlashClkConfig(&src, &div_m);
    return CRM_GetSrcFreq(src) / div_m;

#endif
}
void HAL_CRM_SetPsramClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK = div_m - 1;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK_LD = 0x1;
}
uint32_t HAL_CRM_SetPsramClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllPsram){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = 1;
    }

    if (src == CRM_IpSrcAudPllPsram){
        IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = 2;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcPsramClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetPsramFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_PsramClkIsEnabled()){
        return 0;
    }
    clock_src_name_t src;;

    uint8_t div_m;
    HAL_CRM_GetPsramClkConfig(&src, &div_m);
    return CRM_GetSrcFreq(src) / div_m;

#endif
}
void HAL_CRM_SetRootClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_N = div_n;
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_N = div_n;
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_LD = 0x1;
}
uint32_t HAL_CRM_SetRootClkSrc(clock_src_name_t src){

    if (src == CRM_IpSrcXtal){
        IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = 0;
    }

    if (src == CRM_IpSrcSysPllCore){
        IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = 1;
    }

    if (src == CRM_IpSrcAudPllCore){
        IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = 2;
    }

    return CSK_DRIVER_OK;
}
int32_t HAL_CRM_CalcRootClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetRootFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else

    clock_src_name_t src;;
    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetRootClkConfig(&src, &div_n, &div_m);
    return CRM_GetSrcFreq(src) * div_n / div_m;

#endif
}
uint32_t CRM_GetNpuFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_NpuClkIsEnabled()){
        return 0;
    }




    return CRM_GetRootFreq();

#endif
}
uint32_t CRM_GetCpFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_CpClkIsEnabled()){
        return 0;
    }




    return CRM_GetRootFreq();

#endif
}
uint32_t CRM_GetCpahbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetRootFreq();

#endif
}
void HAL_CRM_SetCpapbClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_N = div_n;
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_N = div_n;
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_LD = 0x1;
}
int32_t HAL_CRM_CalcCpapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetCpapbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else


    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetCpapbClkConfig(&div_n, &div_m);
    return CRM_GetRootFreq() * div_n / div_m;

#endif
}
void HAL_CRM_SetAprootClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_N = div_n;
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_N = div_n;
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_LD = 0x1;
}
int32_t HAL_CRM_CalcAprootClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 31; m++){
        for (n = 1; n <= 31 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 32; m++){
        for (n = 1; n <= 31 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetAprootFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else


    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetAprootClkConfig(&div_n, &div_m);
    return CRM_GetRootFreq() * div_n / div_m;

#endif
}
uint32_t CRM_GetApFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (!HAL_CRM_ApClkIsEnabled()){
        return 0;
    }




    return CRM_GetAprootFreq();

#endif
}
uint32_t CRM_GetApahbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetAprootFreq();

#endif
}
void HAL_CRM_SetApapbClkDiv(uint8_t div_n, uint8_t div_m){
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_LD = 0x0;
#if defined(CSK6001_MPW)
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_N = div_n;
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M = div_m;
#elif defined(CSK6001)
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_N = div_n;
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_LD = 0x1;
}
int32_t HAL_CRM_CalcApapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 7 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetApapbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else


    uint8_t div_n;
    uint8_t div_m;
    HAL_CRM_GetApapbClkConfig(&div_n, &div_m);
    return CRM_GetAprootFreq() * div_n / div_m;

#endif
}
uint32_t CRM_GetApperiapbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetApapbFreq();

#endif
}
uint32_t CRM_GetApcfgapbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else





    return CRM_GetApapbFreq();

#endif
}
void HAL_CRM_SetApcmnapbClkDiv(uint8_t div_m){
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_LD = 0x0;
#if defined(CSK6001_MPW)

    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M = div_m;
#elif defined(CSK6001)

    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M = div_m-1;
#endif
    MemBarrier();
    IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_LD = 0x1;
}
int32_t HAL_CRM_CalcApcmnapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m){
    uint32_t n, m;
    float coe = (float)des / (float)ref;
<<<<<<< HEAD
=======
    if (coe < 1.0){
        return CSK_DRIVER_ERROR;
    }
>>>>>>> feature/zephyr
    if (coe == 1.0){
        *div_n = 1;
        *div_m = 1;
        return CSK_DRIVER_OK;
    }
#if defined(CSK6001_MPW)
    for (m = 2; m <= 7; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#elif defined(CSK6001)
    for (m = 2; m <= 8; m++){
        for (n = 1; n <= 1 && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                *div_n = n;
                *div_m = m;
                return CSK_DRIVER_OK;
            }
        }
    }
#endif
    return CSK_DRIVER_ERROR;
}
uint32_t CRM_GetApcmnapbFreq(){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else



    uint8_t div_m;
    HAL_CRM_GetApcmnapbClkConfig(&div_m);
    return CRM_GetApapbFreq() / div_m;

#endif
}
/**
  * @}
  */
/**
  * @}
  */
