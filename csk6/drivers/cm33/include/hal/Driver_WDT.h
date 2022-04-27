/*
 * Project:      WDT (watch dog timer)
 *               Driver definitions
 */

#ifndef __DRIVER_WDT_H
#define __DRIVER_WDT_H

#include "aon_pmuctrl_reg_venus.h"
#include "Driver_Common.h"
#include "venus_ap.h"

#define CSK_WDT_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

/****** WDT Control Codes: clock source: 2 bits *****/
#define CSK_WDT_CLKSRC_Pos              0
#define CSK_WDT_CLKSRC_Msk              (0x03UL << CSK_WDT_CLKSRC_Pos)

#define CSK_WDT_CLKSRC_APB              (0x00UL << CSK_WDT_CLKSRC_Pos)
#define CSK_WDT_CLKSRC_EXT              (0x01UL << CSK_WDT_CLKSRC_Pos)

/*----- WDT Control Codes: interrupt interval time: 4 bits -----*/
/*----- actual time = clock_period * 2^(CSK_WDT_INTTIME_X) -----*/

#define CSK_WDT_INTTIME_Pos             4
#define CSK_WDT_INTTIME_Msk             (0x0FUL << CSK_WDT_INTTIME_Pos)

#define CSK_WDT_INTTIME_6               (0UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^6
#define CSK_WDT_INTTIME_8               (1UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^8
#define CSK_WDT_INTTIME_10              (2UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^10
#define CSK_WDT_INTTIME_11              (3UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^11
#define CSK_WDT_INTTIME_12              (4UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^12
#define CSK_WDT_INTTIME_13              (5UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^13
#define CSK_WDT_INTTIME_14              (6UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^14
#define CSK_WDT_INTTIME_15              (7UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^15
#define CSK_WDT_INTTIME_17              (8UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^17
#define CSK_WDT_INTTIME_19              (9UL << CSK_WDT_INTTIME_Pos)    // clock_period * 2^19
#define CSK_WDT_INTTIME_21              (10UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^21
#define CSK_WDT_INTTIME_23              (11UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^23
#define CSK_WDT_INTTIME_25              (12UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^25
#define CSK_WDT_INTTIME_27              (13UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^27
#define CSK_WDT_INTTIME_29              (14UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^29
#define CSK_WDT_INTTIME_31              (15UL << CSK_WDT_INTTIME_Pos)   // clock_period * 2^31

/*----- WDT Control Codes: reset interval time: 4 bits -----*/
/*----- actual time = clock_period * 2^(CSK_WDT_RSTTIME_X) -----*/
#define CSK_WDT_RSTTIME_Pos             8
#define CSK_WDT_RSTTIME_Msk             (0x0FUL << CSK_WDT_RSTTIME_Pos)

#define CSK_WDT_RSTTIME_7               (0UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^7
#define CSK_WDT_RSTTIME_8               (1UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^8
#define CSK_WDT_RSTTIME_9               (2UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^9
#define CSK_WDT_RSTTIME_10              (3UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^10
#define CSK_WDT_RSTTIME_11              (4UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^11
#define CSK_WDT_RSTTIME_12              (5UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^12
#define CSK_WDT_RSTTIME_13              (6UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^13
#define CSK_WDT_RSTTIME_14              (7UL << CSK_WDT_RSTTIME_Pos)    // clock_period * 2^14

#define __HAL_AON_WDT_AP_FEED_EN()              (((AON_RegDef*)AON_PMUCTRL_BASE)->REG_AON_IWDT_FEED_KEY.bit.AP_WDT_FEED_EN_KEY = 0xCAFE)
#define __HAL_AON_WDT_AP_FEED_DIS()             (((AON_RegDef*)AON_PMUCTRL_BASE)->REG_AON_IWDT_FEED_KEY.bit.AP_WDT_FEED_EN_KEY = 0x0)

#define __HAL_AON_WDT_CP_FEED_EN()              (((AON_RegDef*)AON_PMUCTRL_BASE)->REG_AON_IWDT_FEED_KEY.bit.CP_WDT_FEED_EN_KEY = 0xCAFE)
#define __HAL_AON_WDT_CP_FEED_DIS()             (((AON_RegDef*)AON_PMUCTRL_BASE)->REG_AON_IWDT_FEED_KEY.bit.CP_WDT_FEED_EN_KEY = 0x0)

typedef void (*__cp_wdt_callback)(void* para);

CSK_DRIVER_VERSION WDT_GetVersion(void);

int32_t WDT_Initialize(void* res);

int32_t WDT_Uninitialize(void* res);

int32_t WDT_PowerControl(void* res, CSK_POWER_STATE state);

int32_t WDT_Control(void* res, uint32_t control);

int32_t WDT_Enable(void* res);

int32_t WDT_Refresh(void* res);

int32_t WDT_Disable(void* res);

int32_t HAL_CP_WDT_RegisterCallback(__cp_wdt_callback callback, void* para);

void* WDT(void);
void* AON_WDT(void);
#endif /* __DRIVER_WDT_H */
