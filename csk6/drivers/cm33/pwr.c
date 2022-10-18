/**
  ******************************************************************************
  * @file    pwr.c
  * @author  ListenAI Application Team
  * @brief   PWR HAL module driver.
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
#include "pwr.h"
#include "cmsis_gcc.h"

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @defgroup PWR PWR
  * @brief PWR HAL module driver
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CSK_PWR_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#define CHECK_RESOURCES(res)  do{\
        if(res != &pwr_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)


/* Private function prototypes -----------------------------------------------*/
static void PWR0_IRQ_Handler(void);

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const CSK_DRIVER_VERSION pwr_driver_version = {
        CSK_PWR_API_VERSION,
        CSK_PWR_DRV_VERSION
};

PWR_INFO info_pwr = {
        0,
        (void*)0,
		{0},
		(void*)0,
};

static const PWR_RESOURCES pwr_resources = {
		VENUS_PWR,
		IRQ_AON_WAKEUP_VECTOR,
		PWR0_IRQ_Handler,
        &info_pwr,
};

/* Private functions ---------------------------------------------------------*/
static void
pwr_irq_handler(PWR_RESOURCES* hpwr){

	uint32_t intflag, modeflag;

	intflag = hpwr->reg->REG_AON_STATUS_3.bit.IRQ_AON_WAKEUP;

	if(intflag == 1){
		hpwr->info->cb_event((void*)hpwr->user_param);
	}
}

static void
PWR0_IRQ_Handler(void)
{
	pwr_irq_handler(PWR());
}



/** @defgroup PWR_Exported_Functions PWR Exported Functions
  * @{
  */

/** @defgroup PWR_Exported_Functions_Group1 Initialization and de-initialization functions
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
void* PWR(void){
    return (void*)&pwr_resources;
}

/**
  * @brief add description.
  * @note add description
  * @retval CSK_DRIVER_VERSION
  */
