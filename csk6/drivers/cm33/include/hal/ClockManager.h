/**
  ******************************************************************************
  * @file    ClockManager.h
  * @author  ListenAI Application Team
  * @brief   Header file of CRM HAL module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INCLUDE_DRIVER_CLOCKMANAGER_H_
#define INCLUDE_DRIVER_CLOCKMANAGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"
#include "clock_config.h"

#ifdef PLATFORM_AP
#include "venus_ap.h"
#elif defined PLATFORM_CP
#include "venus_cp.h"
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @defgroup CRM
  * @brief
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup CRM_Exported_Types CRM Exported Types
  * @{
  */

/** @defgroup CRM_SYSPLL_CORE_SEL
  * @brief  CRM System Pll clock divider enumeration
  * @{
  */
/** @enum
  * @brief  CRM system pll core clock divider
  */
typedef enum _syspll_core_sel {
    SYSPLL_CORE_SEL_DIV3         = 0x0,//!< SYSPLL_CORE_SEL_DIV3
    SYSPLL_CORE_SEL_DIV4         = 0x1,//!< SYSPLL_CORE_SEL_DIV4
    SYSPLL_CORE_SEL_DIV5         = 0x2,//!< SYSPLL_CORE_SEL_DIV5
    SYSPLL_CORE_SEL_DIV6         = 0x3,//!< SYSPLL_CORE_SEL_DIV6
} syspll_core_sel_t;
/**
  * @}
  */

/** @defgroup CRM_SYSPLL_PERI_SEL
  * @brief  CRM System Pll peripheral clock divider enumeration
  * @{
  */
/** @enum
  * @brief  CRM system pll peripheral clock divider
  */
typedef enum _syspll_peri_sel {
    SYSPLL_PERI_SEL_DIV10        = 0x0,
    SYSPLL_PERI_SEL_DIV11        = 0x1,
    SYSPLL_PERI_SEL_DIV12        = 0x2,
    SYSPLL_PERI_SEL_DIV13        = 0x3,
} syspll_peri_sel_t;
/**
  * @}
  */

/** @defgroup CRM_AUDPLL_CORE_SEL
  * @brief  CRM Audio Pll clock divider enumeration
  * @{
  */
/** @enum
  * @brief  CRM audio pll core clock divider
  */
typedef enum _audiopll_core_sel {
    AUDIOPLL_CORE_SEL_DIV3         = 0x0,//!< AUDPLL_CORE_SEL_DIV3
    AUDIOPLL_CORE_SEL_DIV4         = 0x1,//!< AUDPLL_CORE_SEL_DIV4
    AUDIOPLL_CORE_SEL_DIV5         = 0x2,//!< AUDPLL_CORE_SEL_DIV5
    AUDIOPLL_CORE_SEL_DIV6         = 0x3,//!< AUDPLL_CORE_SEL_DIV6
} audiopll_core_sel_t;
/**
  * @}
  */

/** @defgroup CRM_CLK_SRC
  * @brief Clock source for peripherals that support various clock selections enumeration
  * @{
  */
