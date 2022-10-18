/**
  ******************************************************************************
  * @file    Driver_PWR.h
  * @author  ListenAI Application Team
  * @brief   Header file of PWR HAL module.
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
#ifndef __CSK_DRIVER_PWR_H
#define __CSK_DRIVER_PWR_H


#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "Driver_Common.h"
#include "venus_ap.h"

/** @addtogroup CSK_HAL_Driver
  * @{
  */

/** @addtogroup PWR
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup PWR_Exported_Types PWR Exported Types
  * @{
  */

/**
  * @brief  add description
  */
typedef void (*CSK_PWR_SignalEvent_t) (void* workspace);

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup PWR_Exported_Constants PWR Exported Constants
  * @{
  */
#define CSK_PWR_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)
#define CSK_PWR_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

#ifndef NULL
#define NULL                        (void *)0
#endif

/*---------------------Control mode for application---------------------------------*/
/** @defgroup PWR_HOLD_mode_entry PWR SLEEP mode entry
  * @{
  */
#define PWR_HOLDENTRY_WFI              ((uint8_t)0x01)
#define PWR_HOLDENTRY_WFE              ((uint8_t)0x02)
/**
  * @}
  */

/** @defgroup PWR_LIGHTSLEEP_mode_entry PWR STOP mode entry
  * @{
  */
#define PWR_LIGHTSLEEPENTRY_WFI               ((uint8_t)0x01)
#define PWR_LIGHTSLEEPENTRY_WFE               ((uint8_t)0x02)
/**
  * @}
  */



//wakeup source select
#define CSK_PWR_WAKE_SELECT_Pos             0
#define CSK_PWR_WAKE_SELECT_Msk				(0xFFF << CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_5			(0x01<<11<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_4			(0x01<<10<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_3			(0x01<<9<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_2			(0x01<<8<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_1			(0x01<<7<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_GPIOB_0			(0x01<<6<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_TIMER			(0x01<<5<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_KEY				(0x01<<4<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_CBTN			(0x01<<3<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_IWDT			(0x01<<2<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_VAD				(0x01<<1<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_WAKE_SELECT_RTC				(0x01<<0<<CSK_PWR_WAKE_SELECT_Pos)


#define CSK_PWR_WAKE_GPIOB_POLARITY_Pos     12
#define CSK_PWR_WAKE_GPIOB_POLARITY_Msk		(0x3F << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_0_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_0_POLARITY_HIGH	(0x1 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_1_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_1_POLARITY_HIGH	(0x2 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_2_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_2_POLARITY_HIGH	(0x4 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_3_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_3_POLARITY_HIGH	(0x8 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_4_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_4_POLARITY_HIGH	(0x10 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_5_POLARITY_LOW	(0x0 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)
#define CSK_PWR_WAKE_GPIOB_5_POLARITY_HIGH	(0x20 << CSK_PWR_WAKE_GPIOB_POLARITY_Pos)


/*----- PWR interrupt mode : -----*/

/**
  * @}
  */


/* Exported macro ------------------------------------------------------------*/
/** @defgroup PWR_Exported_Macro PWR Exported Macro
  * @{
  */

/**
  * @brief GPADC module reset signal enable.
  * @return None
  */
#define __HAL_PWR_GPADC_RESET_ENABLE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.GPADC_RESET = 0x1; \
} while(0)

/**
  * @brief GPADC module reset signal release.
  * @return None
  */
#define __HAL_PWR_GPADC_RESET_RELEASE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.GPADC_RESET = 0x0; \
} while(0)


/**
  * @brief CPDMA module reset signal enable.
  * @return None
  */
#define __HAL_PWR_CPDMA_RESET_ENABLE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.CP_DMA_RESET = 0x1; \
} while(0)

/**
  * @brief CPDMA module reset signal release.
  * @return None
  */
#define __HAL_PWR_CPDMA_RESET_RELEASE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.CP_DMA_RESET = 0x0; \
} while(0)

/**
  * @brief APDMA module reset signal enable.
  * @return None
  */
#define __HAL_PWR_APDMA_RESET_ENABLE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.AP_DMA_RESET = 0x1; \
} while(0)

/**
  * @brief APDMA module reset signal release.
  * @return None
  */
