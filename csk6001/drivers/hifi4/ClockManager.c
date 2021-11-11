/*
 * ClockManager.c
 *
 *  Created on: 2021年4月6日
 *      Author: USER
 */

#define PLATFORM_CP
#include "hal/ClockManager.h"
#ifdef PLATFORM_AP
#include "hal/venus_ap.h"
#elif defined PLATFORM_CP
#include "hal/venus_cp.h"
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif
#include "hal/reg/syspll_ctrl_reg_venus.h"
#include "hal/reg/audpll_ctrl_reg_venus.h"
#include "hal/reg/cmn_sysctrl_reg_venus.h"
#include "hal/reg/ana_core_regfile_reg_venus.h"
#include "hal/reg/ana_aon_regfile_reg_venus.h"

#define SYSPLL_CTRL_BASE      ((SYSPLL_RegDef*)   CMN_SYSPLL_CTRL_BASE)
#define AUDPLL_CTRL_BASE      ((AUDPLL_RegDef*)   CMN_AUDPLL_CTRL_BASE)
#define SYSTEM_CTRL_BASE      ((SYSCFG_RegDef*)   CMN_SYSCTRL_BASE)
#define CORE_CTRL_BASE        ((ANA_CORE_RegDef*) CMN_ANACORE_BASE)
#define ANA_AON_BASE          ((ANA_RegDef*)      AON_AON_BASE)

#define IC_BOARD_FPGA_FIX_FREQ    (24000000)

uint32_t CRM_GetApCoreSysClkFreq(void);
uint32_t CRM_GetCpCoreSysClkFreq(void);

#ifdef PLATFORM_AP
#define MemBarrier          __COMPILER_BARRIER()
#elif defined PLATFORM_CP
#define MemBarrier          __asm__("isync")
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif

/*!
 * @brief Configure the system PLL parameter.
 *
 * Configure the system PLL, must deinitialize system PLL first, and
 * then configure the divider or other parameter.
 *
 * @param div
 */

#define WAITTIME_CLOCK100_DIV   10000  //100us
#define WAITTIME_CLOCK10_DIV    100000 //10us