///
/// clock source name
///
typedef enum _clock_src_name {
    CRM_IpSrcInvalide    = 0x0U,  //!< CRM_IpSrcInvalide
    CRM_IpSrcSysPllPeri  = 0x1U,  //!< CRM_IpSrcSysPllPeri
    CRM_IpSrcSysPllPsram = 0x2U,  //!< CRM_IpSrcSysPllPsram
    CRM_IpSrcSysPllFlash = 0x4U,  //!< CRM_IpSrcSysPllFlash
    CRM_IpSrcSysPllCore  = 0x8U,  //!< CRM_IpSrcSysPllCore
    CRM_IpSrcAudPllPsram = 0x10U, //!< CRM_IpSrcAudPllPsram
    CRM_IpSrcAudPllFlash = 0x20U, //!< CRM_IpSrcAudPllFlash
    CRM_IpSrcAudPllCore  = 0x40U, //!< CRM_IpSrcAudPllCore
    CRM_IpSrcAudPllAudio = 0x80U, //!< CRM_IpSrcAudPllAudio
    CRM_IpSrcXtal        = 0x100U,//!< CRM_IpSrcXtal
    CRM_IpSrcCmn32K      = 0x200U,//!< CRM_IpSrcCmn32K
    CRM_IpSrcAon32K      = 0x400U,//!< CRM_IpSrcAon32K
    CRM_IpSrcSysPllUsb   = 0x800U,//!< CRM_IpSrcSysPllUsb
} clock_src_name_t;
/**
  * @}
  */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup CRM_Exported_Constants CRM Exported Constants
  * @{
  */

/** @defgroup CRM_32K_CLK
  * @{
  */
///
/// cmn 32k crystal.
///
#define A2D32K_FIXDIV_SRC_FREQ    (BOARD_H_XTAL_SRC_FREQ/750)
///
/// aon 32k (generate by RC circuit).
///
#define AON32K_CLK_SRC_FREQ       (32768UL)
/**
  * @}
  */
/**
  * @}
  */

/** @defgroup CRM_IP_CLK_FUNC CRM_IP_CLK_FUNC
  * @brief
  * @{
  */

/** @defgroup _CRM_UART0 UART0_CLK_FUNC
  * @brief UART0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the UART0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART0_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_UART0_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the UART0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART0_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_UART0_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the UART0 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Uart0ClkIsEnabled(){
    return IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_UART0_CLK;
}

/**
  * @brief  Function to configure the UART0 clock source
  * @param  src specifies which source that UART0 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: UART0 using CRM_IpSrcXtal to source clock
  *         @arg 1: UART0 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetUart0ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the UART0 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetUart0ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the UART0 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetUart0ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the UART0 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcUart0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get UART0 running clock
 * @return UART0 current clock,if UART0 in disable stage return 0.
 */
uint32_t CRM_GetUart0Freq();
/**
  * @}
  */

/** @defgroup _CRM_UART1 UART1_CLK_FUNC
  * @brief UART1 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the UART1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART1_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART1_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the UART1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART1_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART1_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the UART1 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Uart1ClkIsEnabled(){
    return IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART1_CLK;
}

/**
  * @brief  Function to configure the UART1 clock source
  * @param  src specifies which source that UART1 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: UART1 using CRM_IpSrcXtal to source clock
  *         @arg 1: UART1 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetUart1ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the UART1 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetUart1ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the UART1 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetUart1ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART1_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the UART1 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcUart1ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get UART1 running clock
 * @return UART1 current clock,if UART1 in disable stage return 0.
 */
uint32_t CRM_GetUart1Freq();
/**
  * @}
  */

/** @defgroup _CRM_UART2 UART2_CLK_FUNC
  * @brief UART2 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the UART2 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART2_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART2_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the UART2 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART2_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART2_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the UART2 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Uart2ClkIsEnabled(){
    return IP_SYSCTRL->REG_UART_CLK_CFG.bit.ENA_UART2_CLK;
}

/**
  * @brief  Function to configure the UART2 clock source
  * @param  src specifies which source that UART2 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: UART2 using CRM_IpSrcXtal to source clock
  *         @arg 1: UART2 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetUart2ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the UART2 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetUart2ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the UART2 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetUart2ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_UART_CLK_CFG.bit.SEL_UART2_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the UART2 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcUart2ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get UART2 running clock
 * @return UART2 current clock,if UART2 in disable stage return 0.
 */
uint32_t CRM_GetUart2Freq();
/**
  * @}
  */

/** @defgroup _CRM_UART3 UART3_CLK_FUNC
  * @brief UART3 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the UART3 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART3_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.ENA_UART3_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the UART3 peripheral clock.
  * @return None
  */
#define __HAL_CRM_UART3_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.ENA_UART3_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the UART3 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Uart3ClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.ENA_UART3_CLK;
}

/**
  * @brief  Function to configure the UART3 clock source
  * @param  src specifies which source that UART3 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: UART3 using CRM_IpSrcXtal to source clock
  *         @arg 1: UART3 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetUart3ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the UART3 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetUart3ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the UART3 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetUart3ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the UART3 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcUart3ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get UART3 running clock
 * @return UART3 current clock,if UART3 in disable stage return 0.
 */
uint32_t CRM_GetUart3Freq();
/**
  * @}
  */

/** @defgroup _CRM_SDIO SDIO_CLK_FUNC
  * @brief SDIO clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the SDIO peripheral clock.
  * @return None
  */
