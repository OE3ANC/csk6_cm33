/*
 * Driver_DVP.h
 *
 * Created on: May 13, 2021
 *
 */

#ifndef __DRIVER_DVP_H
#define __DRIVER_DVP_H

#include <assert.h>
#include "Driver_Common.h"

#define CSK_DVP_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)                        /* API version */

/********************  Bits definition for DVP_F_HOR register  ************************/
#define CSK_DVP_F_HOR_Pos                   (0U)
#define CSK_DVP_F_HOR_Msk                   (0x1FFFUL << CSK_DVP_F_HOR_Pos)            /*!< 0x00001FFF */
#define CSK_DVP_F_HOR                       CSK_DVP_F_HOR_Msk

/********************  Bits definition for DVP_F_VER register  ************************/
#define CSK_DVP_F_VER_Pos                   (0U)
#define CSK_DVP_F_VER_Msk                   (0x1FFFUL << CSK_DVP_F_VER_Pos)            /*!< 0x00001FFF */
#define CSK_DVP_F_VER                       CSK_DVP_F_VER_Msk

/********************  Bits definition for DVP_P_OFFSET register  *********************/
#define CSK_DVP_P_OFFSET_Pos                (0U)
#define CSK_DVP_P_OFFSET_Msk                (0x1FFFUL << CSK_DVP_P_OFFSET_Pos)         /*!< 0x00001FFF */
#define CSK_DVP_P_OFFSET                    CSK_DVP_P_OFFSET_Msk

/********************  Bits definition for DVP_L_OFFSET register  *********************/
#define CSK_DVP_L_OFFSET_Pos                (0U)
#define CSK_DVP_L_OFFSET_Msk                (0x1FFFUL << CSK_DVP_L_OFFSET_Pos)         /*!< 0x00001FFF */
#define CSK_DVP_L_OFFSET                    CSK_DVP_L_OFFSET_Msk

/********************  Bits definition for DVP_POL_CNTL register  *********************/
#define CSK_DVP_POL_CNTL_CLOCK_Pos          (0U)
#define CSK_DVP_POL_CNTL_CLOCK_Msk          (0x1UL << CSK_DVP_POL_CNTL_CLOCK_Pos)      /*!< 0x00000001 */
#define CSK_DVP_POL_CNTL_CLOCK              CSK_DVP_POL_CNTL_CLOCK_Msk
#define CSK_DVP_POL_CNTL_CLOCK_RISING       (0x0UL << CSK_DVP_POL_CNTL_CLOCK_Pos)
#define CSK_DVP_POL_CNTL_CLOCK_FALLING      (0x1UL << CSK_DVP_POL_CNTL_CLOCK_Pos)
#define CSK_DVP_POL_CNTL_HSYNC_Pos          (1U)
#define CSK_DVP_POL_CNTL_HSYNC_Msk          (0x1UL << CSK_DVP_POL_CNTL_HSYNC_Pos)      /*!< 0x00000002 */
#define CSK_DVP_POL_CNTL_HSYNC              CSK_DVP_POL_CNTL_HSYNC_Msk
#define CSK_DVP_POL_CNTL_HSYNC_RISING       (0x0UL << CSK_DVP_POL_CNTL_HSYNC_Pos)
#define CSK_DVP_POL_CNTL_HSYNC_FALLING      (0x1UL << CSK_DVP_POL_CNTL_HSYNC_Pos)
#define CSK_DVP_POL_CNTL_VSYNC_Pos          (2U)
#define CSK_DVP_POL_CNTL_VSYNC_Msk          (0x1UL << CSK_DVP_POL_CNTL_VSYNC_Pos)      /*!< 0x00000004 */
#define CSK_DVP_POL_CNTL_VSYNC              CSK_DVP_POL_CNTL_VSYNC_Msk
#define CSK_DVP_POL_CNTL_VSYNC_RISING       (0x0UL << CSK_DVP_POL_CNTL_VSYNC_Pos)
#define CSK_DVP_POL_CNTL_VSYNC_FALLING      (0x1UL << CSK_DVP_POL_CNTL_VSYNC_Pos)


