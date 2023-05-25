/**
  ******************************************************************************
  * @file    gpadc_venus.c
  * @author  ListenAI Application Team
  * @brief   GPADC HAL module driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 ListenAI.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ListenAI under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpadc_venus.h"


/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @defgroup GPADC GPADC
  * @brief GPADC HAL module driver
  * @{
  */




/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define CSK_GPADC_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#define CHECK_RESOURCES(res)  do{\
        if(res != &gpadc_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)


/* Private function prototypes -----------------------------------------------*/
static void GPADC0_IRQ_Handler(void);

/* Private macro -------------------------------------------------------------*/
#define AP_GPADC_DMAREQ			(1<<15)

/* Private variables ---------------------------------------------------------*/
static const CSK_DRIVER_VERSION gpadc_driver_version = {
        CSK_GPADC_API_VERSION,
        CSK_GPADC_DRV_VERSION
};

GPADC_INFO info_gpadc = {
        0,
        {(void*)0},
		(void*)0,
		{0},
		(void*)0,
};

static const GPADC_RESOURCES gpadc_resources = {
		VENUS_GPADC,
		IRQ_GPADC_VECTOR,
		GPADC0_IRQ_Handler,
        &info_gpadc,
};

/* Private functions ---------------------------------------------------------*/
static void
gpadc_irq_handler(GPADC_RESOURCES* hgpadc){

	uint32_t intflag, intflag2, channelflag, cmpflag;

	intflag = hgpadc->reg->REG_ADC_ISR0.all;
	intflag2 = hgpadc->reg->REG_ADC_ISR1.all;
	for(uint32_t cnt = 0; cnt < CSK_GPADC_CHANNEL_NUM; cnt++){
		channelflag = intflag & ((0x01<<cnt)<<CSK_ADC_ISR_FIFO_EMPTY_Pos);
		hgpadc->user_param = channelflag;
		if(channelflag){
			hgpadc->reg->REG_ADC_ICR0.all = channelflag;
			hgpadc->info->cb_event[cnt](CSK_GPADC_FIFO_EMPTY, (void*)hgpadc->user_param);
		}

		channelflag = intflag2 & ((0x01<<cnt)<<CSK_ADC_ISR_FIFO_FULL_Pos);
		hgpadc->user_param = channelflag;
		if(channelflag){
			hgpadc->reg->REG_ADC_ICR1.all = channelflag;
			hgpadc->info->cb_event[cnt](CSK_GPADC_FIFO_FULL, (void*)hgpadc->user_param);
		}

		channelflag = intflag2 & ((0x01<<cnt)<<CSK_ADC_ISR_FIFO_THD_Pos);
		hgpadc->user_param = channelflag;
		if(channelflag){
			hgpadc->reg->REG_ADC_ICR1.all = channelflag;
			hgpadc->info->cb_event[cnt](CSK_GPADC_FIFO_THD, (void*)hgpadc->user_param);
		}
	}

	cmpflag = intflag & (0x01<<CSK_ADC_ISR_COMPLETE_Pos);
	if(cmpflag){
		hgpadc->reg->REG_ADC_ICR0.all = cmpflag;
		while(hgpadc->reg->REG_ADC_IRSR0.bit.ADC_COMPLETE_IRSR == 1){
			hgpadc->reg->REG_ADC_ICR0.all = cmpflag;
		}

		hgpadc->user_param = hgpadc->reg->REG_ADC_CH_SEL.all & 0xFFFF;
		if(hgpadc->reg->REG_ADC_IRSR0.bit.EOC_ERR_IRSR == 1){
			hgpadc->reg->REG_ADC_ICR0.bit.EOC_ERR_ICR = 1;
			while(hgpadc->reg->REG_ADC_IRSR0.bit.EOC_ERR_IRSR == 1){
				hgpadc->reg->REG_ADC_ICR0.bit.EOC_ERR_ICR = 1;
			}
			hgpadc->info->cmp_event(CSK_GPADC_EOC_ERROR, (void*)hgpadc->user_param);
		}else{
			hgpadc->info->cmp_event(CSK_GPADC_COMPLETE, (void*)hgpadc->user_param);
		}

	}
}

static void
GPADC0_IRQ_Handler(void)
{
	gpadc_irq_handler(GPADC());
}


/** @defgroup GPADC_Exported_Functions GPADC Exported Functions
  * @{
  */