#define __HAL_CRM_SDIO_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_SDIO_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the SDIO peripheral clock.
  * @return None
  */
#define __HAL_CRM_SDIO_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_SDIO_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the SDIO peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_SdioClkIsEnabled(){
    return IP_SYSCTRL->REG_UART_SDIO_CLK.bit.ENA_SDIO_CLK;
}

/**
  * @brief  Function to configure the SDIO clock source
  * @param  src specifies which source that SDIO can choose
  *         This parameter can be one of the following values:
  *         @arg 0: SDIO using CRM_IpSrcXtal to source clock
  *         @arg 1: SDIO using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetSdioClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the SDIO clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetSdioClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the SDIO clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetSdioClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the SDIO clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcSdioClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get SDIO running clock
 * @return SDIO current clock,if SDIO in disable stage return 0.
 */
uint32_t CRM_GetSdioFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPIOA GPIOA_CLK_FUNC
  * @brief GPIOA clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the GPIOA clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetGpioaClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the GPIOA clock configuration
  * @param div_m
  */
inline static void HAL_CRM_GetGpioaClkConfig(uint8_t* div_m){


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK + 1;
#endif

}

/**
 * @brief  Function to calculate the GPIOA clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcGpioaClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get GPIOA running clock
 * @return GPIOA current clock,if GPIOA in disable stage return 0.
 */
uint32_t CRM_GetGpioaFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPIOB GPIOB_CLK_FUNC
  * @brief GPIOB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the GPIOB clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetGpiobClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the GPIOB clock configuration
  * @param div_m
  */
inline static void HAL_CRM_GetGpiobClkConfig(uint8_t* div_m){


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK + 1;
#endif

}

/**
 * @brief  Function to calculate the GPIOB clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcGpiobClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get GPIOB running clock
 * @return GPIOB current clock,if GPIOB in disable stage return 0.
 */
uint32_t CRM_GetGpiobFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPTS GPTS_CLK_FUNC
  * @brief GPTS clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the GPTS clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetGptsClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the GPTS clock configuration
  * @param div_m
  */
inline static void HAL_CRM_GetGptsClkConfig(uint8_t* div_m){


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS + 1;
#endif

}

/**
 * @brief  Function to calculate the GPTS clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcGptsClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get GPTS running clock
 * @return GPTS current clock,if GPTS in disable stage return 0.
 */
uint32_t CRM_GetGptsFreq();
/**
  * @}
  */

/** @defgroup _CRM_GPTT0 GPTT0_CLK_FUNC
  * @brief GPTT0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the GPTT0 clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetGptt0ClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the GPTT0 clock configuration
  * @param div_m
  */
inline static void HAL_CRM_GetGptt0ClkConfig(uint8_t* div_m){


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0 + 1;
#endif

}

/**
 * @brief  Function to calculate the GPTT0 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcGptt0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get GPTT0 running clock
 * @return GPTT0 current clock,if GPTT0 in disable stage return 0.
 */
uint32_t CRM_GetGptt0Freq();
/**
  * @}
  */

/** @defgroup _CRM_SPI0 SPI0_CLK_FUNC
  * @brief SPI0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the SPI0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_SPI0_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI0_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the SPI0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_SPI0_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI0_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the SPI0 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Spi0ClkIsEnabled(){
    return IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI0_CLK;
}

/**
  * @brief  Function to configure the SPI0 clock source
  * @param  src specifies which source that SPI0 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: SPI0 using CRM_IpSrcXtal to source clock
  *         @arg 1: SPI0 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetSpi0ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the SPI0 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetSpi0ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the SPI0 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetSpi0ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the SPI0 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcSpi0ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get SPI0 running clock
 * @return SPI0 current clock,if SPI0 in disable stage return 0.
 */
uint32_t CRM_GetSpi0Freq();
/**
  * @}
  */