int32_t SYSPLL_InitPLL(cmn_divider_t div){
    uint32_t sys_freq, rdata, i = 0;
    clock_src_name_t src;
    cmn_divider_t div_g;

    // Parameter check
    if (div.divider_N > SYSPLL_XTAL_REFN_DIV_MASK || div.divider_N == 0 || \
            div.divider_M > SYSPLL_XTAL_REFM_DIV_MASK || div.divider_M == 0){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // SysPLL check
    if (SYSPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N == div.divider_N && \
            SYSPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N == (div.divider_M - 1)&& \
            CORE_CTRL_BASE->REG_0X008.bit.SYSPLL_LOCK == 0x1){
        return CSK_DRIVER_OK;
    }

    // 检查当前系统是否使用SYS PLL，避免系统跑飞
    if (CRM_GetIpConfig(&src, &div_g, CRM_Sysroot))
    {
        if (src == CRM_IpSrcSysPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }

    // 获得当前系统时钟
#ifdef PLATFORM_AP
    sys_freq = CRM_GetApCoreSysClkFreq();

#elif defined PLATFORM_CP
    sys_freq = CRM_GetCpCoreSysClkFreq();
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif
    uint32_t wait_time = sys_freq / WAITTIME_CLOCK10_DIV;

    ANA_AON_BASE->REG_0X000.all = 0x0;
    SYSPLL_CTRL_BASE->REG_0X000.all = 0x0;

    do{
        i++;
        rdata = SYSPLL_CTRL_BASE->REG_0X000.all;
    } while(rdata == 0 && i < wait_time);


    wait_time = sys_freq / WAITTIME_CLOCK100_DIV;
    SYSPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N = div.divider_N;
    SYSPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N = (div.divider_M - 1);

    // Reset. (do this in ROM already, so do nothing here)
    SYSPLL_CTRL_BASE->REG_0X004.all = 0x0;

    // Wait lock
    do{
        i++;
        rdata = CORE_CTRL_BASE->REG_0X008.bit.SYSPLL_LOCK;
    } while(rdata == 0 && i < wait_time);

    // 如果lock超时，手动配置为clock
    if (i >= wait_time){
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIS_SYS_PLL_UNLOCK = 0x1;
    }

    return CSK_DRIVER_OK;
}

int32_t SYSPLL_DeinitPLL(){
    clock_src_name_t src;

    // 检查当前系统是否使用SYS PLL，避免系统跑飞
    if (CRM_GetIpConfig(&src, NULL, CRM_Sysroot))
    {
        if (src == CRM_IpSrcSysPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }

    // Power down
    SYSPLL_CTRL_BASE->REG_0X000.all = 0x7f;

    return CSK_DRIVER_OK;
}

static uint32_t SYSPLL_GetPLLFreq(void){
    uint32_t freq;
    freq = BOARD_H_XTAL_SRC_FREQ * (SYSPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N) / (SYSPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N + 1);
    return freq;
}

/*
 * Post divider ratio for FOUT1 (/3, /4, /5, /6), default: /6
 * Post divider ratio for FOUT2 (/2~/30，step:2), default: /6
 * Post divider ratio for FOUT3 (/2~/30，step:2), default: /10
 * Post divider ratio for FOUT4 (/2~/30，step:2), default: /54
 * */
int32_t SYSPLL_InitUsb(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_USBCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t SYSPLL_GetUsbFreq(void){
    return SYSPLL_GetPLLFreq()/(SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_USBCLK_SEL * 2);
}

int32_t SYSPLL_InitPsram(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t SYSPLL_GetPsramFreq(void){
    return SYSPLL_GetPLLFreq()/(SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL * 2);
}

int32_t SYSPLL_InitFlash(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t SYSPLL_GetFlashFreq(void){
    return SYSPLL_GetPLLFreq()/(SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL * 2);
}

int32_t SYSPLL_InitCore(syspll_core_sel_t div){
    SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_CORECLK_SEL = div;
    return CSK_DRIVER_OK;
}

static uint32_t SYSPLL_GetCoreFreq(void){
    uint8_t div = SYSPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_CORECLK_SEL + 3;
    return SYSPLL_GetPLLFreq()/div;
}


int32_t AUDIOPLL_InitPLL(cmn_divider_t div, uint32_t nf){
    uint32_t sys_freq, rdata, i = 0;
    clock_src_name_t src;
    cmn_divider_t div_g;

    // Parameter check
    if (div.divider_N > AUDIOPLL_XTAL_REFN_DIV_MASK || div.divider_N == 0 || \
            div.divider_M > AUDIOPLL_XTAL_REFM_DIV_MASK || div.divider_M == 0){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // AudPLL check
    if (AUDPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N == div.divider_N && \
            AUDPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N == (div.divider_M - 1) && \
            AUDPLL_CTRL_BASE->REG_0X020.bit.FBSDM_N == nf && \
            CORE_CTRL_BASE->REG_0X008.bit.AUDPLL_LOCK == 0x1){
        return CSK_DRIVER_OK;
    }

    // 检查当前系统是否使用AUD PLL，避免系统跑飞
    if (CRM_GetIpConfig(&src, &div_g, CRM_Sysroot) == CSK_DRIVER_OK)
    {
        if (src == CRM_IpSrcAudPllCore){
            return CSK_DRIVER_ERROR_BUSY;
        }
    }

    // 获得当前系统时钟
#ifdef PLATFORM_AP
    sys_freq = CRM_GetApCoreSysClkFreq();

#elif defined PLATFORM_CP
    sys_freq = CRM_GetCpCoreSysClkFreq();
#else
#error "Undefine PLATFORM_AP or PLATFORM_CP"
#endif

    uint32_t wait_time = sys_freq / WAITTIME_CLOCK10_DIV;

    // Reset
    ANA_AON_BASE->REG_0X000.all = 0x0;
    AUDPLL_CTRL_BASE->REG_0X000.all = 0x0;
    CORE_CTRL_BASE->REG_0X000.all = 0x4;

    do{
        i++;
        rdata = AUDPLL_CTRL_BASE->REG_0X000.all;
    } while(rdata == 0 && i < wait_time);

    wait_time = sys_freq / WAITTIME_CLOCK100_DIV;
    AUDPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N = div.divider_N;
    AUDPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N = (div.divider_M - 1);
    AUDPLL_CTRL_BASE->REG_0X020.bit.FBSDM_N = nf;

    // Reset
    AUDPLL_CTRL_BASE->REG_0X004.all = 0x0;

    // Wait lock
    do{
        i++;
        rdata = CORE_CTRL_BASE->REG_0X008.bit.AUDPLL_LOCK;
    } while(rdata == 0 && i < wait_time);

    // 如果lock超时，手动配置为clock
    if (i >= wait_time){
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIS_AUD_PLL_UNLOCK = 0x1;
    }

    return CSK_DRIVER_OK;
}

#undef WAITTIME_CLOCK100_DIV
#undef WAITTIME_CLOCK10_DIV

// XTAL * (N + NF/2^24) / M
static uint32_t AUDIOPLL_GetPLLFreq(void){
    double nf = ((double)AUDPLL_CTRL_BASE->REG_0X020.bit.FBSDM_N) / (1 << 24);
    double n_nf = (double)AUDPLL_CTRL_BASE->REG_0X014.bit.FBDIV_N + nf;
    return (uint32_t)(BOARD_H_XTAL_SRC_FREQ * n_nf / (AUDPLL_CTRL_BASE->REG_0X014.bit.REFDIV_N + 1));
}


int32_t AUDIOPLL_InitPsram(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t AUDIOPLL_GetPsramFreq(void){
    return AUDIOPLL_GetPLLFreq() / (AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_PSRAMCLK_SEL * 2);
}

int32_t AUDIOPLL_InitFlash(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 30) && div % 2 == 0){
        div_reg = div/2;
        AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t AUDIOPLL_GetFlashFreq(void){
    return AUDIOPLL_GetPLLFreq() / (AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL * 2);
}

int32_t AUDIOPLL_InitCore(audiopll_core_sel_t div){
    AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_CORECLK_SEL = div;
    return CSK_DRIVER_OK;
}

static uint32_t AUDIOPLL_GetCoreFreq(void){
    uint8_t div = AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_CORECLK_SEL + 3;
    return AUDIOPLL_GetPLLFreq() / div;
}

int32_t AUDIOPLL_InitAudio(uint32_t div){
    uint32_t div_reg;
    if ((div >= 2 || div <= 62) && div % 2 == 0){
        div_reg = div/2;
        AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_FLASHCLK_SEL = div_reg;
        return CSK_DRIVER_OK;
    }

    return CSK_DRIVER_ERROR_PARAMETER;
}

static uint32_t AUDIOPLL_GetAudioFreq(void){
    return AUDIOPLL_GetPLLFreq() / (AUDPLL_CTRL_BASE->REG_0X018.bit.POSTDIV_AUDCLK_SEL * 2);
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
    case CRM_IpSrcSysPllUsb:
        return SYSPLL_GetUsbFreq();
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
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }
#endif

}



int32_t CRM_SetSysclkDiv(cmn_ip_name_t ip, cmn_divider_t div){
    switch (ip){
    case CRM_CpCoreClk:
    case CRM_NpuCoreClk:
    case CRM_CpHclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    case CRM_CpPclk:
    {
        uint8_t div_n = div.divider_N & CRM_DN_CP_PCLK_MASK;
        uint8_t div_m = div.divider_M & CRM_DM_CP_PCLK_MASK;

        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M = div_m;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_N = div_n;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_LD = 0x1;
        break;
    }
    case CRM_ApRootClk:
    {
        uint8_t div_n = div.divider_N & CRM_DN_AP_ROOT_MASK;
        uint8_t div_m = div.divider_M & CRM_DM_AP_ROOT_MASK;

        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_N = div_n;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_LD = 0x1;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_SysclkEnable(cmn_ip_name_t ip){
    switch (ip){
    case CRM_CpCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = CRM_CLK_ENABLE;
        break;
    case CRM_NpuCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = CRM_CLK_ENABLE;
        break;
    case CRM_CpPclk:
    case CRM_ApRootClk:
    case CRM_CpHclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_SysclkDisable(cmn_ip_name_t ip){
    switch (ip){
    case CRM_CpCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = CRM_CLK_GATED;
        break;
    case CRM_NpuCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = CRM_CLK_GATED;
        break;
    case CRM_CpPclk:
    case CRM_ApRootClk:
    case CRM_CpHclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_SysclkIsEnabled(cmn_ip_name_t ip){
    switch (ip){
    case CRM_CpCoreClk:
        return SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK;
        break;
    case CRM_NpuCoreClk:
        return SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK;
        break;
    case CRM_CpPclk:
    case CRM_ApRootClk:
    case CRM_CpHclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return true;
}

int32_t CRM_GetSysclkConfig(cmn_ip_name_t ip, cmn_divider_t* div){
    if(div == NULL){return CSK_DRIVER_ERROR_PARAMETER;}

    switch (ip){
    case CRM_CpCoreClk:
    case CRM_NpuCoreClk:
    case CRM_CpHclk:
        div->divider_N = 0x1;
        div->divider_M = 0x1;
        break;
    case CRM_CpPclk:
    {
        div->divider_N = SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CP_PCLK_M;
        break;
    }
    case CRM_ApRootClk:
    {
        div->divider_N = SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_AP_ROOT_M;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}


int32_t CRM_SetApSysclkDiv(cmn_ip_name_t ip, cmn_divider_t div){
    switch (ip){
    case CRM_ApCoreClk:
    case CRM_ApHclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    case CRM_ApPclk:
    {
        uint8_t div_n = div.divider_N & CRM_DN_AP_PCLK_MASK;
        uint8_t div_m = div.divider_M & CRM_DM_AP_PCLK_MASK;

        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_LD = 0x1;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_ApSysclkEnable(cmn_ip_name_t ip){
    switch (ip){
    case CRM_ApCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK = CRM_CLK_ENABLE;
        break;
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_ApSysclkDisable(cmn_ip_name_t ip){
    switch (ip){
    case CRM_ApCoreClk:
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK = CRM_CLK_GATED;
        break;
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_ApSysclkIsEnabled(cmn_ip_name_t ip){
    switch (ip){
    case CRM_ApCoreClk:
        return SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_AP_CORE_CLK;
    case CRM_ApHclk:
    case CRM_ApPclk:
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return true;
}

int32_t CRM_GetApSysclkConfig(cmn_ip_name_t ip, cmn_divider_t* div){
    if(div == NULL){return CSK_DRIVER_ERROR_PARAMETER;}

    switch (ip){
    case CRM_ApCoreClk:
    case CRM_ApHclk:
        div->divider_N = 0x1;
        div->divider_M = 0x1;
        break;
    case CRM_ApPclk:
    {
        div->divider_N = SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_AP_PERI_PCLK_M;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}


int32_t CRM_SetApPclkDiv(cmn_ip_name_t ip, cmn_divider_t div){
    switch (ip){
    case CRM_ApPeriPclk:
    case CRM_ApCfgPclk:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    case CRM_ApCmnPclk:
    {
        uint8_t div_m = div.divider_M & CRM_DM_AP_CMN_CFG_MASK;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_LD = 0x1;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_GetApPclkConfig(cmn_ip_name_t ip, cmn_divider_t* div){
    if(div == NULL){return CSK_DRIVER_ERROR_PARAMETER;}

    switch (ip){
    case CRM_ApPeriPclk:
    case CRM_ApCfgPclk:
        div->divider_M = 0x1;
        div->divider_N = 0x1;
        break;
    case CRM_ApCmnPclk:
    {
        div->divider_M = SYSTEM_CTRL_BASE->REG_BUS_CLK_CFG.bit.DIV_CMN_CFG_PCLK_M;
        div->divider_N = 0x1;
        break;
    }
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    return CSK_DRIVER_OK;
}

uint32_t CRM_GetApHclkClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    uint32_t src_freq;
    cmn_divider_t div;
    src_freq = CRM_GetIpFreq(CRM_Sysroot);
    CRM_GetSysclkConfig(CRM_ApRootClk, &div);
    return (uint32_t) src_freq*div.divider_N/div.divider_M;
#endif
}


uint32_t CRM_GetApCoreSysClkFreq(void){

#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (CRM_ApSysclkIsEnabled(CRM_ApCoreClk)){
        return CRM_GetApHclkClkFreq();
    }

    return 0;
#endif
}

uint32_t CRM_GetApPclkPeriClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    uint32_t src_freq;
    cmn_divider_t div;
    src_freq = CRM_GetApHclkClkFreq();
    CRM_GetApSysclkConfig(CRM_ApPclk, &div);
    return (uint32_t)src_freq*div.divider_N/div.divider_M;
#endif
}

uint32_t CRM_GetApPclkCfgClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    return CRM_GetApPclkPeriClkFreq();
#endif
}

uint32_t CRM_GetApPclkCmnClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    uint32_t src_freq;
    cmn_divider_t div;
    src_freq = CRM_GetApPclkPeriClkFreq();
    CRM_GetApPclkConfig(CRM_ApCmnPclk, &div);
    return (uint32_t)src_freq/div.divider_M;
#endif
}

uint32_t CRM_GetCpHclkClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    return CRM_GetIpFreq(CRM_Sysroot);
#endif
}

uint32_t CRM_GetCpCoreSysClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (CRM_SysclkIsEnabled(CRM_CpCoreClk)){
        return CRM_GetCpHclkClkFreq();
    }

    return 0;
#endif
}

uint32_t CRM_GetCpPclkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    uint32_t src_freq;
    cmn_divider_t div;
    src_freq = CRM_GetCpHclkClkFreq();
    CRM_GetSysclkConfig(CRM_CpPclk, &div);
    return src_freq*div.divider_N/div.divider_M;
#endif
}

uint32_t CRM_GetNpuCoreSysClkFreq(void){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    if (CRM_SysclkIsEnabled(CRM_NpuCoreClk)){
        return CRM_GetCpHclkClkFreq();
    }

    return 0;
#endif
}

uint32_t CRM_GetIpFreq(cmn_ip_name_t ip){
#if (IC_BOARD == 0)
    return IC_BOARD_FPGA_FIX_FREQ;
#else
    switch (ip){
    case CRM_GpADC:
    case CRM_Usbc:
    case CRM_Uart0:
    case CRM_Uart1:
    case CRM_Uart2:
    case CRM_Uart3:
    case CRM_Sdio:
    case CRM_Spi0:
    case CRM_Spi1:
    case CRM_GpioA:
    case CRM_GpioB:
    case CRM_Gpts:
    case CRM_Gptt0:
    case CRM_Psram:
    case CRM_Flash:

    case CRM_AudioAdc:
    case CRM_AudioDac:

    case CRM_Ir:

    case CRM_ApWdt:
    case CRM_CpWdt:

    case CRM_Sysroot:
    {
        clock_src_name_t src;
        cmn_divider_t div;
        CRM_GetIpConfig(&src, &div, ip);

        if (CRM_IpIsEnabled(ip)){
            return CRM_GetSrcFreq(src) * div.divider_N / div.divider_M;
        }
        // disabled
        return 0;
    }
    case CRM_ApDmac:
    case CRM_ApCrypto:
        return CRM_GetIpFreq(CRM_ApHclk);
    case CRM_ApI2c0:
    case CRM_ApI2c1:
    case CRM_ApTrng:
    case CRM_ApAbb:
        return CRM_GetIpFreq(CRM_ApCfgPclk);

    case CRM_CpApc:
    case CRM_CpDmac:
    case CRM_FccRamc:
        return CRM_GetIpFreq(CRM_CpHclk);

    case CRM_CpCoreClk:
        return CRM_GetCpCoreSysClkFreq();
    case CRM_NpuCoreClk:
        return CRM_GetNpuCoreSysClkFreq();
    case CRM_CpHclk:
        return CRM_GetCpHclkClkFreq();
    case CRM_CpPclk:
        return CRM_GetCpPclkFreq();
    case CRM_ApRootClk:
    case CRM_ApCoreClk:
        return CRM_GetApCoreSysClkFreq();
    case CRM_ApHclk:
        return CRM_GetApHclkClkFreq();
    case CRM_ApPclk:
    case CRM_ApPeriPclk:
        return CRM_GetApPclkPeriClkFreq();
    case CRM_ApCfgPclk:
        return CRM_GetApPclkCfgClkFreq();
    case CRM_ApCmnPclk:
        return CRM_GetApPclkCmnClkFreq();

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }
#endif
}


/*!
 * @brief Set the clock source for specific Region.
 *
 * Set the clock source for specific Region, region can't choose all
 * clock source, depending on the region mask
 *
 * @param src Clock source to set.
 * @param ip Which region to check, see \ref cmn_ip_name_t.
 * @param div Clock divider, see \ref cmn_divider_t.
 * @param gate Clock gate, True enable clock, False disable clock.
 */
int32_t CRM_SetIpSrc(cmn_ip_name_t ip, clock_src_name_t src){
    switch (ip){

    case CRM_Uart0:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Uart1:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART1_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART1_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Uart2:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART2_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART2_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Uart3:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Sdio:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Spi0:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Spi1:
        if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcSysPllUsb){
            SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK = CRM_SRC_COMMUNICATE_SEL_SYSPLL;
        } else if ((src & CRM_SRC_COMMUNICATE_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK = CRM_SRC_COMMUNICATE_SEL_24XTAL;
        }
        break;

    case CRM_Psram:
        if ((src & CRM_SRC_PSRAM_MASK) == CRM_IpSrcSysPllPsram){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = CRM_SRC_PSRAM_SEL_SYSPLL;
        } else if ((src & CRM_SRC_PSRAM_MASK) == CRM_IpSrcAudPllPsram){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = CRM_SRC_PSRAM_SEL_AUDPLL;
        } else if ((src & CRM_SRC_PSRAM_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK = CRM_SRC_PSRAM_SEL_24XTAL;
        }
        break;

    case CRM_Flash:
        if ((src & CRM_SRC_FLASH_MASK) == CRM_IpSrcSysPllFlash){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = CRM_SRC_FLASH_SEL_SYSPLL;
        } else if ((src & CRM_SRC_FLASH_MASK) == CRM_IpSrcAudPllFlash){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = CRM_SRC_FLASH_SEL_AUDPLL;
        } else if ((src & CRM_SRC_FLASH_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK = CRM_SRC_FLASH_SEL_24XTAL;
        }
        break;

    case CRM_Sysroot:
        if ((src & CRM_SRC_SYS_ROOT_MASK) == CRM_IpSrcSysPllCore){
            SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = CRM_SRC_SYS_ROOT_SEL_SYSPLL;
        } else if ((src & CRM_SRC_SYS_ROOT_MASK) == CRM_IpSrcAudPllCore){
            SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = CRM_SRC_SYS_ROOT_SEL_AUDPLL;
        } else if ((src & CRM_SRC_SYS_ROOT_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK = CRM_SRC_SYS_ROOT_SEL_24XTAL;
        }
        break;

    case CRM_Ir:
        if ((src & CRM_SRC_IR_MASK) == CRM_IpSrcCmn32K){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = CRM_SRC_ADC_SEL_24XTAL;
        } else if ((src & CRM_SRC_IR_MASK) == CRM_IpSrcAon32K){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK = CRM_SRC_IR_SEL_LP32K;
        }
        break;

    case CRM_ApWdt:
    case CRM_CpWdt:
        if ((src & CRM_SRC_TIMER_MASK) == CRM_IpSrcCmn32K){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_SLOW_CLOCK_SOURCE = CRM_SRC_TIMER_SEL_CMN32K;
        } else if ((src & CRM_SRC_TIMER_MASK) == CRM_IpSrcAon32K){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_SLOW_CLOCK_SOURCE = CRM_SRC_TIMER_SEL_LP32K;
        }
        break;

    case CRM_AudioAdc:
        if ((src & CRM_SRC_ADC_MASK) == CRM_IpSrcAudPllAudio){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = CRM_SRC_ADC_SEL_AUDPLL;
        } else if ((src & CRM_SRC_ADC_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = CRM_SRC_ADC_SEL_24XTAL;
        }
        break;

    case CRM_AudioDac:
        if ((src & CRM_SRC_DAC_MASK) == CRM_IpSrcAudPllAudio){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = CRM_SRC_DAC_SEL_AUDPLL;
        } else if ((src & CRM_SRC_DAC_MASK) == CRM_IpSrcXtal){
            SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = CRM_SRC_DAC_SEL_24XTAL;
        }
        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

static int32_t CRM_SetCommunicateIpDiv(cmn_ip_name_t ip, cmn_divider_t div){
    uint8_t div_n = div.divider_N & CRM_DN_COMMUNICATE_MASK;
    uint8_t div_m = div.divider_M & CRM_DM_COMMUNICATE_MASK;

    switch (ip){
    case CRM_Uart0:
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_LD = 0x1;
        break;

    case CRM_Uart1:
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_LD = 0x1;
        break;

    case CRM_Uart2:
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_LD = 0x1;
        break;

    case CRM_Uart3:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_LD = 0x1;
        break;

    case CRM_Sdio:
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_LD = 0x1;
        break;

    case CRM_Spi0:
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_LD = 0x1;
        break;

    case CRM_Spi1:
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_N = div_n;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_LD = 0x1;
        break;

    default:
        return CSK_DRIVER_ERROR;
    }

    return CSK_DRIVER_OK;
}

static int32_t CRM_SetIoGptIpDiv(cmn_ip_name_t ip, cmn_divider_t div){
    uint8_t div_m = div.divider_M & CRM_DM_IO_GPT_MASK;
    // TODO For the requirement of <= 1/2 ap_peri_pclk:
    /*
     * if 24M / div_m > 1/2 ap pclk:
     *     return error;
     * */
    if (CRM_CLK_XTAL_24MHZ() / div_m > CRM_GetApPclkPeriClkFreq()){
        return CSK_DRIVER_ERROR;
    }

    switch (ip){
    case CRM_GpioA:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK_LD = 0x1;
        break;

    case CRM_GpioB:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK_LD = 0x1;
        break;

    case CRM_Gpts:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS_LD = 0x1;
        break;

    case CRM_Gptt0:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0 = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0_LD = 0x1;
        break;

    default:
        return CSK_DRIVER_ERROR;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_SetIpDiv(cmn_ip_name_t ip, cmn_divider_t div){
    switch (ip){
//    case CRM_GpADC:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CRM_Uart0:
    case CRM_Uart1:
    case CRM_Uart2:
    case CRM_Uart3:
    case CRM_Sdio:
    case CRM_Spi0:
    case CRM_Spi1:
        return CRM_SetCommunicateIpDiv(ip, div);

    case CRM_GpioA:
    case CRM_GpioB:
    case CRM_Gpts:
    case CRM_Gptt0:
        return CRM_SetIoGptIpDiv(ip, div);

    case CRM_Psram:
    {
        uint8_t div_m = div.divider_M & CRM_DM_PSRAM_MASK;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK = div_m - 1;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK_LD = 0x1;
        return CSK_DRIVER_OK;
    }

    case CRM_Flash:
    {
        uint8_t div_m = div.divider_M & CRM_DM_FLASH_MASK;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK = div_m - 1;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK_LD = 0x1;
        return CSK_DRIVER_OK;
    }

//    case CRM_Usbc:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CRM_Sysroot:
    {
        uint8_t div_n = div.divider_N & CRM_DN_SYS_ROOT_MASK;
        uint8_t div_m = div.divider_M & CRM_DM_SYS_ROOT_MASK;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_LD = 0x0;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_N = div_n;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M = div_m;
        MemBarrier;
        SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_LD = 0x1;
        return CSK_DRIVER_OK;
    }

    case CRM_NpuCoreClk:
    case CRM_CpCoreClk:
    case CRM_CpHclk:
    case CRM_CpPclk:
    case CRM_ApRootClk:
        return CRM_SetSysclkDiv(ip, div);

    case CRM_ApCoreClk:
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CRM_SetApSysclkDiv(ip, div);

    case CRM_ApPeriPclk:
    case CRM_ApCfgPclk:
    case CRM_ApCmnPclk:
        return CRM_SetApPclkDiv(ip, div);

//    case CRM_Ir:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;
//
//    case CRM_ApTimer:
//    case CRM_CpTimer:
//    case CRM_ApWdt:
//    case CRM_CpWdt:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;
//
//    case CRM_AudioAdc:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;
//
//    case CRM_AudioDac:
//        return CSK_DRIVER_ERROR_UNSUPPORTED;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_IpClkEnable(cmn_ip_name_t ip){
    switch (ip){
    case CRM_Psram:
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = CRM_CLK_ENABLE;
        break;
    case CRM_Flash:
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK = CRM_CLK_ENABLE;
        break;

    case CRM_AudioAdc:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = CRM_CLK_ENABLE;
        break;
    case CRM_AudioDac:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = CRM_CLK_ENABLE;
        break;

    case CRM_NpuCoreClk:
    case CRM_CpCoreClk:
    case CRM_CpPclk:
    case CRM_CpHclk:
    case CRM_ApRootClk:
        return CRM_SysclkEnable(ip);

    case CRM_ApCoreClk:
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CRM_ApSysclkEnable(ip);

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_IpClkDisable(cmn_ip_name_t ip){
    switch (ip){

    case CRM_Psram:
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = CRM_CLK_GATED;
        break;
    case CRM_Flash:
        SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK = CRM_CLK_GATED;
        break;

    case CRM_AudioAdc:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = CRM_CLK_GATED;
        break;
    case CRM_AudioDac:
        SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = CRM_CLK_GATED;
        break;

    case CRM_NpuCoreClk:
    case CRM_CpCoreClk:
    case CRM_CpPclk:
    case CRM_CpHclk:
    case CRM_ApRootClk:
        return CRM_SysclkDisable(ip);

    case CRM_ApCoreClk:
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CRM_ApSysclkDisable(ip);

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t CRM_IpIsEnabled(cmn_ip_name_t ip){
    switch (ip){
    case CRM_Psram:
        return SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK;
    case CRM_Flash:
        return SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_FLASH_CLK;

    case CRM_AudioAdc:
        return SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC;
    case CRM_AudioDac:
        return SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC;

    case CRM_NpuCoreClk:
    case CRM_CpCoreClk:
    case CRM_CpPclk:
    case CRM_CpHclk:
    case CRM_ApRootClk:
        return CRM_SysclkIsEnabled(ip);

    case CRM_ApCoreClk:
    case CRM_ApHclk:
    case CRM_ApPclk:
        return CRM_ApSysclkIsEnabled(ip);

    default:
        return true;
    }

    return CSK_DRIVER_OK;
}

static int32_t CRM_GetCommunicateConfig(clock_src_name_t* src, cmn_divider_t* div, cmn_ip_name_t ip){
    uint8_t sel;
    switch (ip){
    case CRM_Uart0:
        sel = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_UART0_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_UART0_CLK_M;
        break;
    case CRM_Uart1:
        sel = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART1_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART1_CLK_M;
        break;
    case CRM_Uart2:
        sel = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.SEL_UART2_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_UART_CLK_CFG.bit.DIV_UART2_CLK_M;
        break;
    case CRM_Uart3:
        sel = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.SEL_UART3_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_UART3_CLK_M;
        break;
    case CRM_Sdio:
        sel = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.SEL_SDIO_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_UART_SDIO_CLK.bit.DIV_SDIO_CLK_M;
        break;
    case CRM_Spi0:
        sel = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI0_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI0_CLK_M;
        break;
    case CRM_Spi1:
        sel = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.SEL_SPI1_CLK;
        div->divider_N = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_N;
        div->divider_M = SYSTEM_CTRL_BASE->REG_SPI_CLK_CFG.bit.DIV_SPI1_CLK_M;
        break;
    default:
        return CSK_DRIVER_ERROR;
    }

    switch (div->divider_N){
    case 0:
        div->divider_N = 0x1;
        break;
    case 1:
        break;
    default:
//        div->divider_N += 1;
        break;
    }

    switch (div->divider_M){
    case 0:
        div->divider_M = 0x1;
        break;
    case 1:
        break;
    default:
//        div->divider_M += 1;
        break;
    }

    if (sel == CRM_SRC_COMMUNICATE_SEL_24XTAL){
        *src = CRM_IpSrcXtal;
    } else if (sel == CRM_SRC_COMMUNICATE_SEL_SYSPLL){
        *src = CRM_IpSrcSysPllUsb;
    }

    return CSK_DRIVER_OK;
}

static int32_t CRM_GetIoGptConfig(clock_src_name_t* src, cmn_divider_t* div, cmn_ip_name_t ip){
    switch (ip){

    // GPIO分配系数默认多除以2
    case CRM_GpioA:
        div->divider_M = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOA_EXTCLK;
        div->divider_M *= 2;
        break;
    case CRM_GpioB:
        div->divider_M = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPIOB_EXTCLK;
        div->divider_M *= 2;
        break;
    case CRM_Gpts:
        div->divider_M = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKS;
        break;
    case CRM_Gptt0:
        div->divider_M = SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG1.bit.DIV_GPT_CLKT0;
        break;
    default:
        return CSK_DRIVER_ERROR;
    }

    div->divider_N = 0x1;

    // 0 and 1 all equal to 1
    // other n+1
    switch (div->divider_M){
    case 0:
        div->divider_M = 0x1;
        break;
    case 1:
        break;
    default:
//        div->divider_M += 1;
        break;
    }

    *src = CRM_IpSrcXtal;
    return CSK_DRIVER_OK;
}

int32_t CRM_GetIpConfig(clock_src_name_t* src, cmn_divider_t* div, cmn_ip_name_t ip){
    clock_src_name_t src_t;
    cmn_divider_t div_t;
    int32_t ret = CSK_DRIVER_OK;

    switch (ip){
    case CRM_Uart0:
    case CRM_Uart1:
    case CRM_Uart2:
    case CRM_Uart3:
    case CRM_Sdio:
    case CRM_Spi0:
    case CRM_Spi1:
        ret = CRM_GetCommunicateConfig(&src_t, &div_t, ip);
        break;

    case CRM_GpioA:
    case CRM_GpioB:
    case CRM_Gpts:
    case CRM_Gptt0:
        ret = CRM_GetIoGptConfig(&src_t, &div_t, ip);
        break;

    case CRM_Psram:

        switch (SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_PSRAM_CLK){
        case CRM_SRC_PSRAM_SEL_24XTAL:
            src_t = CRM_IpSrcXtal;
            break;
        case CRM_SRC_PSRAM_SEL_SYSPLL:
            src_t = CRM_IpSrcSysPllPsram;
            break;
        case CRM_SRC_PSRAM_SEL_AUDPLL:
            src_t = CRM_IpSrcAudPllPsram;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }

        div_t.divider_M = SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_PSRAM_CLK + 1;
        div_t.divider_N = 0x1;
        break;

    case CRM_Flash:
        switch (SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.SEL_FLASH_CLK){
        case CRM_SRC_FLASH_SEL_24XTAL:
            src_t = CRM_IpSrcXtal;
            break;
        case CRM_SRC_FLASH_SEL_SYSPLL:
            src_t = CRM_IpSrcSysPllFlash;
            break;
        case CRM_SRC_FLASH_SEL_AUDPLL:
            src_t = CRM_IpSrcAudPllFlash;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }

        div_t.divider_M = SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.DIV_FLASH_CLK + 1;
        div_t.divider_N = 0x1;
        break;

    case CRM_Usbc:
        src_t = CRM_IpSrcSysPllUsb;
        div_t.divider_N = 0x1;
        div_t.divider_M = 0x1;
        break;

    case CRM_AudioAdc:
            switch (SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC)
            {
            case CRM_SRC_ADC_SEL_24XTAL:
                src_t = CRM_IpSrcXtal;
                break;
            case CRM_SRC_ADC_SEL_AUDPLL:
                src_t = CRM_IpSrcAudPllAudio;
                break;
            default:
                return CSK_DRIVER_ERROR;
            }

            div_t.divider_M = 0x1;
            div_t.divider_N = 0x1;

            break;

    case CRM_AudioDac:
        switch (SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC){
        case CRM_SRC_DAC_SEL_24XTAL:
            src_t = CRM_IpSrcXtal;
            break;
        case CRM_SRC_DAC_SEL_AUDPLL:
            src_t = CRM_IpSrcAudPllAudio;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }

        div_t.divider_M = 0x1;
        div_t.divider_N = 0x1;

        break;

    case CRM_Ir:
        switch (SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_IR_CLK){
        case CRM_SRC_IR_SEL_LP32K:
            src_t = CRM_IpSrcAon32K;
            break;
        case CRM_SRC_IR_SEL_CMN32K:
            src_t = CRM_IpSrcCmn32K;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }

        div_t.divider_M = 0x1;
        div_t.divider_N = 0x1;

        break;

    // 默认除以2
    case CRM_ApWdt:
    case CRM_CpWdt:
        switch (SYSTEM_CTRL_BASE->REG_MISC_CLK_CFG2.bit.SEL_SLOW_CLOCK_SOURCE){
        case CRM_SRC_TIMER_SEL_LP32K:
            src_t = CRM_IpSrcAon32K;
            break;
        case CRM_SRC_TIMER_SEL_CMN32K:
            src_t = CRM_IpSrcCmn32K;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }

        div_t.divider_M = 0x2;
        div_t.divider_N = 0x1;

        break;

    case CRM_Sysroot:
        switch (SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.SEL_SYS_ROOT_CLK){
        case CRM_SRC_SYS_ROOT_SEL_24XTAL:
            src_t = CRM_IpSrcXtal;
            break;
        case CRM_SRC_SYS_ROOT_SEL_SYSPLL:
            src_t = CRM_IpSrcSysPllCore;
            break;
        case CRM_SRC_SYS_ROOT_SEL_AUDPLL:
            src_t = CRM_IpSrcAudPllCore;
            break;
        default:
            return CSK_DRIVER_ERROR;
        }
        div_t.divider_N = SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_N;
        div_t.divider_M = SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.DIV_SYS_ROOT_M;

        switch (div_t.divider_N){
        case 0:
            div_t.divider_N = 0x1;
            break;
        case 1:
            break;
        default:
//            div_t.divider_N += 1;
            break;
        }

        switch (div_t.divider_M){
        case 0:
            div_t.divider_M = 0x1;
            break;
        case 1:
            break;
        default:
//            div_t.divider_M += 1;
            break;
        }

        break;

    case CRM_NpuCoreClk:
    case CRM_CpCoreClk:
    case CRM_CpHclk:
    case CRM_CpPclk:
    case CRM_ApRootClk:
        src_t = CRM_IpSrcInvalide;
        ret = CRM_GetSysclkConfig(ip, &div_t);
        break;

    case CRM_ApCoreClk:
    case CRM_ApHclk:
    case CRM_ApPclk:
        src_t = CRM_IpSrcInvalide;
        ret = CRM_GetApSysclkConfig(ip, &div_t);
        break;

    case CRM_ApPeriPclk:
    case CRM_ApCfgPclk:
    case CRM_ApCmnPclk:
        src_t = CRM_IpSrcInvalide;
        ret = CRM_GetApPclkConfig(ip, &div_t);
        break;

    default:
        src_t = CRM_IpSrcInvalide;
        div_t.divider_M = 0x1;
        div_t.divider_N = 0x1;
    }

    // 避免用户填入NULL
    if (src){
        *src = src_t;
    }

    if (div){
        *div = div_t;
    }

    return ret;
}

int32_t CRM_GetIpClkDiv(uint32_t refFreq, uint32_t desireFreq, cmn_divider_t* div, cmn_ip_name_t ip){
    if (div == NULL || refFreq == 0 || desireFreq == 0){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    uint32_t N_bound, M_bound;
    uint32_t n, m;
    float coe = (float)desireFreq / (float)refFreq;

    switch (ip){
    case CRM_Uart0:
    case CRM_Uart1:
    case CRM_Uart2:
    case CRM_Uart3:
    case CRM_Sdio:
    case CRM_Spi0:
    case CRM_Spi1:
        N_bound = CRM_DN_COMMUNICATE_MASK;
        M_bound = CRM_DM_COMMUNICATE_MASK;
        break;

    case CRM_GpioA:
    case CRM_GpioB:
    case CRM_Gpts:
    case CRM_Gptt0:
        N_bound = CRM_DN_IO_GPT_MASK;
        M_bound = CRM_DM_IO_GPT_MASK;
        break;

    case CRM_Psram:
        N_bound = CRM_DN_PSRAM_MASK;
        M_bound = CRM_DM_PSRAM_MASK;
        break;

    case CRM_Flash:
        N_bound = CRM_DN_FLASH_MASK;
        M_bound = CRM_DM_FLASH_MASK;
        break;

    case CRM_Sysroot:
        N_bound = CRM_DN_SYS_ROOT_MASK;
        M_bound = CRM_DM_SYS_ROOT_MASK;
        break;

    case CRM_CpPclk:
        N_bound = CRM_DN_CP_PCLK_MASK;
        M_bound = CRM_DM_CP_PCLK_MASK;
        break;

    case CRM_ApRootClk:
        N_bound = CRM_DN_AP_ROOT_MASK;
        M_bound = CRM_DM_AP_ROOT_MASK;
        break;

    case CRM_ApPclk:
        N_bound = CRM_DN_AP_PCLK_MASK;
        M_bound = CRM_DM_AP_PCLK_MASK;
        break;

    case CRM_ApCmnPclk:
        N_bound = CRM_DN_AP_CMN_CFG_MASK;
        M_bound = CRM_DM_AP_CMN_CFG_MASK;
        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    if (coe == 1.0){
        div->divider_N = 1;
        div->divider_M = 1;
        return CSK_DRIVER_OK;
    }

    // 计算分频系数
    for (m = 2; m <= M_bound; m++){
        for (n = 1; n <= N_bound && n < m; n++){
            if (coe == (float)((float)n/(float)m)){
                div->divider_N = n;
                div->divider_M = m;
                return CSK_DRIVER_OK;
            }
        }
    }

    return CSK_DRIVER_ERROR;
}