CSK_DRIVER_VERSION
HAL_PWR_GetVersion(void){
    return pwr_driver_version;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_Initialize(void *res){

    CHECK_RESOURCES(res);

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	//add variable initialize
	pPwr->info->flags |= (PWR_FLAG_INITIALIZED);

    return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_Uninitialize(void *res){
    CHECK_RESOURCES(res);

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	pPwr->info->flags &= ~(PWR_FLAG_INITIALIZED);

    return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t HAL_PWR_PowerControl(void *res, CSK_POWER_STATE state)
{
	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;
	uint32_t rddat;

	switch(state){
	case CSK_POWER_OFF:
	    pPwr->info->flags &= ~(PWR_FLAG_POWERED);

	    // disable interrupt
		disable_IRQ(pPwr->irq_num);

		// register original interrupt handle and interrupt number
	    register_ISR(pPwr->irq_num, NULL, NULL);

		break;
	case CSK_POWER_LOW:
		break;
	case CSK_POWER_FULL:
		if((pPwr->info->flags&PWR_FLAG_INITIALIZED) == 0)
		{
			return CSK_DRIVER_ERROR;
		}
		if((pPwr->info->flags&PWR_FLAG_POWERED) == 0)
		{

            // Maybe enable global peripheral interrupt, register interrupt handle and interrupt number
			register_ISR(pPwr->irq_num, pPwr->irq_handler, NULL);
	        enable_IRQ(pPwr->irq_num);

	        pPwr->info->flags |= (PWR_FLAG_POWERED);
		}
		else
		{
			//gpt has been powered before
			return CSK_DRIVER_OK;
		}
		break;
	default:
		return CSK_DRIVER_ERROR_UNSUPPORTED;
	}

	return CSK_DRIVER_OK;
}



/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_Control(void* res, uint32_t control){
    CHECK_RESOURCES(res);

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;
	uint32_t tmp, regValue=0;

    if ((pPwr->info->flags & PWR_FLAG_POWERED) == 0U) {
        // UART not powered
        return CSK_DRIVER_ERROR;
    }

    pPwr->reg->REG_PMU_CTRL1.all &= ~(CSK_PWR_REG_WAKE_SELECT_Msk);
    tmp = (control & CSK_PWR_WAKE_SELECT_Msk) >> CSK_PWR_WAKE_SELECT_Pos;
    pPwr->reg->REG_PMU_CTRL1.all |= (tmp << CSK_PWR_WAKE_SELECT_Pos);

    pPwr->reg->REG_PMU_CTRL1.all &= ~(CSK_PWR_REG_GPIOB_POLARITY_Msk);
    tmp = (control & CSK_PWR_WAKE_GPIOB_POLARITY_Msk) >> CSK_PWR_WAKE_GPIOB_POLARITY_Pos;
    pPwr->reg->REG_PMU_CTRL1.all |= (tmp << CSK_PWR_WAKE_GPIOB_POLARITY_Pos);

    return CSK_DRIVER_OK;
}

/**
  * @}
  */



/** @defgroup PWR_Exported_Functions_Group2 control functions
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
HAL_PWR_InterruptEnable(void* res){

    CHECK_RESOURCES(res);
	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	pPwr->reg->REG_AON_STATUS_2.bit.ENA_IRQ_AON_WAKEUP = ENABLE;

    return CSK_DRIVER_OK;
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_InterruptDisable(void* res){
    CHECK_RESOURCES(res);
	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	pPwr->reg->REG_AON_STATUS_2.bit.ENA_IRQ_AON_WAKEUP = DISABLE;

    return CSK_DRIVER_OK;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_GetSysResetCause(void* res){

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;
	uint32_t rstCause;
	rstCause = (pPwr->reg->REG_AON_STATUS_3.all & CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE) >> CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE_Pos;

	return rstCause;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_GetWakeCause(void* res){

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;
	uint32_t wakeCause;
	wakeCause = (pPwr->reg->REG_AON_STATUS_3.all & CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSE) >> CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSE_Pos;

	return wakeCause;
}

/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_GetBootOption(void* res){

	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;
	uint32_t bootOption;
	bootOption = (pPwr->reg->REG_AON_STATUS_3.all & CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION) >> CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION_Pos;

	return bootOption;
}



/**
  * @brief add description.
  * @note add description: WakeUpPinx refer to CSK_PWR_WAKE_SELECT_GPIOB_0
  * @retval int32_t
  */
void HAL_PWR_EnableWakeUpPin(void* res, uint32_t WakeUpPinx)
{
//  /* Check the parameter */
//  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinx));
	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	/* Enable the wake up pin */
	pPwr->reg->REG_PMU_CTRL1.all |= (WakeUpPinx << CSK_PWR_REG_GPIOB_WAKEUP_Pos);

}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
void HAL_PWR_DisableWakeUpPin(void* res, uint32_t WakeUpPinx)
{
  /* Todu:Check the parameter */
//  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinx));

	PWR_RESOURCES *pPwr = (PWR_RESOURCES *)res;
	/* Disable the wake up pin */
	pPwr->reg->REG_PMU_CTRL2.all &= ~(WakeUpPinx << CSK_PWR_REG_GPIOB_WAKEUP_Pos);

}


/**
* @brief add description.
* @note add description
* @retval int32_t
*/
void HAL_PWR_EnterHoldMode(void* res, uint8_t SLEEPEntry)
{
  /* Todu:Check the parameters */

  /* Clear SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* Select SLEEP mode entry -------------------------------------------------*/
  if(SLEEPEntry == PWR_HOLDENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
}


/**
* @brief add description.
* @note add description
* @retval int32_t
*/
void HAL_PWR_EnterLightSleepMode(void* res, uint8_t STOPEntry)
{
  /* Todu:Check the parameters */
  PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

  pPwr->reg->REG_PMU_CTRL2.bit.MASK_CP_ENTER_SLEEP = 1;

  pPwr->reg->REG_PMU_CTRL2.bit.ENA_DEEPSLEEP = 0;
  pPwr->reg->REG_PMU_CTRL2.bit.ENA_LITESLEEP = 1;

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* Select Stop mode entry --------------------------------------------------*/
  if(STOPEntry == PWR_LIGHTSLEEPENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}


/**
* @brief add description.
* @note add description
* @retval int32_t
*/
void HAL_PWR_EnterDeepSleepMode(void* res, uint32_t CPMask)
{
  PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

  if(CPMask == 1){
	  pPwr->reg->REG_PMU_CTRL2.bit.MASK_CP_ENTER_SLEEP = 1;
  }else{
	  pPwr->reg->REG_PMU_CTRL2.bit.MASK_CP_ENTER_SLEEP = 0;
  }

  pPwr->reg->REG_PMU_CTRL2.bit.ENA_LITESLEEP = 0;
  pPwr->reg->REG_PMU_CTRL2.bit.ENA_DEEPSLEEP = 1;

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  /* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM)
  __force_stores();
#endif
  /* Request Wait For Interrupt */
  __WFI();
}


/**
  * @brief Indicates Sleep-On-Exit when returning from Handler mode to Thread mode.
  * @note Set SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  *       Setting this bit is useful when the processor is expected to run only on
  *       interruptions handling.
  * @retval None
  */
void HAL_PWR_EnableSleepOnExit(void)
{
  /* Set SLEEPONEXIT bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}


/**
  * @brief Disables Sleep-On-Exit feature when returning from Handler mode to Thread mode.
  * @note Clears SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  * @retval None
  */
void HAL_PWR_DisableSleepOnExit(void)
{
  /* Clear SLEEPONEXIT bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}


/**
  * @brief Enables CORTEX M33 SEVONPEND bit.
  * @note Sets SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_EnableSEVOnPend(void)
{
  /* Set SEVONPEND bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}


/**
  * @brief Disables CORTEX M33 SEVONPEND bit.
  * @note Clears SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_DisableSEVOnPend(void)
{
  /* Clear SEVONPEND bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}


/**
  * @brief add description.
  * @note add description
  * @retval int32_t
  */
int32_t
HAL_PWR_RegisterCallback(void *res, CSK_PWR_SignalEvent_t cb_event){

    CHECK_RESOURCES(res);
	PWR_RESOURCES * pPwr = (PWR_RESOURCES *)res;

	pPwr->info->cb_event = cb_event;

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
