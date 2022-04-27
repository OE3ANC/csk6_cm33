/*
 * Copyright (c) 2012-2020 ListenAI Corporation
 * All rights reserved.
 *
 */
 
#ifndef __GPT_CSK6_REGFILE_H__
#define __GPT_CSK6_REGFILE_H__

#include <stdint.h>

/*--------------- defined in driver common file begin ------------------------------*/
#define __I	volatile const
#define __O volatile
#define __IO volatile

/*---------------------------- register definition ---------------------------------*/
typedef struct {
    __I		unsigned int IDREV;                		/* 0x00 ID and Revision Register */
    __IO 	unsigned int CHx_CLK_CTRL[8];      		/* 0x04-0x20 channelx clock control */
    __IO 	unsigned int CHx_CTRL[8];      			/* 0x24-0x40 channelx mode control */
    __IO 	unsigned int CHx_RELOAD[8];      		/* 0x44-0x60 channelx reload value */	
		__IO 	unsigned int CNT_MODE_CTRL_1;		/* 0x64 channel7-channel4 counter direction control */
		__IO 	unsigned int CNT_MODE_CTRL_0;		/* 0x68 channel3-channel0 counter direction control */
		__IO 	unsigned int RUN_MODE_CTRL_1;		/* 0x6C channel7-channel4 run mode control */
		__IO 	unsigned int RUN_MODE_CTRL_0;		/* 0x70 channel3-channel0 run mode control */
		__I 	unsigned int CHx_CNT[8];		  	/* 0x74-0x90 current counter value for channelx */
		__O 	unsigned int CH_CNT_EN;				/* 0x94 counter control for channelx */
		__I 	unsigned int CH_NUM;				/* 0x98 number of channels */
		__IO 	unsigned int CHx_MATCH_0[8];		/* 0x9C-0xB8 compare match data */
		__IO 	unsigned int DMA_CTRL;				/* 0xBC dma status */
		__IO 	unsigned int IMR_CH;				/* 0xC0 ledc and input capture interrupt mask */
		__IO 	unsigned int IMR_TIMER;				/* 0xC4 timer interrupt mask */
		__I 	unsigned int ISR_CH;				/* 0xC8 ledc and input capture interrupt status */
		__I 	unsigned int ISR_TIMER;				/* 0xCC timer interrupt status */
		__O 	unsigned int ICR_CH;				/* 0xD0 ledc and input capture interrupt clear */
		__O 	unsigned int ICR_TIMER;				/* 0xD4 timer1-timer4 interrupt clear */
		__IO 	unsigned int CH_TIMER_ENABLE;		/* 0xD8 timer enable */
		__O 	unsigned int CHxLEDC_TX_FIFO[8];	/* 0xDC-0xF8 ledc channelx transmit fifo for channelx */
		__I 	unsigned int CHx_RX_FIFO[8];		/* 0xFC-0x118 channelx rx fifo */				
		__I 	unsigned int FIFO_STATUS_0;			/* 0x11C channel4-channel7 used data for tx&rx fifo */
		__I 	unsigned int FIFO_STATUS_1;			/* 0x120 channel0-channel3 used data for tx&rx fifo */
		__I 	unsigned int FIFO_STATUS_2;			/* 0x124 channelx fifo status */
		__O 	unsigned int FIFO_CONFIG;			/* 0x128 clear tx&rx fifo write/read pointer */
		__IO	unsigned int SHADOW_SYNC;			/* 0x12C channel0-channel7 shadow register update mode*/
		__O 	unsigned int SHADOW_LOAD;			/* 0x130 channel0-channel7 shadow register load active*/
		__O 	unsigned int IRSR_CH;				/* 0x134 ledc and input capture interrupt raw status */
		__O 	unsigned int IRSR_TIMER;			/* 0x138 timer0 - timer3 interrupt raw status*/
		__O 	unsigned int ICR_FIFO;				/* 0x13C clear fifo overflow&underflow interrupt status*/
} GPT_RegDef;

#endif /* __GPT_CSK6_REGFILE_H__ */