/** @defgroup GPADC_Exported_Functions_Group1 Initialization and de-initialization functions
  *  @brief    Initialization and de-initialization functions
  *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
    [..]
		add description

@endverbatim
  * @{
  */

/**
  * @brief add description.
  * @note add description
  * @retval None
  */
void* GPADC(void){
    return (void*)&gpadc_resources;
}


/**
  * @brief add description.
  * @note add description
  * @retval CSK_DRIVER_VERSION
  */
CSK_DRIVER_VERSION
HAL_GPADC_GetVersion(void){
    return gpadc_driver_version;
}

/**
  * @brief GPADC initialize function.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_GPADC_Initialize(void *res){

    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	pGpadc->info->flags |= GPADC_FLAG_POWERED;

	//mask all interrupt
	pGpadc->reg->REG_ADC_IMR0.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_IMR1.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_ICR0.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_ICR1.all = 0xFFFFFFFF;

    register_ISR(pGpadc->irq_num, pGpadc->irq_handler, NULL);

    enable_IRQ(pGpadc->irq_num);

    if(pGpadc->reg->REG_ADC_CR.bit.ADC_EN != 1){
    	pGpadc->reg->REG_ADC_CR.bit.ADC_EN = 1;
    	while(pGpadc->reg->REG_ADC_IRSR0.bit.ADC_READY_IRSR == 0);
//    	pGpadc->reg->REG_ADC_ICR0.bit.ADC_READY_ICR = 1;
    }

    return CSK_DRIVER_OK;
}

/**
  * @brief GPADC uninitialize function.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_GPADC_Uninitialize(void *res){
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//mask all interrupt
	pGpadc->reg->REG_ADC_IMR0.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_IMR1.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_ICR0.all = 0xFFFFFFFF;
	pGpadc->reg->REG_ADC_ICR1.all = 0xFFFFFFFF;

    disable_IRQ(pGpadc->irq_num);

    register_ISR(pGpadc->irq_num, NULL, NULL);

	pGpadc->reg->REG_ADC_CR.bit.ADC_EN = 0;

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_GPADC_Control(void* res, uint32_t control){
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;
	uint32_t tmp;

    if ((pGpadc->info->flags & GPADC_FLAG_POWERED) == 0U) {
        // UART not powered
        return CSK_DRIVER_ERROR;
    }

    //ADC periodical sampling wait time (high)
    //default value: 0x4e
    pGpadc->reg->REG_ADC_CR.bit.ADC_RWAIT_HIGH = 0x1;

    //ADC periodical sampling wait time (lower)
    //default value: 0xff
    pGpadc->reg->REG_ADC_CR.bit.ADC_RWAIT_LOW = 0x0;

    //Set waiting time after setting
    //default value: 0x01
    pGpadc->reg->REG_ADC_CR.bit.ADC_SETUP_WAIT= 0xFF;

    //trigger mode
    tmp = (control & CSK_GPADC_TRIGGER_CONTROL_Msk) >> CSK_GPADC_TRIGGER_CONTROL_Pos;
<<<<<<< HEAD
    pGpadc->reg->REG_ADC_CR.bit.HARD_TRIG_EN = tmp;
=======
    pGpadc->reg->REG_ADC_CR.bit.HARD_TRIG_EN = tmp&0x01;
    pGpadc->reg->REG_ADC_CR.bit.GPT_TRIG_EN = (tmp&0x02)>>1;
>>>>>>> feature/zephyr

    //fifo write clock
    pGpadc->reg->REG_ADC_CR.bit.FORCE_FIFO_WCLK_ON = 1;

    //Sample time
    tmp = (control & CSK_GPADC_SAMPLETIME_Msk) >> CSK_GPADC_SAMPLETIME_Pos;
    pGpadc->reg->REG_ADC_CONFIG.bit.SAMPLE_T_SET = tmp;

    //Clock frequency
    tmp = (control & CSK_GPADC_CLKFREQ_Msk) >> CSK_GPADC_CLKFREQ_Pos;
    pGpadc->reg->REG_ADC_CONFIG.bit.ADC_CLK_SET = tmp;

    //Channel select
    tmp = (control & CSK_GPADC_CHANNEL_SEL_Msk) >> CSK_GPADC_CHANNEL_SEL_Pos;
    pGpadc->reg->REG_ADC_CH_SEL.bit.ADC_CH_SEL = tmp;
    if((control & CSK_GPADC_DMA_ENABLE_Msk) >> CSK_GPADC_DMA_ENABLE_Pos){
    	pGpadc->reg->REG_ADC_CH_SEL.bit.DMA_CH_EN = tmp;
    	//dma request for gpadc
    	IP_SYSCTRL->REG_AP_CTRL1.bit.AP_DMA_HS_SEL |= AP_GPADC_DMAREQ;
    }else{
    	pGpadc->reg->REG_ADC_CH_SEL.bit.DMA_CH_EN &= ~tmp;
    	IP_SYSCTRL->REG_AP_CTRL1.bit.AP_DMA_HS_SEL &= ~AP_GPADC_DMAREQ;
    }

    //channel VCC special, need LDO25 ON
    if((control & CSK_GPADC_CHANNEL_SEL_VCC) == CSK_GPADC_CHANNEL_SEL_VCC){
    	IP_ANA->REG_0X038.bit.LDO25_EN = 0x1;
    }

    //CVD5 selected
    if((pGpadc->reg->REG_ADC_CH_SEL.bit.ADC_CH_SEL & (CSK_GPADC_CHANNEL_SEL_CVD5 >> CSK_GPADC_CHANNEL_SEL_Pos)) != 0){
    	//select cbutton, not keysense
    	IP_ANA->REG_0X034.all |= 1<<4;
    }

    //input mode
    tmp = (control & CSK_GPADC_INPUT_MODE_Msk) >> CSK_GPADC_INPUT_MODE_Pos;
    pGpadc->reg->REG_ADC_CONFIG.bit.SINGLE = tmp;
    if(tmp == 0){
    	pGpadc->reg->REG_ADC_CONFIG.bit.DIFFER_VINN_SEL = 0;
    }

    //driver strength & vref select
    tmp = (control & CSK_GPADC_VREF_SEL_Msk) >> CSK_GPADC_VREF_SEL_Pos;
    pGpadc->reg->REG_ADC_CONFIG.bit.VREF_BUF_EN = tmp>>2;
    pGpadc->reg->REG_ADC_CONFIG.bit.VREF_SEL = tmp&0x03;

    //keysense hardware trigger
    //clear hardware trigger bit first
    pGpadc->reg->REG_ADC_CR.bit.HARD_TRIG_EN = 0;
    tmp = (control & CSK_GPADC_TRIGGER_CONTROL_Msk) >> CSK_GPADC_TRIGGER_CONTROL_Pos;
    pGpadc->reg->REG_ADC_CR.bit.HARD_TRIG_EN = tmp;

    pGpadc->reg->REG_ADC_CONFIG.bit.EN_VIN_BUF_AMP = 1;
    pGpadc->reg->REG_ADC_CONFIG.bit.BYPASS_VIN_BUF = 0;

    return CSK_DRIVER_OK;
}

/**
  * @}
  */