/** @defgroup _CRM_SPI1 SPI1_CLK_FUNC
  * @brief SPI1 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the SPI1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_SPI1_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI1_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the SPI1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_SPI1_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI1_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the SPI1 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Spi1ClkIsEnabled(){
    return IP_SYSCTRL->REG_SPI_CLK_CFG.bit.ENA_SPI1_CLK;
}

/**
  * @brief  Function to configure the SPI1 clock source
  * @param  src specifies which source that SPI1 can choose
  *         This parameter can be one of the following values:
  *         @arg 0: SPI1 using CRM_IpSrcXtal to source clock
  *         @arg 1: SPI1 using CRM_IpSrcSysPllPeri to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetSpi1ClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the SPI1 clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetSpi1ClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the SPI1 clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetSpi1ClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPeri;
    }

    *div_n = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the SPI1 clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcSpi1ClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get SPI1 running clock
 * @return SPI1 current clock,if SPI1 in disable stage return 0.
 */
uint32_t CRM_GetSpi1Freq();
/**
  * @}
  */

/** @defgroup _CRM_IR IR_CLK_FUNC
  * @brief IR clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the IR peripheral clock.
  * @return None
  */
#define __HAL_CRM_IR_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_IR_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the IR peripheral clock.
  * @return None
  */
#define __HAL_CRM_IR_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_IR_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the IR peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_IrClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_IR_CLK;
}

/**
  * @brief  Function to configure the IR clock source
  * @param  src specifies which source that IR can choose
  *         This parameter can be one of the following values:
  *         @arg 0: IR using CRM_IpSrcAon32K to source clock
  *         @arg 1: IR using CRM_IpSrcCmn32K to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetIrClkSrc(clock_src_name_t src);

/**
  * @brief  Function to get the IR clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  */
inline static void HAL_CRM_GetIrClkConfig(clock_src_name_t* src){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcAon32K;
    }

    if (src_t == 1){
        *src = CRM_IpSrcCmn32K;
    }



}

/**
 * @brief  Function to get IR running clock
 * @return IR current clock,if IR in disable stage return 0.
 */
uint32_t CRM_GetIrFreq();
/**
  * @}
  */

/** @defgroup _CRM_APC APC_CLK_FUNC
  * @brief APC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the APC peripheral clock.
  * @return None
  */
#define __HAL_CRM_APC_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_APC_PCLK = 0x1; \
} while(0)
/**
  * @brief Disable the APC peripheral clock.
  * @return None
  */
#define __HAL_CRM_APC_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_APC_PCLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the APC peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_ApcClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_APC_PCLK;
}

/**
 * @brief  Function to get APC running clock
 * @return APC current clock,if APC in disable stage return 0.
 */
uint32_t CRM_GetApcFreq();
/**
  * @}
  */

/** @defgroup _CRM_DAC DAC_CLK_FUNC
  * @brief DAC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the DAC peripheral clock.
  * @return None
  */
#define __HAL_CRM_DAC_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 0x1; \
} while(0)
/**
  * @brief Disable the DAC peripheral clock.
  * @return None
  */
#define __HAL_CRM_DAC_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the DAC peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_DacClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC;
}

/**
  * @brief  Function to configure the DAC clock source
  * @param  src specifies which source that DAC can choose
  *         This parameter can be one of the following values:
  *         @arg 0: DAC using CRM_IpSrcXtal to source clock
  *         @arg 1: DAC using CRM_IpSrcAudPllAudio to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetDacClkSrc(clock_src_name_t src);

/**
  * @brief  Function to get the DAC clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  */
inline static void HAL_CRM_GetDacClkConfig(clock_src_name_t* src){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcAudPllAudio;
    }



}

/**
 * @brief  Function to get DAC running clock
 * @return DAC current clock,if DAC in disable stage return 0.
 */
uint32_t CRM_GetDacFreq();
/**
  * @}
  */

/** @defgroup _CRM_ADC ADC_CLK_FUNC
  * @brief ADC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the ADC peripheral clock.
  * @return None
  */
#define __HAL_CRM_ADC_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 0x1; \
} while(0)
/**
  * @brief Disable the ADC peripheral clock.
  * @return None
  */
#define __HAL_CRM_ADC_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the ADC peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_AdcClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC;
}

/**
  * @brief  Function to configure the ADC clock source
  * @param  src specifies which source that ADC can choose
  *         This parameter can be one of the following values:
  *         @arg 0: ADC using CRM_IpSrcXtal to source clock
  *         @arg 1: ADC using CRM_IpSrcAudPllAudio to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetAdcClkSrc(clock_src_name_t src);

/**
  * @brief  Function to get the ADC clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  */
