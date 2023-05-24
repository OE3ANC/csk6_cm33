/*
 * aon_wdt.h
 *
 *  Created on: Mar 30, 2022
 *      Author: USER
 */

#ifndef CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_WDT_H_
#define CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_WDT_H_

#include "Driver_AON_WDT.h"

#define AON_WDT_LOAD_VALUE_MASK                     (0xFFFFFF)

#define AON_WDT_FLAG_INITIALIZED                    (1UL << 0)
#define AON_WDT_FLAG_POWERED                        (1UL << 1)

#define AON_WDT_STOP_PROTECT_LOCK                    0xDEADFACE
#define AON_WDT_STOP_PROTECT_RELEASE                 0xBABEBEEF

#define AON_WDT_START_PROTECT_LOCK                   0xBADBEE01
#define AON_WDT_START_PROTECT_RELEASE                0xBADBEE00

#define AON_WDT_RESET_PMU_CTRL                       0x5856E201   /// Reset include AON domain
#define AON_WDT_RESET_DBB                            0x5856e200   /// Reset exclude AON domain

typedef struct _AON_WDT_INFO {
    HAL_AON_WDT_SignalEvent_t cb_event;
    void* workspace;
} AON_WDT_INFO;

typedef struct _AON_WDT_RESOURCES {
    AON_TIMER_RegDef* reg;
    uint32_t irq_num;
    void (*irq_handler)(void);
    AON_WDT_INFO* info;
    uint32_t flags;
} AON_WDT_RESOURCES;

#endif /* CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_WDT_H_ */