#define __HAL_PWR_APDMA_RESET_RELEASE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.AP_DMA_RESET = 0x0; \
} while(0)


/**
  * @brief FLASH CTRL module reset signal enable.
  * @return None
  */
#define __HAL_PWR_FLASH_CTRL_RESET_ENABLE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.FLASH_CTRL_RESET = 0x1; \
} while(0)

/**
  * @brief  FLASH CTRL module reset signal release.
  * @return None
  */
#define __HAL_PWR_FLASH_CTRL_RESET_RELEASE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.FLASH_CTRL_RESET = 0x0; \
} while(0)


/**
  * @brief PSRAM CTRL module reset signal enable.
  * @return None
  */
#define __HAL_PWR_PSRAM_CTRL_RESET_ENABLE()    \
do { \
	IP_SYSCTRL->REG_SW_RESET2.bit.PSRAM_CTRL_RESET = 0x1; \
} while(0)

/**
  * @brief  PSRAM CTRL module reset signal release.
  * @return None
  */
#define __HAL_PWR_PSRAM_CTRL_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.PSRAM_CTRL_RESET = 0x0; \
} while(0)


/**
  * @brief CP CODEC module reset signal enable.
  * @return None
  */
#define __HAL_PWR_CP_CODEC_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.CP_CODEC_RESET = 0x1; \
} while(0)

/**
  * @brief  CP CODEC module reset signal release.
  * @return None
  */
#define __HAL_PWR_CP_CODEC_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.CP_CODEC_RESET = 0x0; \
} while(0)


/**
  * @brief GPIO_A module reset signal enable.
  * @return None
  */
#define __HAL_PWR_GPIO_A_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPIO_A_RESET = 0x1; \
} while(0)

/**
  * @brief  GPIO_A module reset signal release.
  * @return None
  */
#define __HAL_PWR_GPIO_A_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPIO_A_RESET = 0x0; \
} while(0)


/**
  * @brief GPIO_B module reset signal enable.
  * @return None
  */
#define __HAL_PWR_GPIO_B_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPIO_B_RESET = 0x1; \
} while(0)

/**
  * @brief  GPIO_B module reset signal release.
  * @return None
  */
#define __HAL_PWR_GPIO_B_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPIO_B_RESET = 0x0; \
} while(0)


/**
  * @brief CRYPTO module reset signal enable.
  * @return None
  */
#define __HAL_PWR_CRYPTO_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.CRYPTO_RESET = 0x1; \
} while(0)

/**
  * @brief  CRYPTO module reset signal release.
  * @return None
  */
#define __HAL_PWR_CRYPTO_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.CRYPTO_RESET = 0x0; \
} while(0)


/**
  * @brief SDIO module reset signal enable.
  * @return None
  */
#define __HAL_PWR_SDIO_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SDIO_RESET = 0x1; \
} while(0)

/**
  * @brief  SDIO module reset signal release.
  * @return None
  */
#define __HAL_PWR_SDIO_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SDIO_RESET = 0x0; \
} while(0)

/**
  * @brief GPT module reset signal enable.
  * @return None
  */
#define __HAL_PWR_GPT_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPT_RESET = 0x1; \
} while(0)

/**
  * @brief  GPT module reset signal release.
  * @return None
  */
#define __HAL_PWR_GPT_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.GPT_RESET = 0x0; \
} while(0)

/**
  * @brief USBC module reset signal enable.
  * @return None
  */
#define __HAL_PWR_USBC_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.USBC_RESET = 0x1; \
} while(0)

/**
  * @brief  USBC module reset signal release.
  * @return None
  */
#define __HAL_PWR_USBC_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.USBC_RESET = 0x0; \
} while(0)


/**
  * @brief IR module reset signal enable.
  * @return None
  */
#define __HAL_PWR_IR_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.IR_RESET = 0x1; \
} while(0)

/**
  * @brief  IR module reset signal release.
  * @return None
  */
#define __HAL_PWR_IR_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.IR_RESET = 0x0; \
} while(0)


/**
  * @brief I2C1 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_I2C1_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.I2C1_RESET = 0x1; \
} while(0)

/**
  * @brief  I2C1 module reset signal release.
  * @return None
  */