inline static void HAL_CRM_GetAdcClkConfig(clock_src_name_t* src){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcAudPllAudio;
    }



}

/**
 * @brief  Function to get ADC running clock
 * @return ADC current clock,if ADC in disable stage return 0.
 */
uint32_t CRM_GetAdcFreq();
/**
  * @}
  */

/** @defgroup _CRM_XTAL32K XTAL32K_CLK_FUNC
  * @brief XTAL32K clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the XTAL32K peripheral clock.
  * @return None
  */
#define __HAL_CRM_XTAL32K_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_XTAL_DIV_32K = 0x1; \
} while(0)
/**
  * @brief Disable the XTAL32K peripheral clock.
  * @return None
  */
#define __HAL_CRM_XTAL32K_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_XTAL_DIV_32K = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the XTAL32K peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_Xtal32kClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_XTAL_DIV_32K;
}

/**
  * @}
  */

/** @defgroup _CRM_USB USB_CLK_FUNC
  * @brief USB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the USB peripheral clock.
  * @return None
  */
#define __HAL_CRM_USB_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_USBC_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the USB peripheral clock.
  * @return None
  */
#define __HAL_CRM_USB_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_USBC_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the USB peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_UsbClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_USBC_CLK;
}

/**
 * @brief  Function to get USB running clock
 * @return USB current clock,if USB in disable stage return 0.
 */
uint32_t CRM_GetUsbFreq();
/**
  * @}
  */

/** @defgroup _CRM_I2C1 I2C1_CLK_FUNC
  * @brief I2C1 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the I2C1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_I2C1_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C1_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the I2C1 peripheral clock.
  * @return None
  */
#define __HAL_CRM_I2C1_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C1_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the I2C1 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_I2c1ClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C1_CLK;
}

/**
 * @brief  Function to get I2C1 running clock
 * @return I2C1 current clock,if I2C1 in disable stage return 0.
 */
uint32_t CRM_GetI2c1Freq();
/**
  * @}
  */

/** @defgroup _CRM_I2C0 I2C0_CLK_FUNC
  * @brief I2C0 clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the I2C0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_I2C0_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C0_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the I2C0 peripheral clock.
  * @return None
  */
#define __HAL_CRM_I2C0_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C0_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the I2C0 peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_I2c0ClkIsEnabled(){
    return IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C0_CLK;
}

/**
 * @brief  Function to get I2C0 running clock
 * @return I2C0 current clock,if I2C0 in disable stage return 0.
 */
uint32_t CRM_GetI2c0Freq();
/**
  * @}
  */

/** @defgroup _CRM_TRNG TRNG_CLK_FUNC
  * @brief TRNG clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get TRNG running clock
 * @return TRNG current clock,if TRNG in disable stage return 0.
 */
uint32_t CRM_GetTrngFreq();
/**
  * @}
  */

/** @defgroup _CRM_CRYPTO CRYPTO_CLK_FUNC
  * @brief CRYPTO clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get CRYPTO running clock
 * @return CRYPTO current clock,if CRYPTO in disable stage return 0.
 */
uint32_t CRM_GetCryptoFreq();
/**
  * @}
  */

/** @defgroup _CRM_APDMAC APDMAC_CLK_FUNC
  * @brief APDMAC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get APDMAC running clock
 * @return APDMAC current clock,if APDMAC in disable stage return 0.
 */
uint32_t CRM_GetApdmacFreq();
/**
  * @}
  */

/** @defgroup _CRM_CPDMAC CPDMAC_CLK_FUNC
  * @brief CPDMAC clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get CPDMAC running clock
 * @return CPDMAC current clock,if CPDMAC in disable stage return 0.
 */
uint32_t CRM_GetCpdmacFreq();
/**
  * @}
  */

/** @defgroup _CRM_FLASH FLASH_CLK_FUNC
  * @brief FLASH clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the FLASH peripheral clock.
  * @return None
  */
#define __HAL_CRM_FLASH_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the FLASH peripheral clock.
  * @return None
  */
#define __HAL_CRM_FLASH_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the FLASH peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_FlashClkIsEnabled(){
    return IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK;
}

