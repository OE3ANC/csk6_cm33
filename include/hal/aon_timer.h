/*
 * aon_timer.h
 *
 *  Created on: Mar 29, 2022
 *      Author: USER
 */

#ifndef CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_TIMER_H_
#define CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_TIMER_H_

#include "Driver_AON_TIMER.h"

#define AON_TIMER_LOAD_VALUE_MASK       (0xFFFFFF)

#define AON_TIMER_FLAG_INITIALIZED      (1UL << 0)
#define AON_TIMER_FLAG_POWERED          (1UL << 1)

typedef struct _AON_TIMER_INFO {
    HAL_AON_TIMER_SignalEvent_t cb_event;
    void* workspace;
    uint32_t reload_cnt;
    uint32_t mode;
} AON_TIMER_INFO;

typedef struct _AON_TIMER_RESOURCES {
    AON_TIMER_RegDef* reg;
    uint32_t irq_num;
    void (*irq_handler)(void);
    AON_TIMER_INFO* info;
    uint32_t flags;
} AON_TIMER_RESOURCES;

#endif /* CHIP_VEGA_DRIVER_PRIVATE_INCLUDE_AON_TIMER_H_ */
