/*
 * ClockManager.h
 *
 *  Created on: 2021年4月1日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_CLOCKMANAGER_H_
#define INCLUDE_DRIVER_CLOCKMANAGER_H_

#include "Driver_Common.h"
#include "hal/clock_config.h"

// Board configuration
#define A2D32K_FIXDIV_SRC_FREQ    (BOARD_H_XTAL_SRC_FREQ/750)
#define AON32K_CLK_SRC_FREQ       (32768UL)    // TODO 该参数来源与AON clock manager的输出时钟


#define SYSPLL_XTAL_REFM_DIV_MASK    (0xF)
#define SYSPLL_XTAL_REFN_DIV_MASK    (0x3F)


typedef enum _syspll_core_sel {
    SYSPLL_CORE_SEL_DIV3         = 0x0,
    SYSPLL_CORE_SEL_DIV4         = 0x1,
    SYSPLL_CORE_SEL_DIV5         = 0x2,
    SYSPLL_CORE_SEL_DIV6         = 0x3,
} syspll_core_sel_t;


#define AUDIOPLL_XTAL_REFM_DIV_MASK    (0xF)
#define AUDIOPLL_XTAL_REFN_DIV_MASK    (0x3F)


typedef enum _audiopll_core_sel {
    AUDIOPLL_CORE_SEL_DIV3         = 0x0,
    AUDIOPLL_CORE_SEL_DIV4         = 0x1,
    AUDIOPLL_CORE_SEL_DIV5         = 0x2,
    AUDIOPLL_CORE_SEL_DIV6         = 0x3,
} audiopll_core_sel_t;



/*!
 * @brief Clock source for peripherals that support various clock selections.
 */
typedef enum _clock_src_name {
    CRM_IpSrcInvalide    = 0x0U,
    CRM_IpSrcSysPllUsb   = 0x1U,
    CRM_IpSrcSysPllPsram = 0x2U,
    CRM_IpSrcSysPllFlash = 0x4U,
    CRM_IpSrcSysPllCore  = 0x8U,
    CRM_IpSrcAudPllPsram = 0x10U,
    CRM_IpSrcAudPllFlash = 0x20U,
    CRM_IpSrcAudPllCore  = 0x40U,
    CRM_IpSrcAudPllAudio = 0x80U,
    CRM_IpSrcXtal        = 0x100U,
    CRM_IpSrcCmn32K      = 0x200U,
    CRM_IpSrcAon32K      = 0x400U,
} clock_src_name_t;

/*!
 * 各个子模块可以选择 CLOCK SOURCE 的范围
 */
#define CRM_SRC_COMMUNICATE_MASK  (CRM_IpSrcSysPllUsb | CRM_IpSrcXtal)
#define CRM_SRC_COMMUNICATE_SEL_24XTAL  (0x0)
#define CRM_SRC_COMMUNICATE_SEL_SYSPLL  (0x1)

#define CRM_SRC_IO_GPT_MASK       (CRM_IpSrcXtal)

#define CRM_SRC_PSRAM_MASK        (CRM_IpSrcSysPllPsram | CRM_IpSrcAudPllPsram | CRM_IpSrcXtal)
#define CRM_SRC_PSRAM_SEL_24XTAL  (0x0)
#define CRM_SRC_PSRAM_SEL_SYSPLL  (0x1)
#define CRM_SRC_PSRAM_SEL_AUDPLL  (0x2)

#define CRM_SRC_FLASH_MASK        (CRM_IpSrcSysPllFlash | CRM_IpSrcAudPllFlash | CRM_IpSrcXtal)
#define CRM_SRC_FLASH_SEL_24XTAL  (0x0)
#define CRM_SRC_FLASH_SEL_SYSPLL  (0x1)
#define CRM_SRC_FLASH_SEL_AUDPLL  (0x2)

#define CRM_SRC_SYS_ROOT_MASK     (CRM_IpSrcSysPllCore | CRM_IpSrcAudPllCore | CRM_IpSrcXtal)
#define CRM_SRC_SYS_ROOT_SEL_24XTAL  (0x0)
#define CRM_SRC_SYS_ROOT_SEL_SYSPLL  (0x1)
#define CRM_SRC_SYS_ROOT_SEL_AUDPLL  (0x2)

