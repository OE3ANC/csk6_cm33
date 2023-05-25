/**
  ******************************************************************************
  * @file    Driver_GPADC.h
  * @author  ListenAI Application Team
  * @brief   Header file of GPADC HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CSK_DRIVER_GPADC_H
#define __CSK_DRIVER_GPADC_H


#ifdef __cplusplus
 extern "C" {
#endif

 /* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"


/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @addtogroup GPADC
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup GPADC_Exported_Types TRNG Exported Types
  * @{
  */

/**
  * @brief  add description
  */
typedef void (*CSK_GPADC_SignalEvent_t) (uint32_t event, void* workspace);

/*----- GPADC channel define -----*/
typedef enum {
	CSK_GPADC_CHANNEL0                  = 0,                /**< GPADC channel 0 */
	CSK_GPADC_CHANNEL1                  = 1,                /**< GPADC channel 1 */
	CSK_GPADC_CHANNEL2                  = 2,                /**< GPADC channel 2 */
	CSK_GPADC_CHANNEL3                  = 3,                /**< GPADC channel 3 */
	CSK_GPADC_VDDIO                   	= 4,                /**< GPADC channel 4 */
	CSK_GPADC_VCC                     	= 5,                /**< GPADC channel 5 */
	CSK_GPADC_KEYSENSE                  = 6,                /**< GPADC channel 6 */
	CSK_GPADC_TEMPSENSOR                = 7,				/**< GPADC channel 7 */
	CSK_GPADC_CVD0                  	= 8,                /**< GPADC channel 8 */
	CSK_GPADC_CVD1                		= 9,				/**< GPADC channel 9 */
	CSK_GPADC_CVD2                  	= 10,                /**< GPADC channel 10 */
	CSK_GPADC_CVD3                		= 11,				/**< GPADC channel 11 */
	CSK_GPADC_CVD4                  	= 12,                /**< GPADC channel 12 */
	CSK_GPADC_CVD5                		= 13,				/**< GPADC channel 13 */
} GPADC_CHANNEL_TYPE;


typedef enum {
	CSK_GPADC_FIFO_EMPTY                  = 0,
	CSK_GPADC_FIFO_FULL                  = 1,
	CSK_GPADC_FIFO_THD                  = 2,
}GPADC_FIFOINT_TYPE;