#define __HAL_PWR_I2C1_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.I2C1_RESET = 0x0; \
} while(0)


/**
  * @brief I2C0 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_I2C0_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.I2C0_RESET = 0x1; \
} while(0)

/**
  * @brief  I2C0 module reset signal release.
  * @return None
  */
#define __HAL_PWR_I2C0_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.I2C0_RESET = 0x0; \
} while(0)


/**
  * @brief SPI1 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_SPI1_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SPI1_RESET = 0x1; \
} while(0)

/**
  * @brief  SPI1 module reset signal release.
  * @return None
  */
#define __HAL_PWR_SPI1_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SPI1_RESET = 0x0; \
} while(0)


/**
  * @brief SPI0 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_SPI0_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SPI0_RESET = 0x1; \
} while(0)

/**
  * @brief  SPI0 module reset signal release.
  * @return None
  */
#define __HAL_PWR_SPI0_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.SPI0_RESET = 0x0; \
} while(0)

/**
  * @brief UART3 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_UART3_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART3_RESET = 0x1; \
} while(0)

/**
  * @brief  UART3 module reset signal release.
  * @return None
  */
#define __HAL_PWR_UART3_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART3_RESET = 0x0; \
} while(0)

/**
  * @brief UART2 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_UART2_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART2_RESET = 0x1; \
} while(0)

/**
  * @brief  UART2 module reset signal release.
  * @return None
  */
#define __HAL_PWR_UART2_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART2_RESET = 0x0; \
} while(0)


/**
  * @brief UART1 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_UART1_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART1_RESET = 0x1; \
} while(0)

/**
  * @brief  UART1 module reset signal release.
  * @return None
  */
#define __HAL_PWR_UART1_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART1_RESET = 0x0; \
} while(0)


/**
  * @brief UART0 module reset signal enable.
  * @return None
  */
#define __HAL_PWR_UART0_RESET_ENABLE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART0_RESET = 0x1; \
} while(0)

/**
  * @brief  UART0 module reset signal release.
  * @return None
  */
#define __HAL_PWR_UART0_RESET_RELEASE()    \
do { \
    IP_SYSCTRL->REG_SW_RESET2.bit.UART0_RESET = 0x0; \
} while(0)


/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup PWR_Exported_Functions PWR Exported Functions
  * @{
  */

/** @addtogroup PWR_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
void* PWR(void);
int32_t HAL_PWR_Initialize(void *res);
int32_t HAL_PWR_Uninitialize(void *res);
int32_t HAL_PWR_PowerControl(void *res, CSK_POWER_STATE state);
int32_t HAL_PWR_Control(void* res, uint32_t control);
/**
  * @}
  */


/** @addtogroup PWR_Exported_Functions_Group2 Peripheral Control functions
  * @{
  */
CSK_DRIVER_VERSION HAL_PWR_GetVersion(void);
int32_t HAL_PWR_InterruptEnable(void* res);
int32_t HAL_PWR_InterruptDisable(void* res);

int32_t HAL_PWR_GetSysResetCause(void* res);
int32_t HAL_PWR_GetWakeCause(void* res);
int32_t HAL_PWR_GetBootOption(void* res);

int32_t HAL_PWR_RegisterCallback(void *res, CSK_PWR_SignalEvent_t cb_event);

/* WakeUp pins configuration */
void HAL_PWR_EnableWakeUpPin(void *res,uint32_t WakeUpPinx);
void HAL_PWR_DisableWakeUpPin(void *res,uint32_t WakeUpPinx);

/* Low Power modes entry */
void HAL_PWR_EnterHoldMode(void* res, uint8_t HOLDEntry);
void HAL_PWR_EnterLightSleepMode(void* res, uint8_t SLEEPEntry);
void HAL_PWR_EnterDeepSleepMode(void* res, uint32_t CPMask);

/* Cortex System Control functions  *******************************************/
void HAL_PWR_EnableSleepOnExit(void);
void HAL_PWR_DisableSleepOnExit(void);
void HAL_PWR_EnableSEVOnPend(void);
void HAL_PWR_DisableSEVOnPend(void);



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

#endif /* __CSK_DRIVER_PWR_H */