#define CRM_SRC_IR_MASK           (CRM_IpSrcCmn32K | CRM_IpSrcAon32K)
#define CRM_SRC_IR_SEL_LP32K      (0x0)
#define CRM_SRC_IR_SEL_CMN32K     (0x1)

#define CRM_SRC_TIMER_MASK        (CRM_IpSrcCmn32K | CRM_IpSrcAon32K)
#define CRM_SRC_TIMER_SEL_LP32K   (0x0)
#define CRM_SRC_TIMER_SEL_CMN32K  (0x1)

#define CRM_SRC_ADC_MASK          (CRM_IpSrcAudPllAudio | CRM_IpSrcXtal)
#define CRM_SRC_ADC_SEL_24XTAL    (0x0)
#define CRM_SRC_ADC_SEL_AUDPLL    (0x1)

#define CRM_SRC_DAC_MASK          (CRM_IpSrcAudPllAudio | CRM_IpSrcXtal)
#define CRM_SRC_DAC_SEL_24XTAL    (0x0)
#define CRM_SRC_DAC_SEL_AUDPLL    (0x1)


// 各个子模块 divider 的选择 mask，以及是否存在 clock gate
#define CRM_DN_COMMUNICATE_MASK   0x7
#define CRM_DM_COMMUNICATE_MASK   0x7
#define CRM_GATE_COMMUNICATE_MASK 0x0

#define CRM_DN_IO_GPT_MASK        0x1
#define CRM_DM_IO_GPT_MASK        0x7
#define CRM_GATE_IO_GPT_MASK      0x0

#define CRM_DN_PSRAM_MASK         0x1
#define CRM_DM_PSRAM_MASK         0x7
#define CRM_GATE_PSRAM_MASK       0x1

#define CRM_DN_FLASH_MASK         0x1
#define CRM_DM_FLASH_MASK         0x3
#define CRM_GATE_FLASH_MASK       0x0

#define CRM_DN_SYS_ROOT_MASK      0x7
#define CRM_DM_SYS_ROOT_MASK      0x7
#define CRM_GATE_SYS_ROOT_MASK    0x0

#define CRM_DN_CP_PCLK_MASK       0x7
#define CRM_DM_CP_PCLK_MASK       0x7
#define CRM_DN_AP_ROOT_MASK       0x1F
#define CRM_DM_AP_ROOT_MASK       0x1F
#define CRM_DN_AP_PCLK_MASK       0x7
#define CRM_DM_AP_PCLK_MASK       0x7
#define CRM_DN_AP_CMN_CFG_MASK    0x1
#define CRM_DM_AP_CMN_CFG_MASK    0x7

#define CRM_GATE_IR_MASK          0x0

#define CRM_GATE_TIMER_MASK       0x0

#define CRM_GATE_ADC_MASK         0x1

#define CRM_GATE_DAC_MASK         0x1

// Clock gating 宏参数
#define CRM_CLK_ENABLE            0x1
#define CRM_CLK_GATED             0x0

// 各个模块IP的 enumerate 参数
typedef enum _cmn_ip_name {
    // SARADC
    CRM_GpADC,

    CRM_GpioA,
    CRM_GpioB,
    CRM_Gpts,
    CRM_Gptt0,

    CRM_Uart0,
    CRM_Uart1,
    CRM_Uart2,
    CRM_Sdio,
    CRM_Spi0,
    CRM_Spi1,
    CRM_Uart3,

    CRM_Psram,

    CRM_Flash,

    CRM_Usbc,

    // Audio
//    CRM_AudioAonAdc,
    CRM_AudioAdc,
    CRM_AudioDac,

    CRM_Ir,

    CRM_ApWdt,
    CRM_CpWdt,

//    CRM_AonKeysene,

//    CRM_AonIwdt,
//    CRM_AonRtc,
//    CRM_AonCtrl,

//    CRM_AonVad,

//    CRM_AonEfusec,

    // Root Sysclk
    CRM_Sysroot,

    CRM_NpuCoreClk,
    CRM_CpCoreClk,
    CRM_CpHclk,
    CRM_CpPclk,
    CRM_ApRootClk,
    // Ap Sysclk
    CRM_ApCoreClk,
    CRM_ApHclk,
    CRM_ApPclk,
    // Ap Pclk
    CRM_ApPeriPclk,
    CRM_ApCfgPclk,
    CRM_ApCmnPclk,

    // Ap Hclk
    CRM_ApDmac,
    CRM_ApCrypto,

    CRM_ApTimer,

    // Ap Peri Pclk
    CRM_ApI2c0,
    CRM_ApI2c1,
    CRM_ApTrng,
    CRM_ApAbb,

    // Cp Hclk
    CRM_CpApc,
    CRM_CpDmac,
    CRM_FccRamc,

    CRM_CpTimer,
} cmn_ip_name_t;

