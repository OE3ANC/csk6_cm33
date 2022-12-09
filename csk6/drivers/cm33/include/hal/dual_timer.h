/*
 * timer_csk6001.h
 *
 *  Created on: 202年8月4号
 *      Author: USER
 */

#ifndef __TIMER_CSK6001_H__
#define __TIMER_CSK6001_H__

#include "dual_timer_reg_venus.h"

#include "Driver_DUAL_TIMER.h"

#include "venus_ap.h"

// CONSTANT MACRO
// ******************************************TIMER0
#define VENUS_TIMER0                  ((DUAL_TIMER_RegDef*) TIMER_BASE)
// ******************************************

#define TIMER_FLAG_INITIALIZED      (1UL << 0)
#define TIMER_FLAG_POWERED          (1UL << 1)
#define TIMER_FLAG_CH_ACTIVED(CH)   (1UL << (2 + CH))

#define CSK_TIMER_MAX_CHANNEL_NUM   (2)
#define CSK_TIMER_CLR_INT           (0x951202)


typedef struct _TIMER_INFO {
    CSK_TIMER_SignalEvent_t cb_event;
    void* workspace;
    uint32_t reload;
    uint32_t cnt;
} TIMER_INFO;

typedef struct _TIMER_RESOURCES {
    DUAL_TIMER_RegDef* reg;
    uint32_t irq_num;
    void (*irq_handler)(void);
    TIMER_INFO info[CSK_TIMER_MAX_CHANNEL_NUM];
    uint32_t flags;
} TIMER_RESOURCES;

#endif /*  */
