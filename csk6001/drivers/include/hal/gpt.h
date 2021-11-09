/*
 * Copyright (c) 2012-2020 ListenAI Corporation
 * All rights reserved.
 *
 */

#ifndef __GPT_CSK6001_H
#define __GPT_CSK6001_H
 
#include <stdint.h>

#include "gpt_common.h"
#include "venus_ap.h"
#include "clock_config.h"

/*--------------- defined in driver common file begin ------------------------------*/
#define __I	volatile const
#define __O volatile
#define __IO volatile

#ifndef NULL
#define NULL                        (void *)0
#endif

#define PCLOCK	 	100000000
#define CLOCK_XTAL 	24000000
#define CLOCK_EXT 	12000000
/*--------------- defined in driver common file begin ------------------------------*/


/*----------------------------- private definition ---------------------------------*/
#define GPT_NUMBER_OF_CHANNELS	8

// GPT flags
#define GPT_FLAG_INITIALIZED             (1U << 0)
#define GPT_FLAG_POWERED                 (1U << 1)
#define GPT_FLAG_CHANNEL_ACTIVED(CHN)    (1U << (8 + CHN))

#define LEDC_FIFO_DEPTH		4


typedef enum _HARDWARE_CHANNEL_STATE{
	HARDWARE_CHANNEL_STAT_IDLE,						
	HARDWARE_CHANNEL_STAT_USED_BY_TIM,		
	HARDWARE_CHANNEL_STAT_USED_BY_PWM,			
	HARDWARE_CHANNEL_STAT_USED_BY_IC,
	HARDWARE_CHANNEL_STAT_USED_BY_LEDC
}HARDWARE_CHANNEL_STATE;


typedef enum _TRANSFER_STATE_TYPE{
  POLLING_MODE				  = 0,
  INTERRUPT_MODE              = 1,
  DMA_MODE                    = 2
} TRANSFER_STATE_TYPE;

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
} CSK6001_GPT_RegDef;

/*---------------------------------CHx_CLK_CTRL-------------------------------------*/
#define GPT_CHx_CLK_CTRL_CLK_DIV_Pos            (16U)                                         
#define GPT_CHx_CLK_CTRL_CLK_DIV_Msk            (0x7UL << GPT_CHx_CLK_CTRL_CLK_DIV_Pos)                     
#define GPT_CHx_CLK_CTRL_CLK_DIV                GPT_CHx_CLK_CTRL_CLK_DIV_Msk  

#define GPT_CHx_CLK_CTRL_CLK_SEL_Pos            (19U)                                         
#define GPT_CHx_CLK_CTRL_CLK_SEL_Msk            (0x3UL << GPT_CHx_CLK_CTRL_CLK_SEL_Pos)
#define GPT_CHx_CLK_CTRL_CLK_SEL                GPT_CHx_CLK_CTRL_CLK_SEL_Msk    

#define GPT_CHx_CLK_CTRL_CLK_GATE_Pos           (21U)
#define GPT_CHx_CLK_CTRL_CLK_GATE_Msk           (0x1UL << GPT_CHx_CLK_CTRL_CLK_GATE_Pos)
#define GPT_CHx_CLK_CTRL_CLK_GATE               GPT_CHx_CLK_CTRL_CLK_GATE_Msk

#define GPT_CHx_CLK_CTRL_CLK_PREDIV_LD_Pos      (23U)
#define GPT_CHx_CLK_CTRL_CLK_RPEDIV_LD_Msk      (0x1UL << GPT_CHx_CLK_CTRL_CLK_PREDIV_LD_Pos)
#define GPT_CHx_CLK_CTRL_CLK_PREDIV_LD          GPT_CHx_CLK_CTRL_CLK_RPEDIV_LD_Msk

