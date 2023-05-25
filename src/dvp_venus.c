/*
 * dvp_venus.c
 *
 * Created on: May 13, 2021
 *
 */

#include "Driver_Common.h"
#include "dvp_venus.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define DEBUG_LOG   0
#if DEBUG_LOG
#define LOGD(format, ...)   CLOGD(format, ##__VA_ARGS__)
#else
#define LOGD(format, ...)   ((void)0)
#endif // DEBUG_LOG

#define CSK_DVP_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)
#define SYSCTRL_CFG    ((SYSCFG_RegDef*) CMN_SYSCTRL_BASE)

// driver version
// static const
// CSK_DRIVER_VERSION dvp_driver_version = { CSK_DVP_API_VERSION, CSK_DVP_DRV_VERSION };

//------------------------------------------------------------------------------------------


_FAST_DATA_VI static DVP_HandleTypeDef dvp_dev = {
		((DVP_TypeDef *)(CMN_DVP_BASE)),
		{0},
		NULL,
		0,
		0,
};


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DVP_TIMEOUT_STOP               ((uint32_t)1000) /* Set timeout to 1s  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  Handles DVP interrupt request.
  * @retval None
  */
_FAST_FUNC_RO void DVP_IRQ_Handler()
{
	uint32_t status = dvp_dev.Instance->STATUS;
	/* Clear the corresponding interrupt flags */
	dvp_dev.Instance->INTR_CLR = status;

 	/* Frame complete interrupt management */
	if((status & CSK_DVP_STATUS_FRM_CMPLT) == CSK_DVP_STATUS_FRM_CMPLT)
	{
		/* Reset the JLB status while it comes along with the frame complete flag  */
		status &= ~(CSK_DVP_STATUS_JLB0_FULL | CSK_DVP_STATUS_JLB1_FULL);

		if(dvp_dev.cb_event)
			dvp_dev.cb_event(CSK_DVP_STATUS_FRM_CMPLT, 0);

		dvp_dev.Instance->JLB0_HANSHK_U0 = 1;
		dvp_dev.Instance->JLB0_HANSHK_U1 = 1;
		dvp_dev.Instance->JLB1_HANSHK_U0 = 1;
		dvp_dev.Instance->JLB1_HANSHK_U1 = 1;
	}

	/* Overflow interrupt management */
	if((status & CSK_DVP_STATUS_FIFO_OVF) == CSK_DVP_STATUS_FIFO_OVF)
	{
		/* Reset the JLB status while it comes along with the fifo overflow flag  */
#if 1
		if(status & CSK_DVP_STATUS_JLB0_FULL)
		{
			status &= ~(CSK_DVP_STATUS_JLB0_FULL);
			dvp_dev.Instance->JLB0_HANSHK_U0 = 1;
			dvp_dev.Instance->JLB0_HANSHK_U1 = 1;
		}
		if(status & CSK_DVP_STATUS_JLB1_FULL)
		{
			status &= ~(CSK_DVP_STATUS_JLB1_FULL);
			dvp_dev.Instance->JLB1_HANSHK_U0 = 1;
			dvp_dev.Instance->JLB1_HANSHK_U1 = 1;
		}
#else
		status &= ~(CSK_DVP_STATUS_JLB0_FULL | CSK_DVP_STATUS_JLB1_FULL);
#endif

		/* Update error code */
		dvp_dev.ErrorCode |= DVP_ERROR_OVR;

		/* Change the state */
		dvp_dev.State = DVP_STATE_ERROR;

		if(dvp_dev.cb_event)
			dvp_dev.cb_event(CSK_DVP_STATUS_FIFO_OVF, 0);
	}

	/* JLB0 full interrupt management */
	if((status & CSK_DVP_STATUS_JLB0_FULL) == CSK_DVP_STATUS_JLB0_FULL)
	{
		if((dvp_dev.Init.InputFormat & CSK_DVP_INPUT_FORM_LUMINA) == CSK_DVP_INPUT_FORM_LUMINA)
		{
			if(dvp_dev.Init.InputFormat == CSK_DVP_INPUT_FORM_LUMINA_8BIT)
				dvp_dev.Instance->Y_ADDR0 += (dvp_dev.Init.FrameWidth * 16);
			else /* CSK_DVP_INPUT_FORM_LUMINA_10BIT or CSK_DVP_INPUT_FORM_LUMINA_12BIT */
				dvp_dev.Instance->Y_ADDR0 += (dvp_dev.Init.FrameWidth * 32);
		}
		else if((dvp_dev.Init.InputFormat & CSK_DVP_INPUT_FORM_YUV420) == CSK_DVP_INPUT_FORM_YUV420)
		{
			dvp_dev.Instance->Y_ADDR0 += (dvp_dev.Init.FrameWidth * 16);
			dvp_dev.Instance->CB_ADDR0 += (dvp_dev.Init.FrameWidth * 4);
			dvp_dev.Instance->CR_ADDR0 += (dvp_dev.Init.FrameWidth * 4);
		}
		else /* CSK_DVP_INPUT_FORM_YUV422 */
		{
			dvp_dev.Instance->Y_ADDR0 += (dvp_dev.Init.FrameWidth * 16);
			dvp_dev.Instance->CB_ADDR0 += (dvp_dev.Init.FrameWidth * 8);
			dvp_dev.Instance->CR_ADDR0 += (dvp_dev.Init.FrameWidth * 8);
		}

		if((dvp_dev.Instance->Y_ADDR0 > dvp_dev.Init.MaxAddressY) \
				|| (dvp_dev.Instance->CB_ADDR0 > dvp_dev.Init.MaxAddressCb) \
				|| (dvp_dev.Instance->CR_ADDR0 > dvp_dev.Init.MaxAddressCr) )
		{
			/* Update error code */
			dvp_dev.ErrorCode |= DVP_ERROR_MEM_RANGE;
		}
		else
		{
			dvp_dev.Instance->JLB0_HANSHK_U0 = 1;
			dvp_dev.Instance->JLB0_HANSHK_U1 = 1;
		}

		if(dvp_dev.cb_event)
			dvp_dev.cb_event(CSK_DVP_STATUS_JLB0_FULL, 0);
	}

	/* JLB1 full interrupt management */
	if((status & CSK_DVP_STATUS_JLB1_FULL) == CSK_DVP_STATUS_JLB1_FULL)
	{
		if((dvp_dev.Init.InputFormat & CSK_DVP_INPUT_FORM_LUMINA) == CSK_DVP_INPUT_FORM_LUMINA)
		{
			if(dvp_dev.Init.InputFormat == CSK_DVP_INPUT_FORM_LUMINA_8BIT)
				dvp_dev.Instance->Y_ADDR1 += (dvp_dev.Init.FrameWidth * 16);
			else /* CSK_DVP_INPUT_FORM_LUMINA_10BIT or CSK_DVP_INPUT_FORM_LUMINA_12BIT */
				dvp_dev.Instance->Y_ADDR1 += (dvp_dev.Init.FrameWidth * 32);
		}
		else if((dvp_dev.Init.InputFormat & CSK_DVP_INPUT_FORM_YUV420) == CSK_DVP_INPUT_FORM_YUV420)
		{
			dvp_dev.Instance->Y_ADDR1 += (dvp_dev.Init.FrameWidth * 16);
			dvp_dev.Instance->CB_ADDR1 += (dvp_dev.Init.FrameWidth * 4);
			dvp_dev.Instance->CR_ADDR1 += (dvp_dev.Init.FrameWidth * 4);
		}
		else /* CSK_DVP_INPUT_FORM_YUV422 */
		{
			dvp_dev.Instance->Y_ADDR1 += (dvp_dev.Init.FrameWidth * 16);
			dvp_dev.Instance->CB_ADDR1 += (dvp_dev.Init.FrameWidth * 8);
			dvp_dev.Instance->CR_ADDR1 += (dvp_dev.Init.FrameWidth * 8);
		}

		if((dvp_dev.Instance->Y_ADDR1 > dvp_dev.Init.MaxAddressY) \
				|| (dvp_dev.Instance->CB_ADDR1 > dvp_dev.Init.MaxAddressCb) \
				|| (dvp_dev.Instance->CR_ADDR1 > dvp_dev.Init.MaxAddressCr) )
		{
			/* Update error code */
			dvp_dev.ErrorCode |= DVP_ERROR_MEM_RANGE;
		}
		else
		{
			dvp_dev.Instance->JLB1_HANSHK_U0 = 1;
			dvp_dev.Instance->JLB1_HANSHK_U1 = 1;
		}

		if(dvp_dev.cb_event)
			dvp_dev.cb_event(CSK_DVP_STATUS_JLB1_FULL, 0);
	}

	/* Error interrupt management */
	if((status & CSK_DVP_STATUS_AHB_ERR) == CSK_DVP_STATUS_AHB_ERR)
	{
		/* Update error code */
		dvp_dev.ErrorCode |= DVP_ERROR_AHB;

		/* Change the state */
		dvp_dev.State = DVP_STATE_ERROR;

		if(dvp_dev.cb_event)
			dvp_dev.cb_event(CSK_DVP_STATUS_AHB_ERR, 0);
	}
}


