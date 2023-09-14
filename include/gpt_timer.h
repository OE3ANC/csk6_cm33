/*
 * Copyright (c) 2012-2020 CSK Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __GPT_TIMER_CSK6001_H
#define __GPT_TIMER_CSK6001_H

#include "Driver_GPT_TIMER.h"
#include "gpt.h"
#include "ClockManager.h"

/*-----------------------------Typedef for driver-----------------------------------*/

typedef struct _GPT_TIMER_INFO
{
    uint32_t period_count[GPT_NUMBER_OF_CHANNELS]; // channel period count
}GPT_TIMER_INFO;


typedef struct _GPT_TIMER_RESOURCES
{
	GPT_RESOURCES *gpt_resources;
    GPT_TIMER_INFO *timer_info;
}GPT_TIMER_RESOURCES;




#endif /* __GPT_TIMER_CSK6001_H */