/** @defgroup GPADC_Exported_Functions_Group2 control functions
  *  @brief    Interrupt enable/disable functions
  *
@verbatim
 ===============================================================================
              ##### Interrupt Enable and disable functions #####
              ##### Module start and stop functions #####
 ===============================================================================
    [..]
		add description

@endverbatim
  * @{
  */

/**
  * @brief  Enables ADC and starts conversion of the regular channels.
  * @param  hadc pointer to a ADC_HandleTypeDef structure that contains
  *         the configuration information for the specified ADC.
  * @retval int32_t
  */
uint32_t HAL_GPADC_Start(void* res)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//software adc conversion
	pGpadc->reg->REG_ADC_CR.bit.SOFT_TRIG = 1;

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
uint32_t HAL_GPADC_Stop(void* res)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//if not complete, clear status manually
	if(pGpadc->reg->REG_ADC_IRSR0.bit.ADC_COMPLETE_IRSR == 1){
		pGpadc->reg->REG_ADC_ICR0.bit.ADC_COMPLETE_ICR = 1;
	}
	pGpadc->reg->REG_ADC_CR.bit.ADC_EN = 0;

	//software adc conversion
	pGpadc->reg->REG_ADC_CR.bit.SOFT_TRIG = 0;

	return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
uint32_t HAL_GPADC_PollForConversion(void* res, uint32_t Timeout)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	if(pGpadc->reg->REG_ADC_CH_SEL.bit.DMA_CH_EN == 0){
		while(pGpadc->reg->REG_ADC_IRSR0.bit.ADC_COMPLETE_IRSR == 0);
		pGpadc->reg->REG_ADC_ICR0.bit.ADC_COMPLETE_ICR = 1;
	}

	return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