#define GPT_CHx_CLK_CTRL_CLK_DIV_LD_Pos         (24U)
#define GPT_CHx_CLK_CTRL_CLK_DIV_LD_Msk         (0x1UL << GPT_CHx_CLK_CTRL_CLK_DIV_LD_Pos)
#define GPT_CHx_CLK_CTRL_CLK_DIV_LD             GPT_CHx_CLK_CTRL_CLK_DIV_LD_Msk    

/*---------------------------------CHx_CTRL-------------------------------------*/
#define GPT_CHx_CTRL_CH_OPERATION_Pos           (0U)                                         
#define GPT_CHx_CTRL_CH_OPERATION_Msk           (0x7UL << GPT_CHx_CTRL_CH_OPERATION_Pos)                     
#define GPT_CHx_CTRL_CH_OPERATION               GPT_CHx_CTRL_CH_OPERATION_Msk   

#define GPT_CHx_CTRL_CH_MODE_Pos            	(3U)
#define GPT_CHx_CTRL_CH_MODE_Msk            	(0x7UL << GPT_CHx_CTRL_CH_MODE_Pos)
#define GPT_CHx_CTRL_CH_MODE               		GPT_CHx_CTRL_CH_MODE_Msk

#define GPT_CHx_CTRL_PWM_POLARITY_Pos           (6U)                                         
#define GPT_CHx_CTRL_PWM_POLARITY_Msk           (0x1UL << GPT_CHx_CTRL_PWM_POLARITY_Pos)                     
#define GPT_CHx_CTRL_PWM_POLARITY               GPT_CHx_CTRL_PWM_POLARITY_Msk  

#define GPT_CHx_CTRL_PWMOUT_MODE_Pos            (7U)                                         
#define GPT_CHx_CTRL_PWMOUT_MODE_Msk            (0x1UL << GPT_CHx_CTRL_PWMOUT_MODE_Pos)                     
#define GPT_CHx_CTRL_PWMOUT_MODE               	GPT_CHx_CTRL_PWMOUT_MODE_Msk  

#define GPT_CHx_CTRL_IC_SOURCE_Pos            	(8U)                                         
#define GPT_CHx_CTRL_IC_SOURCE_Msk            	(0x1UL << GPT_CHx_CTRL_IC_SOURCE_Pos)                     
#define GPT_CHx_CTRL_IC_SOURCE               	GPT_CHx_CTRL_IC_SOURCE_Msk

#define GPT_CHx_CTRL_IC_EDGE_Pos            	(9U)
#define GPT_CHx_CTRL_IC_EDGE_Msk            	(0x3UL << GPT_CHx_CTRL_IC_EDGE_Pos)
#define GPT_CHx_CTRL_IC_EDGE               		GPT_CHx_CTRL_IC_EDGE_Msk

#define GPT_CHx_CTRL_IC_FILTER_Pos            	(11U)                                         
#define GPT_CHx_CTRL_IC_FILTER_Msk            	(0x7UL << GPT_CHx_CTRL_IC_FILTER_Pos)                     
#define GPT_CHx_CTRL_IC_FILTER               	GPT_CHx_CTRL_IC_FILTER_Msk

#define GPT_CHx_CTRL_CLK_SAMPLE_GATE_Pos        (14U)
#define GPT_CHx_CTRL_CLK_SAMPLE_GATE_Msk        (0x1UL << GPT_CHx_CTRL_CLK_SAMPLE_GATE_Pos)
#define GPT_CHx_CTRL_CLK_SAMPLE_GATE            GPT_CHx_CTRL_CLK_SAMPLE_GATE_Msk

#define GPT_CHx_CTRL_CLK_CNT_GATE_Pos           (15U)
#define GPT_CHx_CTRL_CLK_CNT_GATE_Msk           (0x1UL << GPT_CHx_CTRL_CLK_CNT_GATE_Pos)
#define GPT_CHx_CTRL_CLK_CNT_GATE               GPT_CHx_CTRL_CLK_CNT_GATE_Msk