/********************  Bits definition for DVP_INPUT_FORM register  *******************/
#define CSK_DVP_INPUT_FORM_MODE_Pos         (2U)
#define CSK_DVP_INPUT_FORM_MODE_Msk         (0x3UL << CSK_DVP_INPUT_FORM_MODE_Pos)     /*!< 0x00000003 */
#define CSK_DVP_INPUT_FORM_MODE             CSK_DVP_INPUT_FORM_MODE_Msk
#define CSK_DVP_INPUT_FORM_YUV422           (0x0UL << CSK_DVP_INPUT_FORM_MODE_Pos)
#define CSK_DVP_INPUT_FORM_YUV420           (0x1UL << CSK_DVP_INPUT_FORM_MODE_Pos)
#define CSK_DVP_INPUT_FORM_LUMINA           (0x2UL << CSK_DVP_INPUT_FORM_MODE_Pos)

#define CSK_DVP_INPUT_FORM_YUV422_Y0CBY1CR        (0x00000000U | CSK_DVP_INPUT_FORM_YUV422)
#define CSK_DVP_INPUT_FORM_YUV422_CBY0CRY1        (0x00000001U | CSK_DVP_INPUT_FORM_YUV422)
#define CSK_DVP_INPUT_FORM_YUV422_Y0CRY1CB        (0x00000002U | CSK_DVP_INPUT_FORM_YUV422)
#define CSK_DVP_INPUT_FORM_YUV422_CRY0CBY1        (0x00000003U | CSK_DVP_INPUT_FORM_YUV422)
#define CSK_DVP_INPUT_FORM_YUV420_Y0CBY1Y2CRY3    (0x00000000U | CSK_DVP_INPUT_FORM_YUV420)
#define CSK_DVP_INPUT_FORM_YUV420_CBY0Y1CRY2Y3    (0x00000001U | CSK_DVP_INPUT_FORM_YUV420)
#define CSK_DVP_INPUT_FORM_YUV420_Y0Y1CBY2Y3CR    (0x00000002U | CSK_DVP_INPUT_FORM_YUV420)
#define CSK_DVP_INPUT_FORM_LUMINA_12BIT           (0x00000000U | CSK_DVP_INPUT_FORM_LUMINA)
#define CSK_DVP_INPUT_FORM_LUMINA_10BIT           (0x00000001U | CSK_DVP_INPUT_FORM_LUMINA)
#define CSK_DVP_INPUT_FORM_LUMINA_8BIT            (0x00000002U | CSK_DVP_INPUT_FORM_LUMINA)


/********************  Bits definition for DVP_JLB_HANSHK_SEL register  ***************/
#define CSK_DVP_JLB_HANSHK_SEL_USER0_Pos    (0U)
#define CSK_DVP_JLB_HANSHK_SEL_USER0_Msk    (0x1UL << CSK_DVP_JLB_HANSHK_SEL_USER0_Pos)/*!< 0x00000001 */
#define CSK_DVP_JLB_HANSHK_SEL_USER0        CSK_DVP_JLB_HANSHK_SEL_USER0_Msk
#define CSK_DVP_JLB_HANSHK_SEL_USER1_Pos    (1U)
#define CSK_DVP_JLB_HANSHK_SEL_USER1_Msk    (0x1UL << CSK_DVP_JLB_HANSHK_SEL_USER1_Pos)/*!< 0x00000002 */
#define CSK_DVP_JLB_HANSHK_SEL_USER1        CSK_DVP_JLB_HANSHK_SEL_USER1_Msk
#define CSK_DVP_JLB_HANSHK_SEL_MODE_Pos     (2U)
#define CSK_DVP_JLB_HANSHK_SEL_MODE_Msk     (0x1UL << CSK_DVP_JLB_HANSHK_SEL_MODE_Pos) /*!< 0x00000004 */
#define CSK_DVP_JLB_HANSHK_SEL_MODE         CSK_DVP_JLB_HANSHK_SEL_MODE_Msk
#define CSK_DVP_JLB_HANSHK_SEL_MODE_HARD    (0x0UL << CSK_DVP_JLB_HANSHK_SEL_MODE_Pos)
#define CSK_DVP_JLB_HANSHK_SEL_MODE_SOFT    (0x1UL << CSK_DVP_JLB_HANSHK_SEL_MODE_Pos)

/********************  Bits definition for DVP_DATA_BUS_SEL register  *****************/
#define CSK_DVP_DATA_BUS_SEL_Pos            (0U)
#define CSK_DVP_DATA_BUS_SEL_Msk            (0x1UL << CSK_DVP_DATA_BUS_SEL_Pos)        /*!< 0x00000001 */
#define CSK_DVP_DATA_BUS_SEL                CSK_DVP_DATA_BUS_SEL_Msk
#define CSK_DVP_DATA_BUS_SEL_RIGHT          (0x0UL << CSK_DVP_DATA_BUS_SEL_Pos)
#define CSK_DVP_DATA_BUS_SEL_LEFT           (0x1UL << CSK_DVP_DATA_BUS_SEL_Pos)

