/*
 * Driver_AON_WDT.h
 *
 *  Created on: Mar 30, 2022
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_DRIVER_AON_WDT_H_
#define INCLUDE_DRIVER_DRIVER_AON_WDT_H_

#include "venus_ap.h"
#include "Driver_Common.h"

#define HAL_AON_WDT_TIME_Pos                    0
#define HAL_AON_WDT_TIME_CFG                   (0x1UL << HAL_AON_WDT_TIME_Pos)

#define HAL_AON_WDT_INTERRUPT_Pos               1
#define HAL_AON_WDT_INTERRUPT_EN               (0x1UL << HAL_AON_WDT_INTERRUPT_Pos)

#define HAL_AON_WDT_RESET_DOMAIN_Pos            2
#define HAL_AON_WDT_RESET_PMU_CTRL             (0x1UL << HAL_AON_WDT_RESET_DOMAIN_Pos)

enum _pmu_ctrl_sel{
    reset_pmu_ctrl = 0x0,
    reset_dbb_domain = 0x1,
};

typedef void (*HAL_AON_WDT_SignalEvent_t)(void* workspace);

int32_t AON_WDT_Initialize(void* res, HAL_AON_WDT_SignalEvent_t callback, void* workspace);

int32_t AON_WDT_Uninitialize(void* res);

int32_t AON_WDT_PowerControl(void* res, CSK_POWER_STATE state);

int32_t AON_WDT_Control(void* res, uint32_t control, uint32_t arg);

int32_t AON_WDT_Enable(void* res);

int32_t AON_WDT_Refresh(void* res);

int32_t AON_WDT_Disable(void* res);

uint32_t AON_WDT_GetMaxValue(void);

void* AON_WDT(void);

void AON_TIMER_WDT_SHARE_Handler(void);

#endif /* INCLUDE_DRIVER_DRIVER_AON_WDT_H_ */
