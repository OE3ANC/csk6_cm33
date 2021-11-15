/*
 * Copyright (c) 2020-2025 ChipSky Technology
 * All rights reserved.
 *
 */
#include <assert.h>
#include "hal/reg/cmn_mailbox_reg_venus.h"
#include "hal/Driver_MBX.h"

//#define MBX_AP_WORK

#define CSK_MBX_CH(ch)            (0x1UL << ch)

typedef struct
{
	MAILBOX_RegDef *mbx_cb;
	CSK_MBX_SignalEvent_t cb_event;
	uint32_t flags;
	uint32_t *rx_buf;  // (0~14) are virtual data channels, 15 as notified channel
} MBX_DEV;


CSK_DRIVER_VERSION MBX_GetVersion()
{
	CSK_DRIVER_VERSION version = {1, 0};
    return version;
}

#ifdef MBX_AP_WORK
#include "venus_ap.h"
#include "venus_log.h"

static MBX_DEV mbx_res = {
	((MAILBOX_RegDef *)CMN_MAILBOX_BASE),
	((CSK_MBX_SignalEvent_t)NULL),
	0,
	NULL
};

static void mbx_irq_handler(void)
{
	uint32_t status, i, val = 0;

	status = mbx_res.mbx_cb->REG_CP_MAILBOX_IRQ.all;

	if(status & CSK_MBX_CH(15)) {  // send completely, notified interrupt
		mbx_res.mbx_cb->REG_CP_MAILBOX_IRQ.all = CSK_MBX_CH(15);  //clear irq bits
		__COMPILER_BARRIER();
		mbx_res.cb_event(CSK_MBX_EVENT_SEND_COMPLETE, 15);
		status &= ~CSK_MBX_CH(15);
	}
	if(status) {  // data receive interrupt
		switch(status) {
			case CSK_MBX_CH(0):  i = 0;  break;
			case CSK_MBX_CH(1):  i = 1;  break;
			case CSK_MBX_CH(2):  i = 2;  break;
			case CSK_MBX_CH(3):  i = 3;  break;
			case CSK_MBX_CH(4):  i = 4;  break;
			case CSK_MBX_CH(5):  i = 5;  break;
			case CSK_MBX_CH(6):  i = 6;  break;
			case CSK_MBX_CH(7):  i = 7;  break;
			case CSK_MBX_CH(8):  i = 8;  break;
			case CSK_MBX_CH(9):  i = 9;  break;
			case CSK_MBX_CH(10): i = 10; break;
			case CSK_MBX_CH(11): i = 11; break;
			case CSK_MBX_CH(12): i = 12; break;
			case CSK_MBX_CH(13): i = 13; break;
			case CSK_MBX_CH(14): i = 14; break;
			default:             i = 15; break;
		}
		if(i < 15) {
			if(mbx_res.rx_buf) {
				mbx_res.rx_buf[0] = mbx_res.mbx_cb->REG_0X030.all;
				mbx_res.rx_buf[1] = mbx_res.mbx_cb->REG_0X034.all;
				mbx_res.rx_buf[2] = mbx_res.mbx_cb->REG_0X038.all;
				mbx_res.rx_buf[3] = mbx_res.mbx_cb->REG_0X03C.all;
			}
			mbx_res.rx_buf = NULL;  //avoid data overwrite
			__COMPILER_BARRIER();
			mbx_res.mbx_cb->REG_CP_MAILBOX_IRQ.all = 0x1U << i;  //clear irq bits
			__COMPILER_BARRIER();

			mbx_res.mbx_cb->REG_AP_MAILBOX_CTRL.all |= 0x1U << (15 + 16);  //notify
			mbx_res.cb_event(CSK_MBX_EVENT_RECEIVE_COMPLETE, i);
		}
	}
}