uint32_t HAL_GPADC_Start_IT(void* res)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//mask complete = 1
	pGpadc->reg->REG_ADC_IMR0.bit.ADC_COMPLETE_IMR = 1;

	//if not complete, clear status manually
	if(pGpadc->reg->REG_ADC_IRSR0.bit.ADC_COMPLETE_IRSR == 1){
		pGpadc->reg->REG_ADC_ICR0.bit.ADC_COMPLETE_ICR = 1;
	}

	//mask complete = 0
	pGpadc->reg->REG_ADC_IMR0.bit.ADC_COMPLETE_IMR = 0;

<<<<<<< HEAD
//	//software adc conversion
//	pGpadc->reg->REG_ADC_CR.bit.SOFT_TRIG = 1;
=======
	//software adc conversion
	pGpadc->reg->REG_ADC_CR.bit.SOFT_TRIG = 1;
>>>>>>> feature/zephyr

	return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
uint32_t HAL_GPADC_Stop_IT(void* res)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//mask complete = 1
	pGpadc->reg->REG_ADC_IMR0.bit.ADC_COMPLETE_IMR = 1;

	//if not complete, clear status manually
	if(pGpadc->reg->REG_ADC_IRSR0.bit.ADC_COMPLETE_IRSR == 1){
		pGpadc->reg->REG_ADC_ICR0.bit.ADC_COMPLETE_ICR = 1;
	}
	pGpadc->reg->REG_ADC_CR.bit.ADC_EN = 0;

	//software adc conversion
	pGpadc->reg->REG_ADC_CR.bit.SOFT_TRIG = 0;

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval uint32_t
  */
uint32_t HAL_GPADC_SetTriggerNum(void* res, uint32_t trignum)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	pGpadc->reg->REG_ADC_CONFIG.bit.ADC_TRIG_NUM = trignum;

	return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval uint32_t
  */
uint32_t HAL_GPADC_SetSampleWaitTime(void* res, uint32_t waitTime)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//ADC periodical sampling wait time (high)
	pGpadc->reg->REG_ADC_CR.bit.ADC_RWAIT_HIGH = (waitTime&0xFF00) >> 8;
	//ADC periodical sampling wait time (lower)
	pGpadc->reg->REG_ADC_CR.bit.ADC_RWAIT_LOW = (waitTime&0x00FF) >> 0;

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval uint32_t
  */
uint32_t HAL_GPADC_SetFifoThd(void* res, uint8_t channelnum, uint8_t threshold)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	if(channelnum < 8){
		pGpadc->reg->REG_ADC_FIFO_THD0.all = threshold;
	}else if(channelnum < 14){
		pGpadc->reg->REG_ADC_FIFO_THD1.all = threshold;
	}else{
		return CSK_DRIVER_ERROR_PARAMETER;
	}

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note channelnum: GPADC_CHANNEL_TYPE
  * @retval uint32_t
  */
uint32_t HAL_GPADC_EnableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

    switch (type) {
    case CSK_GPADC_FIFO_EMPTY:
    	pGpadc->reg->REG_ADC_IMR0.all &= ~(0x01<<channelnum<<16);
        break;
    case CSK_GPADC_FIFO_FULL:
    	pGpadc->reg->REG_ADC_IMR1.all &= ~(0x01<<channelnum<<0);
        break;
    case CSK_GPADC_FIFO_THD:
    	pGpadc->reg->REG_ADC_IMR1.all &= ~(0x01<<channelnum<<16);
    	break;
    default:
    	return CSK_DRIVER_ERROR_PARAMETER;
    }

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note channelnum: GPADC_CHANNEL_TYPE
  * @retval uint32_t
  */
uint32_t HAL_GPADC_DisableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

    switch (type) {
    case CSK_GPADC_FIFO_EMPTY:
    	pGpadc->reg->REG_ADC_IMR0.all |= (0x01<<channelnum<<16);
        break;
    case CSK_GPADC_FIFO_FULL:
    	pGpadc->reg->REG_ADC_IMR1.all |= (0x01<<channelnum<<0);
        break;
    case CSK_GPADC_FIFO_THD:
    	pGpadc->reg->REG_ADC_IMR1.all |= (0x01<<channelnum<<16);
    	break;
    default:
    	return CSK_DRIVER_ERROR_PARAMETER;
    }

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note channelnum: GPADC_CHANNEL_TYPE
  * @retval uint32_t
  */