typedef struct _cmn_divider {
    uint8_t divider_N; // 1, 2, 3 ...... N
    uint8_t divider_M; // 1, 2, 3 ...... M
} cmn_divider_t;


/**
 *
 *              osc
 *               |
 *          -----------
 *          |    |    |
 *         src0 src1 src2
 *          |    |    |
 *          -----------
 *               |
 *          ------------------------
 *          |    |           ...   |
 *         Ip0   Ip1(root)   ...  Ipx
 *               |
 *         ---------------------
 *         |     |     |       |
 *         cp   npu  ap_root cp_pclk
 *                     |
 *                 ----------
 *                 |        |
 *                ap      ap_pclk
 *                          |
 *                   ---------------
 *                   |      |      |
 *                  peri   cfg   cmn_cfg
 */

int32_t SYSPLL_InitPLL(cmn_divider_t div);
//int32_t SYSPLL_DeinitPLL(void);
/*
 * div (/2~/30，step:2)
 * */
int32_t SYSPLL_InitUsb(uint32_t div);

int32_t SYSPLL_InitPsram(uint32_t div);

int32_t SYSPLL_InitFlash(uint32_t div);

int32_t SYSPLL_InitCore(syspll_core_sel_t div);



int32_t AUDIOPLL_InitPLL(cmn_divider_t div, uint32_t nf);
//int32_t AUDIOPLL_DeinitPLL(void);

int32_t AUDIOPLL_InitPsram(uint32_t div);

int32_t AUDIOPLL_InitFlash(uint32_t div);

int32_t AUDIOPLL_InitCore(audiopll_core_sel_t div);

int32_t AUDIOPLL_InitAudio(uint32_t div);


uint32_t CRM_GetSrcFreq(clock_src_name_t src);

/*!
 * @brief Set the clock source for specific Region.
 *
 * Set the clock source for specific Region, region can't choose all
 * clock source, depending on the region mask
 *
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 * @param src Clock source to set, see \ref clock_src_name_t.
 */
int32_t CRM_SetIpSrc(cmn_ip_name_t ip, clock_src_name_t src);

/*!
 * @brief Set the clock divider parameter.
 *
 * Set the clock divider, not all region have divider
 * ,depending on the divider mask
 *
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 * @param div Divider m and n parameter, see \ref cmn_divider_t.
 */
int32_t CRM_SetIpDiv(cmn_ip_name_t ip, cmn_divider_t div);

/*!
 * @brief Set the clock enable.
 *
 * Set the clock enable, not all region have gating
 * ,depending on the gating mask
 *
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 */
int32_t CRM_IpClkEnable(cmn_ip_name_t ip);

/*!
 * @brief Set the clock disenable.
 *
 * Set the clock disenable, not all region have gating
 * ,depending on the gating mask
 *
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 */
int32_t CRM_IpClkDisable(cmn_ip_name_t ip);

/*!
 * @brief Check the clock is enabled.
 *
 * Check the clock is enabled
 *
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 *
 * @return Enable return true, disable return false.
 */
int32_t CRM_IpIsEnabled(cmn_ip_name_t ip);

/*!
 * @brief Get the clock configure parameter.
 *
 * Get the clock configure parameter, include source and divider,
 * but not all have it, depending to corresponding mask.
 *
 * @param src Which get the source, see \ref clock_src_name_t.
 * @param div Which get the divider, see \ref cmn_divider_t.
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 */
int32_t CRM_GetIpConfig(clock_src_name_t* src, cmn_divider_t* div, cmn_ip_name_t ip);

uint32_t CRM_GetIpFreq(cmn_ip_name_t ip);

int32_t CRM_GetIpClkDiv(uint32_t refFreq, uint32_t desireFreq, cmn_divider_t* div, cmn_ip_name_t ip);

#endif /* INCLUDE_DRIVER_CLOCKMANAGER_H_ */