/********************  Bits definition for DVP_VI_EN register  ************************/
#define CSK_DVP_VI_EN_Pos                   (0U)
#define CSK_DVP_VI_EN_Msk                   (0x1UL << CSK_DVP_VI_EN_Pos)               /*!< 0x00000001 */
#define CSK_DVP_VI_EN                       CSK_DVP_VI_EN_Msk

/********************  Bits definition for DVP_STATUS register  ***********************/
#define CSK_DVP_STATUS_FIFO_OVF_Pos         (0U)
#define CSK_DVP_STATUS_FIFO_OVF_Msk         (0x1UL << CSK_DVP_STATUS_FIFO_OVF_Pos)     /*!< 0x00000001 */
#define CSK_DVP_STATUS_FIFO_OVF             CSK_DVP_STATUS_FIFO_OVF_Msk
#define CSK_DVP_STATUS_AHB_ERR_Pos          (1U)
#define CSK_DVP_STATUS_AHB_ERR_Msk          (0x1UL << CSK_DVP_STATUS_AHB_ERR_Pos)      /*!< 0x00000002 */
#define CSK_DVP_STATUS_AHB_ERR              CSK_DVP_STATUS_AHB_ERR_Msk
#define CSK_DVP_STATUS_FRM_CMPLT_Pos        (2U)
#define CSK_DVP_STATUS_FRM_CMPLT_Msk        (0x1UL << CSK_DVP_STATUS_FRM_CMPLT_Pos)    /*!< 0x00000004 */
#define CSK_DVP_STATUS_FRM_CMPLT            CSK_DVP_STATUS_FRM_CMPLT_Msk
#define CSK_DVP_STATUS_JLB0_FULL_Pos        (3U)
#define CSK_DVP_STATUS_JLB0_FULL_Msk        (0x1UL << CSK_DVP_STATUS_JLB0_FULL_Pos)    /*!< 0x00000008 */
#define CSK_DVP_STATUS_JLB0_FULL            CSK_DVP_STATUS_JLB0_FULL_Msk
#define CSK_DVP_STATUS_JLB1_FULL_Pos        (4U)
#define CSK_DVP_STATUS_JLB1_FULL_Msk        (0x1UL << CSK_DVP_STATUS_JLB1_FULL_Pos)    /*!< 0x00000010 */
#define CSK_DVP_STATUS_JLB1_FULL            CSK_DVP_STATUS_JLB1_FULL_Msk

/********************  Bits definition for DVP_INTR_MSK register  *********************/
#define CSK_DVP_INTR_MSK_FIFO_OVF_Pos       (0U)
#define CSK_DVP_INTR_MSK_FIFO_OVF_Msk       (0x1UL << CSK_DVP_INTR_MSK_FIFO_OVF_Pos)   /*!< 0x00000001 */
#define CSK_DVP_INTR_MSK_FIFO_OVF           CSK_DVP_INTR_MSK_FIFO_OVF_Msk
#define CSK_DVP_INTR_MSK_AHB_ERR_Pos        (1U)
#define CSK_DVP_INTR_MSK_AHB_ERR_Msk        (0x1UL << CSK_DVP_INTR_MSK_AHB_ERR_Pos)    /*!< 0x00000002 */
#define CSK_DVP_INTR_MSK_AHB_ERR            CSK_DVP_INTR_MSK_AHB_ERR_Msk
#define CSK_DVP_INTR_MSK_FRM_CMPLT_Pos      (2U)
#define CSK_DVP_INTR_MSK_FRM_CMPLT_Msk      (0x1UL << CSK_DVP_INTR_MSK_FRM_CMPLT_Pos)  /*!< 0x00000004 */
#define CSK_DVP_INTR_MSK_FRM_CMPLT          CSK_DVP_INTR_MSK_FRM_CMPLT_Msk
#define CSK_DVP_INTR_MSK_JLB0_FULL_Pos      (3U)
#define CSK_DVP_INTR_MSK_JLB0_FULL_Msk      (0x1UL << CSK_DVP_INTR_MSK_JLB0_FULL_Pos)  /*!< 0x00000008 */
#define CSK_DVP_INTR_MSK_JLB0_FULL          CSK_DVP_INTR_MSK_JLB0_FULL_Msk
#define CSK_DVP_INTR_MSK_JLB1_FULL_Pos      (4U)
#define CSK_DVP_INTR_MSK_JLB1_FULL_Msk      (0x1UL << CSK_DVP_INTR_MSK_JLB1_FULL_Pos)  /*!< 0x00000010 */
#define CSK_DVP_INTR_MSK_JLB1_FULL          CSK_DVP_INTR_MSK_JLB1_FULL_Msk