/**
  * @brief  Function to configure the FLASH clock source
  * @param  src specifies which source that FLASH can choose
  *         This parameter can be one of the following values:
  *         @arg 0: FLASH using CRM_IpSrcXtal to source clock
  *         @arg 1: FLASH using CRM_IpSrcSysPllFlash to source clock
  *         @arg 2: FLASH using CRM_IpSrcAudPllFlash to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetFlashClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the FLASH clock divider
  * @param div_m must less than or equal to 3
  */
void HAL_CRM_SetFlashClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the FLASH clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_m
  */
inline static void HAL_CRM_GetFlashClkConfig(clock_src_name_t* src, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllFlash;
    }

    if (src_t == 2){
        *src = CRM_IpSrcAudPllFlash;
    }


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK + 1;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK + 1;
#endif

}

/**
 * @brief  Function to calculate the FLASH clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcFlashClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get FLASH running clock
 * @return FLASH current clock,if FLASH in disable stage return 0.
 */
uint32_t CRM_GetFlashFreq();
/**
  * @}
  */

/** @defgroup _CRM_PSRAM PSRAM_CLK_FUNC
  * @brief PSRAM clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the PSRAM peripheral clock.
  * @return None
  */
#define __HAL_CRM_PSRAM_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the PSRAM peripheral clock.
  * @return None
  */
#define __HAL_CRM_PSRAM_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the PSRAM peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_PsramClkIsEnabled(){
    return IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK;
}

/**
  * @brief  Function to configure the PSRAM clock source
  * @param  src specifies which source that PSRAM can choose
  *         This parameter can be one of the following values:
  *         @arg 0: PSRAM using CRM_IpSrcXtal to source clock
  *         @arg 1: PSRAM using CRM_IpSrcSysPllPsram to source clock
  *         @arg 2: PSRAM using CRM_IpSrcAudPllPsram to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetPsramClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the PSRAM clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetPsramClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the PSRAM clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_m
  */
inline static void HAL_CRM_GetPsramClkConfig(clock_src_name_t* src, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllPsram;
    }

    if (src_t == 2){
        *src = CRM_IpSrcAudPllPsram;
    }


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK + 1;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK + 1;
#endif

}

/**
 * @brief  Function to calculate the PSRAM clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcPsramClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get PSRAM running clock
 * @return PSRAM current clock,if PSRAM in disable stage return 0.
 */
uint32_t CRM_GetPsramFreq();
/**
  * @}
  */

/** @defgroup _CRM_ROOT ROOT_CLK_FUNC
  * @brief ROOT clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the ROOT clock source
  * @param  src specifies which source that ROOT can choose
  *         This parameter can be one of the following values:
  *         @arg 0: ROOT using CRM_IpSrcXtal to source clock
  *         @arg 1: ROOT using CRM_IpSrcSysPllCore to source clock
  *         @arg 2: ROOT using CRM_IpSrcAudPllCore to source clock
  * @return CSK HAL status
  */
uint32_t HAL_CRM_SetRootClkSrc(clock_src_name_t src);

/**
  * @brief  Function to configure the ROOT clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetRootClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the ROOT clock configuration
  * @param src, this parameter can be a value of @ref CRM_CLK_SRC
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetRootClkConfig(clock_src_name_t* src, uint8_t* div_n, uint8_t* div_m){
    uint8_t src_t;
    src_t = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK;
    if (src_t == 0){
        *src = CRM_IpSrcXtal;
    }

    if (src_t == 1){
        *src = CRM_IpSrcSysPllCore;
    }

    if (src_t == 2){
        *src = CRM_IpSrcAudPllCore;
    }

    *div_n = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M + 1;
#endif

}

/**
 * @brief  Function to calculate the ROOT clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcRootClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get ROOT running clock
 * @return ROOT current clock,if ROOT in disable stage return 0.
 */
uint32_t CRM_GetRootFreq();
/**
  * @}
  */

/** @defgroup _CRM_NPU NPU_CLK_FUNC
  * @brief NPU clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the NPU peripheral clock.
  * @return None
  */
#define __HAL_CRM_NPU_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the NPU peripheral clock.
  * @return None
  */
#define __HAL_CRM_NPU_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the NPU peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_NpuClkIsEnabled(){
    return IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK;
}