/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Return DVP instance.
  * @retval Instance of DVP
  */
void* DVP_Instance()
{
	return &dvp_dev;
}

/**
  * @brief  Initializes the DVP according to the specified
  *         parameters in the DVP_InitTypeDef and create the associated handle.
  * @param  dvp_dev: pointer to a DVP_HandleTypeDef structure that contains
  *                the configuration information.
  * @param  callback: pointer to the callback function
  * @retval execution status
  */
int32_t DVP_Init(DVP_HandleTypeDef *dvp_dev, void *callback)
{
	/* Check the DVP instance */
	if(dvp_dev == NULL)
	{
		return CSK_DRIVER_ERROR_PARAMETER;
	}

	/* Check if a valid width - multiplier of 16 */
    if ((dvp_dev->Init.FrameWidth % 16) != 0 || dvp_dev->Init.FrameWidth == 0)
    {
        return CSK_DRIVER_ERROR_PARAMETER;
    }

	/* Check if a valid height - multiplier of 8 */
    if ((dvp_dev->Init.FrameHeight % 8) != 0 || dvp_dev->Init.FrameHeight == 0)
    {
        return CSK_DRIVER_ERROR_PARAMETER;
    }

	/* Disable VIC first before configuration */
	dvp_dev->Instance->VI_EN = 0;

	if(dvp_dev->State == DVP_STATE_RESET)
	{
		/* Init the low level hardware and interrupt */
		// register_ISR(IRQ_DVP_VECTOR, (ISR)DVP_IRQ_Handler, NULL);
		// enable_IRQ(IRQ_DVP_VECTOR);
	}

	/* Change the DVP state */
	dvp_dev->State = DVP_STATE_BUSY;

	/* Clear the VIC state */
	dvp_dev->Instance->JLB0_HANSHK_U0 = 1;
	dvp_dev->Instance->JLB1_HANSHK_U0 = 1;
	dvp_dev->Instance->JLB0_HANSHK_U1 = 1;
	dvp_dev->Instance->JLB1_HANSHK_U1 = 1;

	/* Enable users */
	dvp_dev->Instance->JLB_HANSHK_SEL = CSK_DVP_JLB_HANSHK_SEL_USER0 | CSK_DVP_JLB_HANSHK_SEL_USER1;

	/* Configure the VIC input format */
	dvp_dev->Instance->INPUT_FORM = (uint32_t)(dvp_dev->Init.InputFormat);

	/* Configure the VIC frame */
	dvp_dev->Instance->F_HOR = (uint32_t)(dvp_dev->Init.FrameWidth - 1);
	dvp_dev->Instance->F_VER = (uint32_t)(dvp_dev->Init.FrameHeight - 1);

	/* Configure the VIC polarity */
	dvp_dev->Instance->POL_CNTL = (uint32_t)(dvp_dev->Init.PCKPolarity | dvp_dev->Init.VSPolarity |\
			dvp_dev->Init.HSPolarity);

	/* Configure the VIC input offset */
	dvp_dev->Instance->P_OFFSET = (uint32_t)(dvp_dev->Init.PixelOffset);
	dvp_dev->Instance->L_OFFSET = (uint32_t)(dvp_dev->Init.LineOffset);

	/* Configure the data alignment */
	dvp_dev->Instance->DATA_BUS_SEL = (uint32_t)(dvp_dev->Init.DataAlign);

	/* Configure the VIC handshaking mode */
	/* Currently only support software mode */
	dvp_dev->Instance->JLB_HANSHK_SEL |= CSK_DVP_JLB_HANSHK_SEL_MODE_SOFT;//(uint32_t)(dvp_dev->Init.HandshakingMode);

	/* Clear the FIFO overflow, AHB error, JLB full and frame complete status */
	dvp_dev->Instance->INTR_CLR = CSK_DVP_INTR_CLR_FIFO_OVF | CSK_DVP_INTR_CLR_AHB_ERR |\
			CSK_DVP_INTR_CLR_FRM_CMPLT | CSK_DVP_INTR_CLR_JLB0_FULL | CSK_DVP_INTR_CLR_JLB1_FULL;

	/* Enable the FIFO overflow, AHB error, JLB full and frame complete interrupts */
	dvp_dev->Instance->INTR_MSK &= ~(CSK_DVP_INTR_MSK_FIFO_OVF | CSK_DVP_INTR_MSK_AHB_ERR |\
			CSK_DVP_INTR_MSK_JLB0_FULL | CSK_DVP_INTR_MSK_JLB1_FULL | CSK_DVP_INTR_MSK_FRM_CMPLT);

	/* Set up callback */
	dvp_dev->cb_event = (CSK_DVP_SignalEvent_t)callback;

	/* Update error code */
	dvp_dev->ErrorCode = DVP_ERROR_NONE;

	/* Initialize the DVP state*/
	dvp_dev->State  = DVP_STATE_READY;

	return CSK_DRIVER_OK;
}