/********************  Bits definition for DVP_INTR_CLR register  *********************/
#define CSK_DVP_INTR_CLR_FIFO_OVF_Pos       (0U)
#define CSK_DVP_INTR_CLR_FIFO_OVF_CLR       (0x1UL << CSK_DVP_INTR_CLR_FIFO_OVF_Pos)   /*!< 0x00000001 */
#define CSK_DVP_INTR_CLR_FIFO_OVF           CSK_DVP_INTR_CLR_FIFO_OVF_CLR
#define CSK_DVP_INTR_CLR_AHB_ERR_Pos        (1U)
#define CSK_DVP_INTR_CLR_AHB_ERR_CLR        (0x1UL << CSK_DVP_INTR_CLR_AHB_ERR_Pos)    /*!< 0x00000002 */
#define CSK_DVP_INTR_CLR_AHB_ERR            CSK_DVP_INTR_CLR_AHB_ERR_CLR
#define CSK_DVP_INTR_CLR_FRM_CMPLT_Pos      (2U)
#define CSK_DVP_INTR_CLR_FRM_CMPLT_CLR      (0x1UL << CSK_DVP_INTR_CLR_FRM_CMPLT_Pos)  /*!< 0x00000004 */
#define CSK_DVP_INTR_CLR_FRM_CMPLT          CSK_DVP_INTR_CLR_FRM_CMPLT_CLR
#define CSK_DVP_INTR_CLR_JLB0_FULL_Pos      (3U)
#define CSK_DVP_INTR_CLR_JLB0_FULL_CLR      (0x1UL << CSK_DVP_INTR_CLR_JLB0_FULL_Pos)  /*!< 0x00000008 */
#define CSK_DVP_INTR_CLR_JLB0_FULL          CSK_DVP_INTR_CLR_JLB0_FULL_CLR
#define CSK_DVP_INTR_CLR_JLB1_FULL_Pos      (4U)
#define CSK_DVP_INTR_CLR_JLB1_FULL_CLR      (0x1UL << CSK_DVP_INTR_CLR_JLB1_FULL_Pos)  /*!< 0x00000010 */
#define CSK_DVP_INTR_CLR_JLB1_FULL          CSK_DVP_INTR_CLR_JLB1_FULL_CLR

/********************  Bits definition for DVP_Y_ADDR0 register  **********************/
#define CSK_DVP_Y_ADDR0_Pos                 (2U)
#define CSK_DVP_Y_ADDR0_Msk                 (0x3FFFFFFFUL << CSK_DVP_Y_ADDR0_Pos)      /*!< 0x3FFFFFFF */
#define CSK_DVP_Y_ADDR0                     CSK_DVP_Y_ADDR0_Msk

/********************  Bits definition for DVP_CB_ADDR0 register  *********************/
#define CSK_DVP_CB_ADDR0_Pos                (2U)
#define CSK_DVP_CB_ADDR0_Msk                (0x3FFFFFFFUL << CSK_DVP_CB_ADDR0_Pos)     /*!< 0x3FFFFFFF */
#define CSK_DVP_CB_ADDR0                    CSK_DVP_CB_ADDR0_Msk

/********************  Bits definition for DVP_CR_ADDR0 register  *********************/
#define CSK_DVP_CR_ADDR0_Pos                (2U)
#define CSK_DVP_CR_ADDR0_Msk                (0x3FFFFFFFUL << CSK_DVP_CR_ADDR0_Pos)     /*!< 0x3FFFFFFF */
#define CSK_DVP_CR_ADDR0                    CSK_DVP_CR_ADDR0_Msk

/********************  Bits definition for DVP_INTR_CLR register  *********************/
#define CSK_DVP_INTR_CLR_Pos                (0U)
#define CSK_DVP_INTR_CLR_Msk                (0x1UL << CSK_DVP_INTR_CLR_Pos)            /*!< 0x00000001 */
#define CSK_DVP_INTR_CLR                    CSK_DVP_INTR_CLR_Msk