/**
 * @brief  Function to get NPU running clock
 * @return NPU current clock,if NPU in disable stage return 0.
 */
uint32_t CRM_GetNpuFreq();
/**
  * @}
  */

/** @defgroup _CRM_CP CP_CLK_FUNC
  * @brief CP clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the CP peripheral clock.
  * @return None
  */
#define __HAL_CRM_CP_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the CP peripheral clock.
  * @return None
  */
#define __HAL_CRM_CP_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the CP peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_CpClkIsEnabled(){
    return IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK;
}

/**
 * @brief  Function to get CP running clock
 * @return CP current clock,if CP in disable stage return 0.
 */
uint32_t CRM_GetCpFreq();
/**
  * @}
  */

/** @defgroup _CRM_CPAHB CPAHB_CLK_FUNC
  * @brief CPAHB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get CPAHB running clock
 * @return CPAHB current clock,if CPAHB in disable stage return 0.
 */
uint32_t CRM_GetCpahbFreq();
/**
  * @}
  */

/** @defgroup _CRM_CPAPB CPAPB_CLK_FUNC
  * @brief CPAPB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the CPAPB clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetCpapbClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the CPAPB clock configuration
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetCpapbClkConfig(uint8_t* div_n, uint8_t* div_m){

    *div_n = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the CPAPB clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcCpapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get CPAPB running clock
 * @return CPAPB current clock,if CPAPB in disable stage return 0.
 */
uint32_t CRM_GetCpapbFreq();
/**
  * @}
  */

/** @defgroup _CRM_APROOT APROOT_CLK_FUNC
  * @brief APROOT clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the APROOT clock divider
  * @param div_n must less than or equal to 31
  * @param div_m must less than or equal to 31
  */
void HAL_CRM_SetAprootClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the APROOT clock configuration
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetAprootClkConfig(uint8_t* div_n, uint8_t* div_m){

    *div_n = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M + 1;
#endif

}

/**
 * @brief  Function to calculate the APROOT clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcAprootClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get APROOT running clock
 * @return APROOT current clock,if APROOT in disable stage return 0.
 */
uint32_t CRM_GetAprootFreq();
/**
  * @}
  */

/** @defgroup _CRM_AP AP_CLK_FUNC
  * @brief AP clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief Enable the AP peripheral clock.
  * @return None
  */
#define __HAL_CRM_AP_CLK_ENABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK = 0x1; \
} while(0)
/**
  * @brief Disable the AP peripheral clock.
  * @return None
  */
#define __HAL_CRM_AP_CLK_DISABLE()    \
do { \
    IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK = 0x0; \
} while(0)
/**
  * @brief Get the enable or disable status of the AP peripheral clock.
  * @return True or False
  */
inline static uint8_t HAL_CRM_ApClkIsEnabled(){
    return IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK;
}

/**
 * @brief  Function to get AP running clock
 * @return AP current clock,if AP in disable stage return 0.
 */
uint32_t CRM_GetApFreq();
/**
  * @}
  */

/** @defgroup _CRM_APAHB APAHB_CLK_FUNC
  * @brief APAHB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get APAHB running clock
 * @return APAHB current clock,if APAHB in disable stage return 0.
 */
uint32_t CRM_GetApahbFreq();
/**
  * @}
  */

/** @defgroup _CRM_APAPB APAPB_CLK_FUNC
  * @brief APAPB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the APAPB clock divider
  * @param div_n must less than or equal to 7
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetApapbClkDiv(uint8_t div_n, uint8_t div_m);

/**
  * @brief  Function to get the APAPB clock configuration
  * @param div_n
  * @param div_m
  */