/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup GPADC_Exported_Constants GPADC Exported Constants
  * @{
  */
#define CSK_GPADC_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1, 0)
#define CSK_GPADC_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#ifndef NULL
#define NULL                        (void *)0
#endif

#define CSK_GPADC_CHANNEL_NUM					  0x08

/*---------------------Control mode for application---------------------------------*/
/*----- GPADC Trigger mode : hardware or software or both -----*/
#define CSK_GPADC_TRIGGER_CONTROL_Pos             0
<<<<<<< HEAD
#define CSK_GPADC_TRIGGER_CONTROL_Msk             (0x01UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_HARDWARE_Enable         (0x01UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
=======
#define CSK_GPADC_TRIGGER_CONTROL_Msk             (0x03UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_HARDWARE_Enable         (0x01UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
#define CSK_GPADC_TRIGGER_GPT_Enable              (0x02UL << CSK_GPADC_TRIGGER_CONTROL_Pos)
>>>>>>> feature/zephyr

/*----- GPADC Sample time : 4/8/16/32 adc clocks -----*/
#define CSK_GPADC_SAMPLETIME_Pos             	  2
#define CSK_GPADC_SAMPLETIME_Msk             	  (0x3UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_4                	  (0x00UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_8                	  (0x01UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_16					  (0x02UL << CSK_GPADC_SAMPLETIME_Pos)
#define CSK_GPADC_SAMPLETIME_32					  (0x03UL << CSK_GPADC_SAMPLETIME_Pos)

/*----- GPADC Clock frequency : 24M/12M/6M/3M -----*/
#define CSK_GPADC_CLKFREQ_Pos             	  	  4
#define CSK_GPADC_CLKFREQ_Msk             	  	  (0x3UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_24M					  (0x00UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_12M					  (0x01UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_6M					  (0x02UL << CSK_GPADC_CLKFREQ_Pos)
#define CSK_GPADC_CLKFREQ_3M					  (0x03UL << CSK_GPADC_CLKFREQ_Pos)

/*----- GPADC Channel select : 0 / 1 / 2 / 3 / (1/6VDDIO) / (1/8VCC) / keysense / temp-sensor -----*/
#define CSK_GPADC_CHANNEL_SEL_Pos             	  6
#define CSK_GPADC_CHANNEL_SEL_Msk             	  (0x3FFFUL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_0					  (0x01UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_1					  (0x02UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_2					  (0x04UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_3					  (0x08UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_VDDIO				  (0x10UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_VCC				  (0x20UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_KEYSENSE			  (0x40UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_TEMP				  (0x80UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD0				  (0x100UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD1				  (0x200UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD2				  (0x400UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD3				  (0x800UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD4				  (0x1000UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_CVD5				  (0x2000UL << CSK_GPADC_CHANNEL_SEL_Pos)
#define CSK_GPADC_CHANNEL_SEL_ALL				  (0xFFUL << CSK_GPADC_CHANNEL_SEL_Pos)

/*----- GPADC input mode : single end / differential -----*/
#define CSK_GPADC_INPUT_MODE_Pos             	  20
#define CSK_GPADC_INPUT_MODE_Msk             	  (0x01UL << CSK_GPADC_INPUT_MODE_Pos)
#define CSK_GPADC_INPUT_MODE_Diff				  (0x00UL << CSK_GPADC_INPUT_MODE_Pos)
#define CSK_GPADC_INPUT_MODE_Single				  (0x01UL << CSK_GPADC_INPUT_MODE_Pos)

/*----- GPADC vref sel : 1.25 internal vref / 0.5VDDIO /external -----*/
#define CSK_GPADC_VREF_SEL_Pos             	  	  21
#define CSK_GPADC_VREF_SEL_Msk             	  	  (0x07UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_1_25V				  (0x07UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_VDDIO				  (0x05UL << CSK_GPADC_VREF_SEL_Pos)
#define CSK_GPADC_VREF_SEL_EXT				  	  (0x02UL << CSK_GPADC_VREF_SEL_Pos)

/*----- GPADC data transfer mode : dma mode enable/disable -----*/
#define CSK_GPADC_DMA_ENABLE_Pos             	  24
#define CSK_GPADC_DMA_ENABLE_Msk             	  (0x01UL << CSK_GPADC_DMA_ENABLE_Pos)
#define CSK_GPADC_DMA_ENABLE             	  	  (0x01UL << CSK_GPADC_DMA_ENABLE_Pos)
#define CSK_GPADC_DMA_DISABLE             	  	  (0x00UL << CSK_GPADC_DMA_ENABLE_Pos)

////CVD control

#define CSK_GPADC_GURADRING_ENABLE					1
#define CSK_GPADC_GURADRING_DISABLE					0

#define CSK_GPADC_DOUBLE_SAPMLE_ENABLE				1
#define CSK_GPADC_DOUBLE_SAPMLE_DISABLE				0

#define CSK_GPADC_CHARGE_INV_ENABLE					1
#define CSK_GPADC_CHARGE_INV_DISABLE				0


/*----- CVD prechargeA timer -----*/
#define CSK_GPADC_PRECHARGEA_CONTROL_Pos             0
#define CSK_GPADC_PRECHARGEA_CONTROL_Msk             (0xFFUL << CSK_GPADC_PRECHARGEA_CONTROL_Pos)

/*----- CVD prechargeB timer -----*/
#define CSK_GPADC_PRECHARGEB_CONTROL_Pos             8
#define CSK_GPADC_PRECHARGEB_CONTROL_Msk             (0xFFUL << CSK_GPADC_PRECHARGEB_CONTROL_Pos)

/*----- CVD Acquicition timer -----*/
#define CSK_GPADC_ACQUICITION_CONTROL_Pos            16
#define CSK_GPADC_ACQUICITION_CONTROL_Msk            (0xFFUL << CSK_GPADC_ACQUICITION_CONTROL_Pos)

/*----- CVD Additional Capacitor -----*/
#define CSK_GPADC_ADDITIONAL_CAP_CONTROL_Pos         24
#define CSK_GPADC_ADDITIONAL_CAP_CONTROL_Msk         (0x07UL << CSK_GPADC_ADDITIONAL_CAP_CONTROL_Pos)




/****** GPADC Event *****/
//#define CSK_GPADC_FIFO_EMPTY    					(0x01)
//#define CSK_GPADC_FIFO_FULL  						(0x02)
//#define CSK_GPADC_FIFO_THD  						(0x03)
#define CSK_GPADC_COMPLETE  						(0x04)
#define CSK_GPADC_EOC_ERROR  						(0x05)


/**
  * @}
  */




/* Exported macro ------------------------------------------------------------*/
/** @defgroup GPADC_Exported_Macro GPADC Exported Macro
  * @{
  */

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup GPADC_Exported_Functions GPADC Exported Functions
  * @{
  */

/** @addtogroup GPADC_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
void* GPADC(void);
int32_t HAL_GPADC_Initialize(void *res);
int32_t HAL_GPADC_Uninitialize(void *res);
int32_t HAL_GPADC_Control(void* res, uint32_t control);
/**
  * @}
  */

/** @addtogroup GPADC_Exported_Functions_Group2 control functions
  * @{
  */
uint32_t HAL_GPADC_Start(void* res);
uint32_t HAL_GPADC_Stop(void* res);
uint32_t HAL_GPADC_PollForConversion(void* res, uint32_t Timeout);
uint32_t HAL_GPADC_Start_IT(void* res);
uint32_t HAL_GPADC_Stop_IT(void* res);
uint16_t HAL_GPADC_GetValue(void* res, uint32_t channelnum);
uint32_t HAL_GPADC_SetTriggerNum(void* res, uint32_t trignum);
uint32_t HAL_GPADC_SetSampleWaitTime(void* res, uint32_t waitTime);
uint32_t HAL_GPADC_SetSetupWaitTime(void* res, uint8_t waitTime);
uint32_t HAL_GPADC_SetFifoThd(void* res, uint8_t channelnum, uint8_t threshold);
uint32_t HAL_GPADC_EnableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);
uint32_t HAL_GPADC_DisableFifoInterrupt(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);
uint32_t HAL_GPADC_ClearFifoIntStatus(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);
uint32_t HAL_GPADC_GetFifoIntStatus(void* res, uint8_t channelnum, GPADC_FIFOINT_TYPE type);
uint32_t HAL_GPADC_GetFifoCnt(void* res, uint8_t channelnum);

int32_t HAL_GPADC_RegisterChannelCallback(void *res, GPADC_CHANNEL_TYPE channel, CSK_GPADC_SignalEvent_t cb_event);
int32_t HAL_GPADC_RegisterCompleteCallback(void *res, CSK_GPADC_SignalEvent_t cb_event);

uint32_t HAL_GPADC_CVD_Config(void* res, uint32_t gr_state, uint32_t db_sample_state, uint32_t second_charge_invert);
uint32_t HAL_GPADC_CVD_Control(void* res, uint32_t control);
uint32_t HAL_GPADC_CVD_GetValue(void* res, uint32_t channelnum);
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



#ifdef __cplusplus
}
#endif

#endif /* __DRIVER_GPADC_H */