/********************  Bits definition for DVP_Y_ADDR1 register  **********************/
#define CSK_DVP_Y_ADDR1_Pos                 (2U)
#define CSK_DVP_Y_ADDR1_Msk                 (0x3FFFFFFFUL << CSK_DVP_Y_ADDR1_Pos)      /*!< 0x3FFFFFFF */
#define CSK_DVP_Y_ADDR1                     CSK_DVP_Y_ADDR1_Msk

/********************  Bits definition for DVP_CB_ADDR1 register  *********************/
#define CSK_DVP_CB_ADDR1_Pos                (2U)
#define CSK_DVP_CB_ADDR1_Msk                (0x3FFFFFFFUL << CSK_DVP_CB_ADDR1_Pos)     /*!< 0x3FFFFFFF */
#define CSK_DVP_CB_ADDR1                    CSK_DVP_CB_ADDR1_Msk

/********************  Bits definition for DVP_CR_ADDR1 register  *********************/
#define CSK_DVP_CR_ADDR1_Pos                (2U)
#define CSK_DVP_CR_ADDR1_Msk                (0x3FFFFFFFUL << CSK_DVP_CR_ADDR1_Pos)     /*!< 0x3FFFFFFF */
#define CSK_DVP_CR_ADDR1                    CSK_DVP_CR_ADDR1_Msk

/********************  Bits definition for DVP_JLB0_HANSHK_U0 register  ***************/
#define CSK_DVP_JLB0_HANSHK_U0_Pos          (0U)
#define CSK_DVP_JLB0_HANSHK_U0_Msk          (0x1UL << CSK_DVP_JLB0_HANSHK_U0_Pos)      /*!< 0x00000001 */
#define CSK_DVP_JLB0_HANSHK_U0              CSK_DVP_JLB0_HANSHK_U0_Msk

/********************  Bits definition for DVP_JLB0_HANSHK_U1 register  ***********/
#define CSK_DVP_JLB0_HANSHK_U1_Pos          (0U)
#define CSK_DVP_JLB0_HANSHK_U1_Msk          (0x1UL << CSK_DVP_JLB0_HANSHK_U1_Pos)      /*!< 0x00000001 */
#define CSK_DVP_JLB0_HANSHK_U1              CSK_DVP_JLB0_HANSHK_U1_Msk

/********************  Bits definition for DVP_JLB1_HANSHK_U0 register  ***************/
#define CSK_DVP_JLB1_HANSHK_U0_Pos          (0U)
#define CSK_DVP_JLB1_HANSHK_U0_Msk          (0x1UL << CSK_DVP_JLB1_HANSHK_U0_Pos)      /*!< 0x00000001 */
#define CSK_DVP_JLB1_HANSHK_U0              CSK_DVP_JLB1_HANSHK_U0_Msk

/********************  Bits definition for DVP_JLB1_HANSHK_U1 register  ***************/
#define CSK_DVP_JLB1_HANSHK_U1_Pos          (0U)
#define CSK_DVP_JLB1_HANSHK_U1_Msk          (0x1UL << CSK_DVP_JLB1_HANSHK_U1_Pos)      /*!< 0x00000001 */
#define CSK_DVP_JLB1_HANSHK_U1              CSK_DVP_JLB1_HANSHK_U1_Msk

/********************  Bits definition for DVP_JLB_HANSHK_O register  *****************/
#define CSK_DVP_JLB_HANSHK_O_FULL0_Pos      (0U)
#define CSK_DVP_JLB_HANSHK_O_FULL0_Msk      (0x1UL << CSK_DVP_JLB_HANSHK_O_FULL0_Pos)  /*!< 0x00000001 */
#define CSK_DVP_JLB_HANSHK_O_FULL0          CSK_DVP_JLB_HANSHK_O_FULL0_Msk
#define CSK_DVP_JLB_HANSHK_O_FULL1_Pos      (1U)
#define CSK_DVP_JLB_HANSHK_O_FULL1_Msk      (0x1UL << CSK_DVP_JLB_HANSHK_O_FULL1_Pos)  /*!< 0x00000002 */
#define CSK_DVP_JLB_HANSHK_O_FULL1          CSK_DVP_JLB_HANSHK_O_FULL1_Msk