inline static void HAL_CRM_GetApapbClkConfig(uint8_t* div_n, uint8_t* div_m){

    *div_n = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_N;
#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the APAPB clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcApapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get APAPB running clock
 * @return APAPB current clock,if APAPB in disable stage return 0.
 */
uint32_t CRM_GetApapbFreq();
/**
  * @}
  */

/** @defgroup _CRM_APPERIAPB APPERIAPB_CLK_FUNC
  * @brief APPERIAPB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get APPERIAPB running clock
 * @return APPERIAPB current clock,if APPERIAPB in disable stage return 0.
 */
uint32_t CRM_GetApperiapbFreq();
/**
  * @}
  */

/** @defgroup _CRM_APCFGAPB APCFGAPB_CLK_FUNC
  * @brief APCFGAPB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
 * @brief  Function to get APCFGAPB running clock
 * @return APCFGAPB current clock,if APCFGAPB in disable stage return 0.
 */
uint32_t CRM_GetApcfgapbFreq();
/**
  * @}
  */

/** @defgroup _CRM_APCMNAPB APCMNAPB_CLK_FUNC
  * @brief APCMNAPB clock control function which can enable, disable or get status from corresponding
  * device, set ip clock source, set ip clock divider, or get ip clock configuration, help you calculate the ip divider
  * parameter when having ip reference clock and desire clock
  * @{
  */

/**
  * @brief  Function to configure the APCMNAPB clock divider
  * @param div_m must less than or equal to 7
  */
void HAL_CRM_SetApcmnapbClkDiv(uint8_t div_m);

/**
  * @brief  Function to get the APCMNAPB clock configuration
  * @param div_m
  */
inline static void HAL_CRM_GetApcmnapbClkConfig(uint8_t* div_m){


#if defined(CSK6001_MPW)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M;
#elif defined(CSK6001)
    *div_m = IP_SYSCTRL->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M + 1;
#endif

}

/**
 * @brief  Function to calculate the APCMNAPB clock divider according to the reference value and
 * desired value
 * @param ref reference clock
 * @param des desired clock
 * @param div_n uint8_t type pointer
 * @param div_m uint8_t type pointer
 * @return CSK HAL status
 */
int32_t HAL_CRM_CalcApcmnapbClkDiv(uint32_t ref, uint32_t des, uint8_t* div_n, uint8_t* div_m);

/**
 * @brief  Function to get APCMNAPB running clock
 * @return APCMNAPB current clock,if APCMNAPB in disable stage return 0.
 */
uint32_t CRM_GetApcmnapbFreq();
/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup CRM_PLL_FUNC
  * @{
  */
/** @addtogroup CRM_SYSPLL
  * @{
  */
/**
 *
 * @param div_n @ref SysPll_XtalMask
 * @param div_m @ref SysPll_XtalMask
 * @return CSK HAL status
 */
int32_t SYSPLL_InitPLL(uint8_t div_n, uint8_t div_m);

/**
 *
 * @return CSK HAL status
 */
int32_t SYSPLL_DeinitPLL(void);

/**
 *
 * @param div @ref CRM_SYSPLL_PERI_SEL
 * @return CSK HAL status
 */
int32_t SYSPLL_InitPeri(syspll_peri_sel_t div);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t SYSPLL_InitUsb(uint32_t div);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t SYSPLL_InitPsram(uint32_t div);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t SYSPLL_InitFlash(uint32_t div);

/**
 *
 * @param div @ref CRM_SYSPLL_CORE_SEL
 * @return CSK HAL status
 */
int32_t SYSPLL_InitCore(syspll_core_sel_t div);

/**
  * @}
  */

/** @addtogroup CRM_AUDPLL
  * @{
  */
/**
 *
 * @param div_n @ref AudPll_XtalMask
 * @param div_m @ref AudPll_XtalMask
 * @param nf
 * @return CSK HAL status
 */
int32_t AUDIOPLL_InitPLL(uint8_t div_n, uint8_t div_m, uint32_t nf);

/**
 *
 * @return CSK HAL status
 */
int32_t AUDIOPLL_DeinitPLL(void);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t AUDIOPLL_InitPsram(uint32_t div);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t AUDIOPLL_InitFlash(uint32_t div);

/**
 *
 * @param div @ref CRM_AUDPLL_CORE_SEL
 * @return CSK HAL status
 */
int32_t AUDIOPLL_InitCore(audiopll_core_sel_t div);

/**
 *
 * @param div (/2~/30，step:2)
 * @return CSK HAL status
 */
int32_t AUDIOPLL_InitAudio(uint32_t div);

/**
  * @}
  */
/**
 *
 * @param src @ref CRM_CLK_SRC
 * @return source clock frequence
 */
uint32_t CRM_GetSrcFreq(clock_src_name_t src);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/** @example clockmanager_nos_chk.c
 *
 */
#endif /* INCLUDE_DRIVER_CLOCKMANAGER_H_ */