uint32_t HAL_GPADC_ClearFifoIntStatus(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

    switch (type) {
    case CSK_GPADC_FIFO_EMPTY:
    	pGpadc->reg->REG_ADC_ICR0.all |= (0x01<<channelnum<<16);
        break;
    case CSK_GPADC_FIFO_FULL:
    	pGpadc->reg->REG_ADC_ICR1.all |= (0x01<<channelnum<<0);
        break;
    case CSK_GPADC_FIFO_THD:
    	pGpadc->reg->REG_ADC_ICR1.all |= (0x01<<channelnum<<16);
    	break;
    default:
    	return CSK_DRIVER_ERROR_PARAMETER;
    }

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note channelnum: GPADC_CHANNEL_TYPE
  * @retval uint32_t
  */
uint32_t HAL_GPADC_GetFifoIntStatus(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	uint32_t intStatus;

    switch (type) {
    case CSK_GPADC_FIFO_EMPTY:
    	intStatus = pGpadc->reg->REG_ADC_IRSR0.all & (0x01<<channelnum<<16);
        break;
    case CSK_GPADC_FIFO_FULL:
    	intStatus = pGpadc->reg->REG_ADC_IRSR1.all & (0x01<<channelnum<<0);
        break;
    case CSK_GPADC_FIFO_THD:
    	intStatus = pGpadc->reg->REG_ADC_IRSR1.all & (0x01<<channelnum<<16);
    	break;
    default:
    	return CSK_DRIVER_ERROR_PARAMETER;
    }

	return intStatus;
}


/**
  * @brief add description.
  * @note channelnum: GPADC_CHANNEL_TYPE
  * @retval uint32_t
  */
uint32_t HAL_GPADC_GetFifoCnt(void* res, uint8_t channelnum)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;
	uint32_t fifocnt;

	if(channelnum >= 8){
		fifocnt = (pGpadc->reg->REG_ADC_FIFO_DATA_CNT1.all & (0x0F<<((channelnum-8)<<2))) >> ((channelnum-8)<<2);
	}else{
		fifocnt = (pGpadc->reg->REG_ADC_FIFO_DATA_CNT0.all & (0x0F<<(channelnum<<2))) >> (channelnum<<2);
	}

	return fifocnt;
}




/**
  * @brief add description.
  * @note add description
  * @retval uint32_t
  */
uint32_t HAL_GPADC_SetSetupWaitTime(void* res, uint8_t waitTime)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	//Set waiting time after setting
	//default value: 0x01
	pGpadc->reg->REG_ADC_CR.bit.ADC_SETUP_WAIT= waitTime;

	return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval int16_t
  */
uint16_t HAL_GPADC_GetValue(void* res, uint32_t channelnum)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	uint16_t adc_value = 0;

	switch(channelnum){
		case CSK_GPADC_CHANNEL_SEL_0:
			adc_value = pGpadc->reg->REG_ADC_RDR0.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_1:
			adc_value = pGpadc->reg->REG_ADC_RDR1.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_2:
			adc_value = pGpadc->reg->REG_ADC_RDR2.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_3:
			adc_value = pGpadc->reg->REG_ADC_RDR3.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_VDDIO:
			adc_value = pGpadc->reg->REG_ADC_RDR4.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_VCC:
			adc_value = pGpadc->reg->REG_ADC_RDR5.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_KEYSENSE:
			adc_value = pGpadc->reg->REG_ADC_RDR6.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_TEMP:
			adc_value = pGpadc->reg->REG_ADC_RDR7.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD0:
			adc_value = pGpadc->reg->REG_ADC_RDR8.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD1:
			adc_value = pGpadc->reg->REG_ADC_RDR9.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD2:
			adc_value = pGpadc->reg->REG_ADC_RDR10.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD3:
			adc_value = pGpadc->reg->REG_ADC_RDR11.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD4:
			adc_value = pGpadc->reg->REG_ADC_RDR12.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD5:
			adc_value = pGpadc->reg->REG_ADC_RDR13.all;
			break;
		default:
			break;
	}

	return adc_value;
}