/********************  DVP_Error_Code DVP Error Code  **********************************/
#define DVP_ERROR_NONE             			((uint32_t)0x00000000U)  				   /*!< No error     */
#define DVP_ERROR_OVR              			((uint32_t)0x00000001U) 				   /*!< Overrun error*/
#define DVP_ERROR_AHB             			((uint32_t)0x00000002U)  				   /*!< AHB error    */
#define DVP_ERROR_MEM_RANGE        			((uint32_t)0x00000010U)  				   /*!< Memory error */
#define DVP_ERROR_TIMEOUT          			((uint32_t)0x00000020U)  				   /*!< Timeout error*/

typedef void (*CSK_DVP_SignalEvent_t)(uint32_t event, uint32_t param);

/**
  * @brief DVP
  */

typedef struct
{
	volatile uint32_t F_HOR;            /*!< DVP frame horizontal pixels,        Address offset: 0x00 */
	volatile uint32_t F_VER;            /*!< DVP frame vertical lines,           Address offset: 0x04 */
	volatile uint32_t PIXEL_CLK_EN;     /*!< DVP clock out enable,               Address offset: 0x08 */
	volatile uint32_t PIXEL_CLK_DIV;    /*!< DVP clock out divider,              Address offset: 0x0C */
	volatile uint32_t P_OFFSET;         /*!< DVP pixel offset,                   Address offset: 0x10 */
	volatile uint32_t L_OFFSET;         /*!< DVP line offset,                    Address offset: 0x14 */
	volatile uint32_t POL_CNTL;         /*!< DVP polarity control,               Address offset: 0x18 */
	volatile uint32_t PIXEL_RST_MASK;   /*!< DVP reset signal mask,              Address offset: 0x1C */
	volatile uint32_t INPUT_FORM;       /*!< DVP input format,                   Address offset: 0x20 */
	volatile uint32_t JLB_HANSHK_SEL;   /*!< DVP JLB handshaking mechanism set,  Address offset: 0x24 */
	volatile uint32_t DATA_BUS_SEL;     /*!< DVP data bus selection,             Address offset: 0x28 */
	volatile uint32_t VI_EN;            /*!< DVP video controller enable,        Address offset: 0x2C */
	volatile uint32_t STATUS;           /*!< DVP status,                         Address offset: 0x30 */
	volatile uint32_t INTR_MSK;         /*!< DVP interrupt mask,                 Address offset: 0x34 */
	volatile uint32_t INTR_CLR;         /*!< DVP interrupt clear,                Address offset: 0x38 */
	uint32_t RESERVED0;
	volatile uint32_t Y_ADDR0;          /*!< DVP buffer0 address of Y,           Address offset: 0x40 */
	volatile uint32_t CB_ADDR0;         /*!< DVP buffer0 address of Cb,          Address offset: 0x44 */
	volatile uint32_t CR_ADDR0;         /*!< DVP buffer0 address of Cr,          Address offset: 0x48 */
	uint32_t RESERVED1;
	volatile uint32_t Y_ADDR1;          /*!< DVP buffer1 address of Y,           Address offset: 0x50 */
	volatile uint32_t CB_ADDR1;         /*!< DVP buffer1 address of Cb,          Address offset: 0x54 */
	volatile uint32_t CR_ADDR1;         /*!< DVP buffer1 address of Cr,          Address offset: 0x58 */
	uint32_t RESERVED2;
	volatile uint32_t JLB0_HANSHK_U0;   /*!< DVP buffer0 user0 indication,       Address offset: 0x60 */
	volatile uint32_t JLB0_HANSHK_U1;   /*!< DVP buffer0 user1 indication,       Address offset: 0x64 */
	volatile uint32_t JLB1_HANSHK_U0;   /*!< DVP buffer1 user0 indication,       Address offset: 0x68 */
	volatile uint32_t JLB1_HANSHK_U1;   /*!< DVP buffer1 user1 indication,       Address offset: 0x6C */
	volatile uint32_t JLB_HANSHK_O;     /*!< DVP buffer indication from VIC,     Address offset: 0x70 */
} DVP_TypeDef;


/**
  * @brief  DVP State structures definition
  */