/**
  * @brief  Initializes the DVP according to the specified
  *         parameters in the DVP_InitTypeDef and create the associated handle.
  * @param  dvp_dev: pointer to a DVP_HandleTypeDef structure that contains
  *                the configuration information.
  * @retval execution status
  */
int32_t DVP_DeInit(DVP_HandleTypeDef *dvp_dev)
{
	/* Disable the VIC */
	dvp_dev->Instance->VI_EN = 0;
	// disable_IRQ(IRQ_DVP_VECTOR);
	dvp_dev->ErrorCode = DVP_ERROR_NONE;
	dvp_dev->State  = DVP_STATE_RESET;

	return CSK_DRIVER_OK;
}


/**
 *  @brief  IO operation functions
 *
 */
int32_t DVP_Start(DVP_HandleTypeDef* dvp_dev, DVP_BufferDef frame_buf)
{
	if(dvp_dev->State != DVP_STATE_READY)
		return CSK_DRIVER_ERROR;

    // if ((frame_buf.pAddrY == NULL) || (frame_buf.pAddrCb == NULL) || (frame_buf.pAddrCr == NULL))
    // {
    // 	return CSK_DRIVER_ERROR;;
    // }

    /* Set the receive buffer */
	if((dvp_dev->Init.InputFormat & CSK_DVP_INPUT_FORM_LUMINA) == CSK_DVP_INPUT_FORM_LUMINA)
	{
		if(dvp_dev->Init.InputFormat == CSK_DVP_INPUT_FORM_LUMINA_8BIT)
		{
			dvp_dev->Instance->Y_ADDR0 = (uint32_t)frame_buf.pAddrY;
			dvp_dev->Instance->Y_ADDR1 = (uint32_t)(frame_buf.pAddrY + dvp_dev->Init.FrameWidth * 8);
			dvp_dev->Init.MaxAddressY = (uint32_t)frame_buf.pAddrY + frame_buf.bufLenY - dvp_dev->Init.FrameWidth * 8;
		}
		else
		{
			dvp_dev->Instance->Y_ADDR0 = (uint32_t)frame_buf.pAddrY;
			dvp_dev->Instance->Y_ADDR1 = (uint32_t)(frame_buf.pAddrY + dvp_dev->Init.FrameWidth * 16);
			dvp_dev->Init.MaxAddressY = (uint32_t)frame_buf.pAddrY + frame_buf.bufLenY - dvp_dev->Init.FrameWidth * 16;
		}

	    dvp_dev->Instance->CB_ADDR0 = (uint32_t)frame_buf.pAddrCb;
	    dvp_dev->Instance->CR_ADDR0 = (uint32_t)frame_buf.pAddrCr;
	    dvp_dev->Instance->CB_ADDR1 = (uint32_t)frame_buf.pAddrCb;
	    dvp_dev->Instance->CR_ADDR1 = (uint32_t)frame_buf.pAddrCr;

	    dvp_dev->Init.MaxAddressCb = (uint32_t)frame_buf.pAddrCb;
		dvp_dev->Init.MaxAddressCr = (uint32_t)frame_buf.pAddrCr;

	}
	else if((dvp_dev->Init.InputFormat & CSK_DVP_INPUT_FORM_YUV420) == CSK_DVP_INPUT_FORM_YUV420)
	{
	    dvp_dev->Instance->Y_ADDR0  = (uint32_t)frame_buf.pAddrY;
	    dvp_dev->Instance->CB_ADDR0 = (uint32_t)frame_buf.pAddrCb;
	    dvp_dev->Instance->CR_ADDR0 = (uint32_t)frame_buf.pAddrCr;
	    dvp_dev->Instance->Y_ADDR1  = (uint32_t)(frame_buf.pAddrY + dvp_dev->Init.FrameWidth * 8);
	    dvp_dev->Instance->CB_ADDR1 = (uint32_t)(frame_buf.pAddrCb + dvp_dev->Init.FrameWidth * 2);
	    dvp_dev->Instance->CR_ADDR1 = (uint32_t)(frame_buf.pAddrCr + dvp_dev->Init.FrameWidth * 2);

		dvp_dev->Init.MaxAddressY = (uint32_t)frame_buf.pAddrY + frame_buf.bufLenY - dvp_dev->Init.FrameWidth * 8;
		dvp_dev->Init.MaxAddressCb = (uint32_t)frame_buf.pAddrCb + frame_buf.bufLenCb - dvp_dev->Init.FrameWidth * 2;
		dvp_dev->Init.MaxAddressCr = (uint32_t)frame_buf.pAddrCr + frame_buf.bufLenCr - dvp_dev->Init.FrameWidth * 2;
	}
	else
	{
	    dvp_dev->Instance->Y_ADDR0  = (uint32_t)frame_buf.pAddrY;
	    dvp_dev->Instance->CB_ADDR0 = (uint32_t)frame_buf.pAddrCb;
	    dvp_dev->Instance->CR_ADDR0 = (uint32_t)frame_buf.pAddrCr;
	    dvp_dev->Instance->Y_ADDR1  = (uint32_t)(frame_buf.pAddrY + dvp_dev->Init.FrameWidth * 8);
	    dvp_dev->Instance->CB_ADDR1 = (uint32_t)(frame_buf.pAddrCb + dvp_dev->Init.FrameWidth * 4);
	    dvp_dev->Instance->CR_ADDR1 = (uint32_t)(frame_buf.pAddrCr + dvp_dev->Init.FrameWidth * 4);

		dvp_dev->Init.MaxAddressY = (uint32_t)frame_buf.pAddrY + frame_buf.bufLenY - dvp_dev->Init.FrameWidth * 8;
		dvp_dev->Init.MaxAddressCb = (uint32_t)frame_buf.pAddrCb + frame_buf.bufLenCb - dvp_dev->Init.FrameWidth * 4;
		dvp_dev->Init.MaxAddressCr = (uint32_t)frame_buf.pAddrCr + frame_buf.bufLenCr - dvp_dev->Init.FrameWidth * 4;
	}

	/* Lock the DVP peripheral state */
	dvp_dev->State = DVP_STATE_BUSY;

	/* Enable Capture */
	dvp_dev->Instance->VI_EN |= CSK_DVP_VI_EN;

	/* Return function status */
	return CSK_DRIVER_OK;
}