uint32_t HAL_GPADC_CVD_GetValue(void* res, uint32_t channelnum)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	uint32_t adc_value = 0;

	switch(channelnum){
		case CSK_GPADC_CHANNEL_SEL_0:
			adc_value = pGpadc->reg->REG_ADC_RDR0.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_1:
			adc_value = pGpadc->reg->REG_ADC_RDR1.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_2:
			adc_value = pGpadc->reg->REG_ADC_RDR2.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_3:
			adc_value = pGpadc->reg->REG_ADC_RDR3.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_VDDIO:
			adc_value = pGpadc->reg->REG_ADC_RDR4.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_VCC:
			adc_value = pGpadc->reg->REG_ADC_RDR5.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_KEYSENSE:
			adc_value = pGpadc->reg->REG_ADC_RDR6.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_TEMP:
			adc_value = pGpadc->reg->REG_ADC_RDR7.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD0:
			adc_value = pGpadc->reg->REG_ADC_RDR8.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD1:
			adc_value = pGpadc->reg->REG_ADC_RDR9.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD2:
			adc_value = pGpadc->reg->REG_ADC_RDR10.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD3:
			adc_value = pGpadc->reg->REG_ADC_RDR11.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD4:
			adc_value = pGpadc->reg->REG_ADC_RDR12.all;
			break;
		case CSK_GPADC_CHANNEL_SEL_CVD5:
			adc_value = pGpadc->reg->REG_ADC_RDR13.all;
			break;
		default:
			break;
	}

	return adc_value;
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t HAL_GPADC_RegisterChannelCallback(void *res, GPADC_CHANNEL_TYPE channel, CSK_GPADC_SignalEvent_t cb_event)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	pGpadc->info->cb_event[channel] = cb_event;

	return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t HAL_GPADC_RegisterCompleteCallback(void *res, CSK_GPADC_SignalEvent_t cb_event)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;

	pGpadc->info->cmp_event = cb_event;

	return CSK_DRIVER_OK;
}


uint32_t HAL_GPADC_CVD_Config(void* res, uint32_t gr_state, uint32_t db_sample_state, uint32_t second_charge_invert)
{
    CHECK_RESOURCES(res);

	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;
	pGpadc->reg->REG_CVD_CTRL.all = 0;

	//guard ring
	pGpadc->reg->REG_CVD_CTRL.bit.GUARD_RING_EN = gr_state;

	//dobule sample
	pGpadc->reg->REG_CVD_CTRL.bit.DOUBLE_SAMP_EN = db_sample_state;

	//adipen
	pGpadc->reg->REG_CVD_CTRL.bit.ADIPEN = second_charge_invert;

	return CSK_DRIVER_OK;
}


uint32_t HAL_GPADC_CVD_Control(void* res, uint32_t control)
{
	GPADC_RESOURCES * pGpadc = (GPADC_RESOURCES *)res;
	uint32_t tmp;

    if ((pGpadc->info->flags & GPADC_FLAG_POWERED) == 0U) {
        // UART not powered
        return CSK_DRIVER_ERROR;
    }

    //CVD prechargeA timer
    tmp = (control & CSK_GPADC_PRECHARGEA_CONTROL_Msk) >> CSK_GPADC_PRECHARGEA_CONTROL_Pos;
    pGpadc->reg->REG_PRE_CHA_TIMER.all = tmp;

    //CVD prechargeB timer
    tmp = (control & CSK_GPADC_PRECHARGEB_CONTROL_Msk) >> CSK_GPADC_PRECHARGEB_CONTROL_Pos;
    pGpadc->reg->REG_PRE_CHB_TIMER.all = tmp;

    //CVD Acquicition timer
    tmp = (control & CSK_GPADC_ACQUICITION_CONTROL_Msk) >> CSK_GPADC_ACQUICITION_CONTROL_Pos;
    pGpadc->reg->REG_ACQ_TIMER.all = tmp;

    //CVD Additional Capacitor
    tmp = (control & CSK_GPADC_ADDITIONAL_CAP_CONTROL_Msk) >> CSK_GPADC_ADDITIONAL_CAP_CONTROL_Pos;
    pGpadc->reg->REG_ADD_CAP.all = tmp;

	return CSK_DRIVER_OK;
}


/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