// export MBX API function: MBX_Initialize
int32_t MBX_Initialize(void *mbx_dev, CSK_MBX_SignalEvent_t cb_event)
{
	if((mbx_res.flags & MBX_FLAG_INITIALIZED) == 0U) {
		uint32_t val = 0xffff;
		MBX_Control(NULL, CSK_MBX_CP_CTRL_SET_CHS, &val);
		mbx_res.cb_event = cb_event;
//		mbx_res.usr_param = usr_param;
		mbx_res.flags |= MBX_FLAG_INITIALIZED;
		register_ISR(IRQ_CMN_MAILBOX_VECTOR, mbx_irq_handler, NULL);
		//clear interrupt bits, and enable
		mbx_res.mbx_cb->REG_CP_MAILBOX_IRQ.all = val;
		enable_IRQ(IRQ_CMN_MAILBOX_VECTOR);
	}

    return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Uninitialize
int32_t MBX_Uninitialize(void *mbx_dev)
{
	mbx_res.flags = 0;
    return CSK_DRIVER_OK;
}

// export MBX API function: MBX_Send
int32_t MBX_Send(void *mbx_dev, const void *data, uint32_t ch)
{
	uint32_t status;

	if(ch > 14) {
		return CSK_DRIVER_ERROR_PARAMETER;
	}
	status = mbx_res.mbx_cb->REG_AP_MAILBOX_IRQ.all;
	if(status & 0x7fff) {
		return CSK_DRIVER_ERROR_BUSY;
	}
	mbx_res.mbx_cb->REG_0X010.all = ((uint32_t *)data)[0];
	mbx_res.mbx_cb->REG_0X014.all = ((uint32_t *)data)[1];
	mbx_res.mbx_cb->REG_0X018.all = ((uint32_t *)data)[2];
	mbx_res.mbx_cb->REG_0X01C.all = ((uint32_t *)data)[3];

	mbx_res.mbx_cb->REG_AP_MAILBOX_CTRL.all |= 0x1 << (ch + 16);
    return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Receive
int32_t MBX_Receive(void *mbx_dev, void *data, uint32_t ch)
{
	mbx_res.rx_buf = data;
    return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Control
int32_t MBX_Control(void *mbx_dev, uint32_t control, void *arg)
{
	switch(control) {
	case CSK_MBX_AP_CTRL_SET_CHS:
		mbx_res.mbx_cb->REG_AP_MAILBOX_CTRL.all = *((uint32_t *)arg) & 0xffff;
		break;
	case CSK_MBX_AP_CTRL_GET_CHS:
		*((uint32_t *)arg) = mbx_res.mbx_cb->REG_AP_MAILBOX_CTRL.all & 0xffff;
		break;
	case CSK_MBX_AP_CTRL_SET_LK:
		if(*((uint32_t *)arg)) { // set lock
			if((mbx_res.mbx_cb->REG_AP_MAILBOX_LOCK.all & 0x1) == 0) {
				mbx_res.mbx_cb->REG_AP_MAILBOX_LOCK.all = 0x5A5A;
			}
		} else { // set unlock
			if(mbx_res.mbx_cb->REG_AP_MAILBOX_LOCK.all & 0x1) {
				mbx_res.mbx_cb->REG_AP_MAILBOX_LOCK.all = 0x5A5A;
			}
		}
		break;
	case CSK_MBX_AP_CTRL_GET_LK:
		*((uint32_t *)arg) = mbx_res.mbx_cb->REG_AP_MAILBOX_LOCK.all & 0x1;
		break;
	default:
		return CSK_DRIVER_ERROR_PARAMETER;
		break;
	}

    return CSK_DRIVER_OK;
}
#else
#include "hal/venus_cp.h"

static MBX_DEV mbx_res = {
	((MAILBOX_RegDef *)CMN_MAILBOX_BASE),
	((CSK_MBX_SignalEvent_t)NULL),
	0,
	NULL
};

static void mbx_irq_handler(void)
{
	uint32_t status, i, val = 0;

	status = mbx_res.mbx_cb->REG_AP_MAILBOX_IRQ.all;

	if(status & CSK_MBX_CH(15)) {  // send completely, notified interrupt
		mbx_res.mbx_cb->REG_AP_MAILBOX_IRQ.all = CSK_MBX_CH(15);  //clear irq bits
//		__COMPILER_BARRIER();
		mbx_res.cb_event(CSK_MBX_EVENT_SEND_COMPLETE, 15);
		status &= ~CSK_MBX_CH(15);
	}
	if(status) {  // data receive interrupt
		switch(status) {
			case CSK_MBX_CH(0):  i = 0;  break;
			case CSK_MBX_CH(1):  i = 1;  break;
			case CSK_MBX_CH(2):  i = 2;  break;
			case CSK_MBX_CH(3):  i = 3;  break;
			case CSK_MBX_CH(4):  i = 4;  break;
			case CSK_MBX_CH(5):  i = 5;  break;
			case CSK_MBX_CH(6):  i = 6;  break;
			case CSK_MBX_CH(7):  i = 7;  break;
			case CSK_MBX_CH(8):  i = 8;  break;
			case CSK_MBX_CH(9):  i = 9;  break;
			case CSK_MBX_CH(10): i = 10; break;
			case CSK_MBX_CH(11): i = 11; break;
			case CSK_MBX_CH(12): i = 12; break;
			case CSK_MBX_CH(13): i = 13; break;
			case CSK_MBX_CH(14): i = 14; break;
			default:             i = 15; break;
		}
		if(i < 15) {
			if(mbx_res.rx_buf) {
				mbx_res.rx_buf[0] = mbx_res.mbx_cb->REG_0X010.all;
				mbx_res.rx_buf[1] = mbx_res.mbx_cb->REG_0X014.all;
				mbx_res.rx_buf[2] = mbx_res.mbx_cb->REG_0X018.all;
				mbx_res.rx_buf[3] = mbx_res.mbx_cb->REG_0X01C.all;
			}
			mbx_res.rx_buf = NULL;  //avoid data overwrite
//			__COMPILER_BARRIER();
			mbx_res.mbx_cb->REG_AP_MAILBOX_IRQ.all = 0x1U << i;  //clear irq bits
//			__COMPILER_BARRIER();

			mbx_res.mbx_cb->REG_CP_MAILBOX_CTRL.all |= 0x1U << (15 + 16);  //notify
			mbx_res.cb_event(CSK_MBX_EVENT_RECEIVE_COMPLETE, i);
		}
	}
}

// export MBX API function: MBX_Initialize
int32_t MBX_Initialize(void *mbx_dev, CSK_MBX_SignalEvent_t cb_event)
{
	if((mbx_res.flags & MBX_FLAG_INITIALIZED) == 0U) {
		uint32_t val = 0xffff;
		MBX_Control(NULL, CSK_MBX_CP_CTRL_SET_CHS, &val);
		mbx_res.cb_event = cb_event;
//		mbx_res.usr_param = usr_param;
		mbx_res.flags |= MBX_FLAG_INITIALIZED;

//		xthal_disable_interrupts(); // ALL INTERRUPTS OFF during initialization
		disable_GINT();
//		xtos_set_interrupt_handler(IRQ_MAILBOX_VECTOR, (xtos_handler) mbx_irq_handler, NULL, NULL);
		// register_ISR(IRQ_MAILBOX_VECTOR, (ISR) mbx_irq_handler, NULL);
//		xthal_enable_interrupts();
		enable_GINT();
		//clear interrupt bits, and enable
		mbx_res.mbx_cb->REG_AP_MAILBOX_IRQ.all = val;
		enable_IRQ(IRQ_MAILBOX_VECTOR);
	}

    return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Uninitialize
int32_t MBX_Uninitialize(void *mbx_dev)
{
	mbx_res.flags = 0;
    return CSK_DRIVER_OK;
}

// export MBX API function: MBX_Send
int32_t MBX_Send(void *mbx_dev, const void *data, uint32_t ch)
{
    uint32_t status;

	if(ch > 14) {
		return CSK_DRIVER_ERROR_PARAMETER;
	}
	status = mbx_res.mbx_cb->REG_CP_MAILBOX_IRQ.all;
	if(status & 0x7fff) {
		return CSK_DRIVER_ERROR_BUSY;
	}
	mbx_res.mbx_cb->REG_0X030.all = ((uint32_t *)data)[0];
	mbx_res.mbx_cb->REG_0X034.all = ((uint32_t *)data)[1];
	mbx_res.mbx_cb->REG_0X038.all = ((uint32_t *)data)[2];
	mbx_res.mbx_cb->REG_0X03C.all = ((uint32_t *)data)[3];

	mbx_res.mbx_cb->REG_CP_MAILBOX_CTRL.all |= 0x1 << (ch + 16);
	return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Receive
int32_t MBX_Receive(void *mbx_dev, void *data, uint32_t ch)
{
	mbx_res.rx_buf = data;
	return CSK_DRIVER_OK;
}


// export MBX API function: MBX_Control
int32_t MBX_Control(void *mbx_dev, uint32_t control, void *arg)
{
	switch(control) {
	case CSK_MBX_CP_CTRL_SET_CHS:
		mbx_res.mbx_cb->REG_CP_MAILBOX_CTRL.all = *((uint32_t *)arg) & 0xffff;
		break;
	case CSK_MBX_CP_CTRL_GET_CHS:
		*((uint32_t *)arg) = mbx_res.mbx_cb->REG_CP_MAILBOX_CTRL.all & 0xffff;
		break;
	case CSK_MBX_CP_CTRL_SET_LK:
		if(*((uint32_t *)arg)) { // set lock
			if((mbx_res.mbx_cb->REG_CP_MAILBOX_LOCK.all & 0x1) == 0) {
				mbx_res.mbx_cb->REG_CP_MAILBOX_LOCK.all = 0x5A5A;
			}
		} else { // set unlock
			if(mbx_res.mbx_cb->REG_CP_MAILBOX_LOCK.all & 0x1) {
				mbx_res.mbx_cb->REG_CP_MAILBOX_LOCK.all = 0x5A5A;
			}
		}
		break;
	case CSK_MBX_CP_CTRL_GET_LK:
		*((uint32_t *)arg) = mbx_res.mbx_cb->REG_CP_MAILBOX_LOCK.all & 0x1;
		break;
	default:
		return CSK_DRIVER_ERROR_PARAMETER;
		break;
	}

    return CSK_DRIVER_OK;
}
#endif