/**
  * @brief  Disable DVP capture
  * @param  dvp_dev pointer to a DVP_HandleTypeDef structure that contains
  *                the configuration information.
  * @retval None
  */
void DVP_Stop(DVP_HandleTypeDef* dvp_dev)
{
	/* Lock the DVP peripheral state */
	dvp_dev->State = DVP_STATE_BUSY;

	/* Disable VIC */
	dvp_dev->Instance->VI_EN &= ~(CSK_DVP_VI_EN);

	/* Update error code */
	dvp_dev->ErrorCode = DVP_ERROR_NONE;

	/* Change DVP state */
	dvp_dev->State = DVP_STATE_READY;
}

/**
 * @brief  Enable the DVP clock out at a specified frequency
 *
 */
void DVP_EnableClockout(DVP_HandleTypeDef *dvp_dev, uint8_t divider)
{
	/* Set the clock output frequency to hclk/2/(divider+1) */
	dvp_dev->Instance->PIXEL_CLK_DIV = divider;
	dvp_dev->Instance->PIXEL_CLK_EN = 1;
}

/**
 * @brief  Disable the DVP clock out
 *
 */
void DVP_DisableClockout(DVP_HandleTypeDef *dvp_dev)
{
	dvp_dev->Instance->PIXEL_CLK_EN = 0;
}

/**
 * @brief  Return the DVP state code
 *
 */
DVP_StateTypeDef DVP_GetState(DVP_HandleTypeDef *dvp_dev)
{
	return dvp_dev->State;
}

/**
 * @brief  Return the DVP error code
 *
 */
uint32_t DVP_GetError(DVP_HandleTypeDef *dvp_dev)
{
	return dvp_dev->ErrorCode;
}