#define GPT_CHx_CHTRIG_RESET_CNT_EN_Pos         (17U)
#define GPT_CHx_CHTRIG_RESET_CNT_EN_Msk         (0x1UL << GPT_CHx_CHTRIG_RESET_CNT_EN_Pos)
#define GPT_CHx_CHTRIG_RESET_CNT_EN             GPT_CHx_CHTRIG_RESET_CNT_EN_Msk

#define GPT_CHx_CTRL_CH_STOP_Pos            	(18U)
#define GPT_CHx_CTRL_CH_STOP_Msk            	(0x1UL << GPT_CHx_CTRL_CH_STOP_Pos)
#define GPT_CHx_CTRL_CH_STOP               		GPT_CHx_CTRL_CH_STOP_Msk

#define GPT_CHx_CTRL_CH_START_Pos            	(19U)
#define GPT_CHx_CTRL_CH_START_Msk            	(0x1UL << GPT_CHx_CTRL_CH_START_Pos)
#define GPT_CHx_CTRL_CH_START               	GPT_CHx_CTRL_CH_START_Msk


#define CSK6001_GPT0              				((CSK6001_GPT_RegDef *)  GPT_BASE)


/*------------------------- common resources definition ----------------------------*/
typedef struct _GPT_HARDWARE
{
    volatile HARDWARE_CHANNEL_STATE channel_stat[GPT_NUMBER_OF_CHANNELS];
}GPT_HARDWARE;

// GPT capture data Information (Run-Time)
typedef struct _GPT_IC_CAPTURE_INFO
{
    uint32_t ic_num[GPT_NUMBER_OF_CHANNELS];        // Total number of data to be captured
    uint32_t *ic_buf[GPT_NUMBER_OF_CHANNELS];       // Pointer to input capture buffer
    uint32_t ic_cnt[GPT_NUMBER_OF_CHANNELS];        // Number of data captured
    TRANSFER_STATE_TYPE capture_mode[GPT_NUMBER_OF_CHANNELS];		//capture mode: cpu read capture data or dma receive
} GPT_IC_CAPTURE_INFO;

// GPT transmit data Information (Run-Time)
typedef struct _GPT_LEDC_TRANSMIT_INFO
{
    uint32_t ledc_tx_num[GPT_NUMBER_OF_CHANNELS];       			// Total number of data to be transmitted
    uint32_t *ledc_tx_buf[GPT_NUMBER_OF_CHANNELS];      			// Pointer to transmit buffer
    uint32_t ledc_tx_cnt[GPT_NUMBER_OF_CHANNELS];       			// Number of transtmited data
    TRANSFER_STATE_TYPE ledc_tx_mode[GPT_NUMBER_OF_CHANNELS];		//transmit mode: cpu write transmit or dma transmit
} GPT_LEDC_TRANSMIT_INFO;

/* GPT Information (Run-Time) */
typedef struct _GPT_INFO
{
    uint32_t flags;         								// GPT driver flags
    CSK_GPT_SignalEvent_t cb_event[GPT_NUMBER_OF_CHANNELS]; // Event callback
    uint32_t clk_source[GPT_NUMBER_OF_CHANNELS];
    void (*origin_IRQ_handler)(void);
} GPT_INFO;


// GPT Resources definitions
typedef struct _GPT_RESOURCES
{
	CSK_GPT_CAPABILITIES capabilities;  				// Capabilities
    CSK6001_GPT_RegDef *reg;          					// Pointer to GPT peripheral
    uint32_t irq_num;       							// IRQ Number
    void (*irq_handler)(void);
	
    GPT_INFO *info;         							// Run-Time Information
	GPT_IC_CAPTURE_INFO *ic_capture_info;				// Run-Time Information
	GPT_LEDC_TRANSMIT_INFO *ledc_transmit_info;			// Run-Time Information
    GPT_HARDWARE *hardware;								// Run-Time Information	
    void *user_param;

}GPT_RESOURCES;

#endif /* __GPT_CSK6001_H */




