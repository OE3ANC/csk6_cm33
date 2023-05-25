/*
 * Driver_IR.h
 *
 *  Created on: 2020年8月11日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_DRIVER_IR_H_
#define INCLUDE_DRIVER_DRIVER_IR_H_

#include "Driver_Common.h"

#define CSK_IR_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)  /* API version */

// adjust pos value
#define CSK_IR_CONTROL_Pos                    0
#define CSK_IR_CONTROL_Msk                   (0xFUL << CSK_IR_CONTROL_Pos)

#define CSK_IR_SW_CARRY_Pos                   5
#define CSK_IR_SW_CARRY_Msk                  (1UL << CSK_IR_SW_CARRY_Pos)
#define CSK_IR_SW_CARRY_CONFIG               (1UL << CSK_IR_SW_CARRY_Pos)

/*----- IR Control Codes: Miscellaneous Controls  -----*/
#define CSK_IR_CONTROL_SW_TIMEOUT_THRES      (0x1UL << CSK_IR_CONTROL_Pos)
#define CSK_IR_CONTROL_ABORT                 (0x2UL << CSK_IR_CONTROL_Pos)
#define CSK_IR_CONTROL_TX_REPEAT             (0x3UL << CSK_IR_CONTROL_Pos)
#define CSK_IR_CONTROL_CLEAR_TX_FIFO         (0x4UL << CSK_IR_CONTROL_Pos)
#define CSK_IR_CONTROL_CLEAR_RX_FIFO         (0x5UL << CSK_IR_CONTROL_Pos)

/*----- IR Control Codes: Below control parameter configure the HARDWARE transmit timing.
 * 配置这些参数只有在一些特殊情况，比如当红外硬件设备因为一些时序精度问题导致无法识别你的红外指令时，需要用以下这些参数去更改时序精度-----*/
#define CSK_IR_HW_PARAMETER_Pos                                 8
#define CSK_IR_HW_PARAMETER_Msk                                (0x3F << CSK_IR_HW_PARAMETER_Pos)

/*************NEC TX TIMING***************/
#define IR_HW_PARAMETER_NEC_TX_S1                              (1UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_TX_S3                              (2UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_TX_BIT_CYCLE                       (3UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_TX_TIME1                           (4UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_TX_TIME2                           (5UL << CSK_IR_HW_PARAMETER_Pos)
/*************NEC RX TIMING***************/
#define IR_HW_PARAMETER_NEC_RX_S1                              (6UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_S2                              (7UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_S3                              (8UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_S4                              (9UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_S5                              (10UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_TIME1                           (11UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_TIME2                           (12UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_RX_BIT_CYCLE                       (13UL << CSK_IR_HW_PARAMETER_Pos)
/*************NEC TX REPEAT TIMING***************/
#define IR_HW_PARAMETER_NEC_REPEAT_S2                          (14UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_NEC_REPEAT_S4                          (15UL << CSK_IR_HW_PARAMETER_Pos)
/*************9012 TX TIMING***************/
#define IR_HW_PARAMETER_9012_TX_S1                             (16UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_TX_S3                             (17UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_TX_BIT_CYCLE                      (18UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_TX_TIME1                          (19UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_TX_TIME2                          (20UL << CSK_IR_HW_PARAMETER_Pos)
/*************9012 RX TIMING***************/
#define IR_HW_PARAMETER_9012_RX_S1                             (21UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_S2                             (22UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_S3                             (23UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_S4                             (24UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_S5                             (25UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_TIME1                          (26UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_TIME2                          (27UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_9012_RX_BIT_CYCLE                      (28UL << CSK_IR_HW_PARAMETER_Pos)
/*************RC5 TX TIMING***************/
#define IR_HW_PARAMETER_RC5_TX_S1                              (29UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_TX_BIT_CYCLE                       (30UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_TX_TIME1                           (31UL << CSK_IR_HW_PARAMETER_Pos)
/*************RC5 RX TIMING***************/
#define IR_HW_PARAMETER_RC5_RX_S1                              (32UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_S2                              (33UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_S3                              (34UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_S4                              (35UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_S5                              (36UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_TIME1                           (37UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_TIME2                           (38UL << CSK_IR_HW_PARAMETER_Pos)
#define IR_HW_PARAMETER_RC5_RX_BIT_CYCLE                       (39UL << CSK_IR_HW_PARAMETER_Pos)

/************IR Event**************/
#define CSK_IR_EVENT_HW_SEND_COMPLETE                         (0x1UL << 0)
#define CSK_IR_EVENT_HW_RECEIVE_COMPLETE                      (0x1UL << 1)
#define CSK_IR_EVENT_HW_TX_REPEAT_COMPLETE                    (0x1UL << 2)
#define CSK_IR_EVENT_HW_RX_REPEAT_TRIGGER                     (0x1UL << 3)
#define CSK_IR_EVENT_HW_ADDRESS_VERIFY_ERROR                  (0x1UL << 4)
#define CSK_IR_EVENT_HW_COMMAND_VERIFY_ERROR                  (0x1UL << 5)
#define CSK_IR_EVENT_SW_SEND_COMPLETE                         (0x1UL << 6)
#define CSK_IR_EVENT_SW_RECEIVE_COMPLETE                      (0x1UL << 7)
#define CSK_IR_EVENT_SW_RECEIVE_TIMEOUT                       (0x1UL << 8)

typedef enum {
    IR_MODE_CUSTOM,
    IR_MODE_NEC,
    IR_MODE_TOSHIBA_9012,
    IR_MODE_PHILIPS_RC5,
} IR_MODE;

typedef void (*CSK_IR_SignalEvent_t) (uint32_t event, void* workspace);

CSK_DRIVER_VERSION CSK_IR_GetVersion(void);

int32_t  IR_Initialize(void *res, CSK_IR_SignalEvent_t cb_event, void* workspace);

int32_t  IR_Uninitialize(void* res);

int32_t  IR_PowerControl(void *res, CSK_POWER_STATE state);

int32_t  IR_Control(void *res, uint32_t control, uint32_t arg);

int32_t  IR_HW_Send(void *res, IR_MODE mode, uint16_t address, uint16_t command);

int32_t  IR_HW_Receive(void *res, IR_MODE mode, uint16_t *address, uint16_t *command);

int32_t  IR_SW_Send(void *res, const int32_t *data, uint32_t num);

int32_t  IR_SW_Receive(void *res, int32_t *data, uint32_t num);

uint32_t IR_GetTxCount(void *res);

uint32_t IR_GetRxCount(void *res);

void* IR0(void);

#endif /* INCLUDE_DRIVER_DRIVER_IR_H_ */
