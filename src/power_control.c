/*
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "reg/ana_aon_regfile_reg_venus.h"
#include "reg/cmn_sysctrl_reg_venus.h"
#include "venus_ap.h"
#include "ClockManager.h"


#define SYSCTRL_RESET_CP (0xCAFE000B)
#define SYSCTRL_RELEASE_CP (0xCAFE100B)
#define SYSCTRL_RESET_NPU (0xCAFE000C)



#define MEGA_VALUE 1000000ul //  1mhz

typedef struct power_ctrl_t {
	uint8_t is_lowpower;
	uint8_t psram_ctrl;
	// void *usr_data;
	// void (*usr_cb)(void *data);
} power_ctrl_t;

typedef struct ap_div_t {
	uint8_t div_n;
	uint8_t div_m;
} ap_div_t;

power_ctrl_t g_power_ctrl = {
	.is_lowpower = 0,
	.psram_ctrl = 0,
};

static int get_ap_freq_div(uint32_t mfreq, ap_div_t *ap_div)
{
	uint32_t freq;
	uint32_t root_freq = CRM_GetRootFreq();

	freq = mfreq * MEGA_VALUE;
	// printf("root_freq [%d] freq [%d]\n", root_freq, freq);

	/* freq = root_freq*div_n/div_m, (0<div_n<32, 1<div_m<33) */
	if(HAL_CRM_CalcAprootClkDiv(root_freq, freq, &(ap_div->div_n), &(ap_div->div_n)) != CSK_DRIVER_OK)
	{
		return 1;
	}
	return 0;
}

static int set_ap_freq(ap_div_t *ap_div)
{
	HAL_CRM_SetAprootClkDiv(ap_div->div_n, ap_div->div_m);
	return 0;
}


static void enable_cp_npu(void)
{
	// enable CP & NPU clock
	IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = 0x1;
	IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = 0x1;

	// release CP & NPU
	IP_SYSCTRL->REG_SW_RESET_CP.all = SYSCTRL_RELEASE_CP;

	if (IP_SYSCTRL->REG_SW_RESET_CP.all) {
		printf("error,cp reset should not be 0, in this momment\n");
	} else {
		// printf("reset cp\n");
		// reset cp
		IP_SYSCTRL->REG_SW_RESET_CP.all = SYSCTRL_RESET_CP;

		// release CP
		IP_SYSCTRL->REG_SW_RESET_CP.all = SYSCTRL_RELEASE_CP;
	}

	// release NPU
	if (IP_SYSCTRL->REG_SW_RESET_NPU.all) { // = 1 NPU reset
		IP_SYSCTRL->REG_SW_RESET_NPU.all = SYSCTRL_RESET_NPU;
	}

	// run HIFI4
	IP_SYSCTRL->REG_CP_CTRL1.bit.CP_HIFI4_RUNSTALL = 0x0;
}

static void disable_cp_npu(void)
{
	// stall HIFI4
	IP_SYSCTRL->REG_CP_CTRL1.bit.CP_HIFI4_RUNSTALL = 0x1;

	//  clear cp npu clock
	IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = 0x0;
	IP_SYSCTRL->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = 0x0;
}


static void enable_codec_driver(void)
{
	__HAL_CRM_ADC_CLK_ENABLE();
	__HAL_CRM_DAC_CLK_ENABLE();
	__HAL_CRM_APC_CLK_ENABLE();
	IP_ANA->REG_0X038.bit.LDO25_EN = 0x1;
}
static void disable_codec_driver(void)
{
	__HAL_CRM_ADC_CLK_DISABLE();
	__HAL_CRM_DAC_CLK_DISABLE();
	__HAL_CRM_APC_CLK_DISABLE();
	IP_ANA->REG_0X038.bit.LDO25_EN = 0x0;
}

#if 0
#include "Driver_PWR.h"
#include "AON_IOMuxManager.h"
#include "IOMuxManager.h"

static void PWR_Wakeup_Event(void* param){

    printf("PWR wake up event generate");
}

void PWR_LightSleep_PBWakeup(void)
{

    //initialize
    HAL_PWR_Initialize(PWR());
    HAL_PWR_PowerControl(PWR(), CSK_POWER_FULL);
    HAL_PWR_Control(PWR(), CSK_PWR_WAKE_SELECT_GPIOB_2 | CSK_PWR_WAKE_GPIOB_2_POLARITY_HIGH);

    AON_IOMuxManager_PinConfigure(CSK_IOMUX_PAD_B, 2, CSK_IOMUX_FUNC_ALTER2);

    HAL_PWR_RegisterCallback(PWR(), PWR_Wakeup_Event);
    HAL_PWR_InterruptEnable(PWR());

    HAL_PWR_EnterLightSleepMode(PWR(), PWR_LIGHTSLEEPENTRY_WFE);

    printf("Wrong!!!! CPU should not get here!!");

    while(1);
}
#endif

/**
  \brief   enter low power mode
  \details when in low power
           cp & npu will stop,  codec driver will stop
           ap will run by a low frequence.
		   psram can run or stop by params2
  \param[in]   freq    ap frequence in low frequence
  \param[in]   psram_ctrl   0: not ctrl psram, 1: ctrl psram,so psram will be stop in low power
*/

int32_t enter_lowpower_mode(uint32_t freq, uint8_t psram_ctrl)
{
	ap_div_t ap_div;
	if (g_power_ctrl.is_lowpower) {
		printf("already in low power\n");
		return -1;
	}

	if(BOARD_BOOTCLOCKRUN_ROOT_CLK <= freq*MEGA_VALUE){
		printf("freq[%d] is too large\n", freq);
		return -1;		
	}

	if(get_ap_freq_div(freq, &ap_div)){
		printf("freq[%d] is illegal\n", freq);
		return 1;
	}

	disable_cp_npu();

	disable_codec_driver();

	g_power_ctrl.psram_ctrl = psram_ctrl;
	if (g_power_ctrl.psram_ctrl) {
		__HAL_CRM_PSRAM_CLK_DISABLE();
	}

	if(set_ap_freq(&ap_div)){
		return 1;
	}

	g_power_ctrl.is_lowpower = 1;

	return 0;
}

/**
  \brief   set low power mode
  \details in low power
           cp & npu will stop,  codec driver will stop
           ap will run by a low frequence.
		   psram can run or stop by params2
  \param[in]   freq    ap frequence in low frequence
  \param[in]   psram_ctrl   0: not ctrl psram, 1: ctrl psram,so psram will be stop in low power
*/
int32_t exit_lowpower_mode(void)
{
	ap_div_t ap_div;
	if (!g_power_ctrl.is_lowpower) {
		printf("already not in low power\n");
		return -1;
	}

	if(get_ap_freq_div(BOARD_BOOTCLOCKRUN_ROOT_CLK/MEGA_VALUE, &ap_div)){
		printf(" sys freq error  \n");
		return 1;
	}
	
	if(set_ap_freq(&ap_div)){
		return 1;
	}

	if (g_power_ctrl.psram_ctrl) {
		__HAL_CRM_PSRAM_CLK_ENABLE();
	}

	enable_codec_driver();

	enable_cp_npu();

	g_power_ctrl.is_lowpower = 0;

	return 0;
}
