/**
  ******************************************************************************
  * @file    Driver_KEYSENSE.h
  * @author  ListenAI Application Team
  * @brief   Header file of KEYSENSE HAL module.
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
#include "keysense.h"

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @defgroup KEYSENSE KEYSENSE
  * @brief KEYSENSE HAL module driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CSK_GPADC_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#define CHECK_RESOURCES(res)  do{\
        if(res != &keysense_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)


/* Private function prototypes -----------------------------------------------*/
static void KEYSENSE0_IRQ_Handler(void);



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const CSK_DRIVER_VERSION keysense_driver_version = {
        CSK_KEYSENSE_API_VERSION,
        CSK_GPADC_DRV_VERSION
};

KEYSENSE_INFO info_keysense = {
        0,
        {(void*)0},
		{0},
		(void*)0,
};

static const KEYSENSE_RESOURCES keysense_resources = {
		VENUS_KEYSENSE,
		IRQ_AON_KEYSENSE_VECTOR,
		KEYSENSE0_IRQ_Handler,
        &info_keysense,
};


/* Private functions ---------------------------------------------------------*/
static void
kensense_irq_handler(KEYSENSE_RESOURCES* hkeysense){

	uint32_t intflag, modeflag;

	intflag = hkeysense->reg->REG_KS_ISR.all;

	for(uint32_t cnt = 0; cnt < CSK_KEYSENSE_MODE_NUM; cnt++){
		modeflag = intflag & (0x01<<cnt);
		if(modeflag){
			hkeysense->reg->REG_KS_ICR.all = modeflag;
			hkeysense->info->cb_event[cnt]((void*)hkeysense->user_param);
		}
	}
}


static void
KEYSENSE0_IRQ_Handler(void)
{
	kensense_irq_handler(KEYSENSE());
}



/** @defgroup KEYSENSE_Exported_Functions KEYSENSE Exported Functions
  * @{
  */

/** @defgroup KEYSENSE_Exported_Functions_Group1 Initialization and de-initialization functions
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
void* KEYSENSE(void){
    return (void*)&keysense_resources;
}


/**
  * @brief add description.
  * @note add description
  * @retval CSK_DRIVER_VERSION
  */
CSK_DRIVER_VERSION
KEYSENSE_GetVersion(void){
    return keysense_driver_version;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_Initialize(void *res){

    CHECK_RESOURCES(res);

	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->info->flags |= KEYSENSE_FLAG_POWERED;

	pKeysense->reg->REG_KS_IMR.all = 0xF;
	pKeysense->reg->REG_KS_ICR.all = 0xF;

	//ANA_AON.ana_rsv: bit4 reserved need to be reset
    *(uint32_t*)(0x46810034) &= ~(1<<4);

    //clock config
    IP_PMUCTRL->REG_AON_CRM_CTRL1.bit.ENA_KEYSENSE_CLK = 1;
    IP_PMUCTRL->REG_AON_CRM_CTRL1.bit.SEL_KEYSENSE_CLK = 0;

    register_ISR(pKeysense->irq_num, pKeysense->irq_handler, NULL);

    enable_IRQ(pKeysense->irq_num);

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_Uninitialize(void *res){
    CHECK_RESOURCES(res);

	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

    disable_IRQ(pKeysense->irq_num);

    register_ISR(pKeysense->irq_num, NULL, NULL);

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_Control(void* res, uint32_t control){
    CHECK_RESOURCES(res);

	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;
	uint32_t tmp;

    if ((pKeysense->info->flags & KEYSENSE_FLAG_POWERED) == 0U) {
        // UART not powered
        return CSK_DRIVER_ERROR;
    }

    tmp = (control & CSK_KEYSENSE_ADC_TRIGGER_THD_Msk) >> CSK_KEYSENSE_ADC_TRIGGER_THD_Pos;
    pKeysense->reg->REG_KS_THD.bit.KS_THD_ADC_TRIG = tmp;

    tmp = (control & CSK_KEYSENSE_WAKEUP_CNT_THD_Msk) >> CSK_KEYSENSE_WAKEUP_CNT_THD_Pos;
    pKeysense->reg->REG_KS_THD.bit.KS_THD_WAKEUP = tmp;

    return CSK_DRIVER_OK;
}

/**
  * @}
  */


/** @defgroup KEYSENSE_Exported_Functions_Group2 control functions
  *  @brief    Interrupt enable/disable functions
  *
@verbatim
 ===============================================================================
              ##### Interrupt Enable and disable functions #####
 ===============================================================================
    [..]
		add description

@endverbatim
  * @{
  */

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_InterruptEnable(void* res, KEYSENSE_INTERRUPT_MODE_TYPE intmode){

    CHECK_RESOURCES(res);
	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->reg->REG_KS_IMR.all &= ~(intmode&0x0F);

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_InterruptDisable(void* res, KEYSENSE_INTERRUPT_MODE_TYPE intmode){
    CHECK_RESOURCES(res);
	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->reg->REG_KS_IMR.all |= (intmode&0x0F);

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_Enable(void* res){

    CHECK_RESOURCES(res);
	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->reg->REG_KS_CFG.bit.KS_EN = 1;

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_Disable(void* res){

    CHECK_RESOURCES(res);
	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->reg->REG_KS_CFG.bit.KS_EN = 0;

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_KEYSENSE_RegisterCallback(void *res, KEYSENSE_MODE_TYPE mode, CSK_KEYSENSE_SignalEvent_t cb_event){

    CHECK_RESOURCES(res);
	KEYSENSE_RESOURCES * pKeysense = (KEYSENSE_RESOURCES *)res;

	pKeysense->info->cb_event[mode] = cb_event;

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





