/*
 * Copyright (c) 2012-2020 CSK Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __GPT_PWM_CSK6001_H
#define __GPT_PWM_CSK6001_H

#include "dma.h"
#include "Driver_GPT_PWM.h"
#include "gpt.h"

typedef struct _GPT_PWM_INFO
{
    uint32_t freq[GPT_NUMBER_OF_CHANNELS];       // PWM channel frequency
    uint32_t duty[GPT_NUMBER_OF_CHANNELS];   	 	 // PWM channel duty cycle (0 ~ 255)
    uint32_t polarity[GPT_NUMBER_OF_CHANNELS];
}GPT_PWM_INFO;

// GPT_LEDC DMA
typedef struct _GPT_LEDC_DMA
{
    uint8_t channel;                       // DMA Channel
    uint8_t reqsel;                        // DMA request selection
    DMA_SignalEvent_t cb_event;            // DMA Event callback
} GPT_LEDC_DMA;


typedef struct _GPT_PWM_RESOURCES
{
	GPT_RESOURCES *gpt_resources;
	GPT_PWM_INFO *pwm_info;	
	GPT_LEDC_DMA *dma_tx;
	uint8_t dma_channel[GPT_NUMBER_OF_CHANNELS];	
}GPT_PWM_RESOURCES;


#endif /* __GPT_PWM_CSK6001_H */