typedef enum
{
	DVP_STATE_RESET             = 0x00U,  /*!< DVP not yet initialized or disabled  */
	DVP_STATE_READY             = 0x01U,  /*!< DVP initialized and ready for use    */
	DVP_STATE_BUSY              = 0x02U,  /*!< DVP internal processing is ongoing   */
	DVP_STATE_TIMEOUT           = 0x03U,  /*!< DVP timeout state                    */
	DVP_STATE_ERROR             = 0x04U,  /*!< DVP error state                      */
	DVP_STATE_SUSPENDED         = 0x05U   /*!< DVP suspend state                    */
}DVP_StateTypeDef;



/**
  * @brief   DVP Init structure definition
  */
typedef struct
{

	uint32_t  PCKPolarity;                /*!< Specifies the Pixel clock polarity: Falling or Rising.
											 This parameter can be a value of @ref DVP_PIXCK_Polarity       */

	uint32_t  VSPolarity;                 /*!< Specifies the Vertical synchronization polarity: High or Low.
											 This parameter can be a value of @ref DVP_VSYNC_Polarity       */

	uint32_t  HSPolarity;                 /*!< Specifies the Horizontal synchronization polarity: High or Low.
											 This parameter can be a value of @ref DVP_HSYNC_Polarity       */

	uint32_t  InputFormat;                /*!< Specifies the input format of image sensor.                  */

	uint32_t  FrameWidth;                 /*!< Specifies the width of frame in pixels.                      */

	uint32_t  FrameHeight;                /*!< Specifies the height of frame in lines.                      */

	uint32_t  PixelOffset;                /*!< Specifies the clock cycles between deactivation and activation
											 of the h_sync.                                                 */

	uint32_t  LineOffset;                 /*!< Specifies the first valid image line offset from the
											 deactivation of the v_sync.                                    */

	uint32_t  HandshakingMode;            /*!< Specifies the handshaking mode including which user/users
											 to enable and software/hardware handshaking selection.         */

	uint32_t  DataAlign;                  /*!< Specifies the data on the bus aligned to right or left.      */

	uint32_t  MaxAddressY;                /*!< Specifies the maximum address of Y.                          */

	uint32_t  MaxAddressCb;               /*!< Specifies the maximum address of Cb.                         */

	uint32_t  MaxAddressCr;               /*!< Specifies the maximum address of Cr.                         */


}DVP_InitTypeDef;


/**
  * @brief   DVP Init structure definition
  */
typedef struct
{
	uint8_t                       *pAddrY;             /*!< Pointer to Y_ADDR buffer   */
	uint8_t                       *pAddrCb;            /*!< Pointer to CB_ADDR buffer  */
	uint8_t                       *pAddrCr;            /*!< Pointer to CR_ADDR buffer  */
	uint32_t                      bufLenY;             /*!< Length of Y_ADDR buffer    */
	uint32_t                      bufLenCb;            /*!< Length of CB_ADDR buffer   */
	uint32_t                      bufLenCr;            /*!< Length of CR_ADDR buffer   */
}DVP_BufferDef;


/**
  * @brief  DVP handle Structure definition
  */
typedef struct __DVP_HandleTypeDef
{
	DVP_TypeDef                   *Instance;           /*!< DVP Register base address  */

	DVP_InitTypeDef               Init;                /*!< DVP parameters             */

	CSK_DVP_SignalEvent_t         cb_event;            /*!< DVP callback               */

	volatile DVP_StateTypeDef     State;               /*!< DVP state                  */

	volatile uint32_t             ErrorCode;           /*!< DVP Error code             */
}DVP_HandleTypeDef;



/* Exported functions --------------------------------------------------------*/

void* DVP_Instance(void);
int32_t DVP_Init(DVP_HandleTypeDef *dvp_dev, void *callback);
int32_t DVP_DeInit(DVP_HandleTypeDef *dvp_dev);
void DVP_MspInit(DVP_HandleTypeDef* dvp_dev);
void DVP_MspDeInit(DVP_HandleTypeDef* dvp_dev);
int32_t DVP_Start(DVP_HandleTypeDef* dvp_dev, DVP_BufferDef frame_buf);
void DVP_Stop(DVP_HandleTypeDef* dvp_dev);
void DVP_EnableClockout(DVP_HandleTypeDef *dvp_dev, uint8_t divider);
void DVP_DisableClockout(DVP_HandleTypeDef *dvp_dev);
DVP_StateTypeDef DVP_GetState(DVP_HandleTypeDef *dvp_dev);
uint32_t DVP_GetError(DVP_HandleTypeDef *dvp_dev);



#endif /* __DRIVER_DVP_H */


