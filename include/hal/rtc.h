#ifndef __RTC_VENUS_H
#define __RTC_VENUS_H

#include "venus_ap.h"
#include "Driver_RTC.h"

/*****************************************************************************
 *  HAL Level : Interrupt                                                    *
 ****************************************************************************/
#define CSK32_HWINT_ID(hw)          CSK32_INT_H##hw
#define CSK32_HWINT(hw)             CSK32_HWINT_ID(hw)

//#ifndef BIT1
#define BIT1(n)                      ((unsigned int) 1 << (n))
#define BITS2(m,n)                  (BIT1(m) | BIT1(n) )

#define BITS(m,n)                   (~(BIT1(m)-1) & ((BIT1(n) - 1) | BIT1(n)))
//#endif /* BIT1 */

#define ATCRTC100_HALF_SECOND_SUPPORT   (0)
#define ATCRTC100_DAY_BITS              (5)        // for counting up to 2^5 = 32 days

// id rev register bit define
#define ATCRTC100_ID                BITS(12,31)    // ID number for ATCRTC100
#define ATCRTC100_MAJOR             BITS(4,11)     // Enable alarm wakeup signal
#define ATCRTC100_MINOR             BITS(0,3)      // Enable alarm interrupt

// counter register bit define
// Day passed after RTC enabled, BITS(17,21), counting up to 2^5 = 32 days
#define CNTR_DAY                    BITS(17,(17 + (ATCRTC100_DAY_BITS - 1)))
#define CNTR_HOUR                   BITS(12,16)    // Hour field of current time; range: 0 ~ 23
#define CNTR_MIN                    BITS(6,11)     // Minute field of current time; range: 0 ~ 59
#define CNTR_SEC                    BITS(0,5)      // Second field of current time; range: 0 ~ 59

// alarm register bit define
#define ALARM_HOUR                  BITS(12,16)    // Hour field of current time; range: 0 ~ 23
#define ALARM_MIN                   BITS(6,11)     // Minute field of current time; range: 0 ~ 59
#define ALARM_SEC                   BITS(0,5)      // Second field of current time; range: 0 ~ 59

// ctrl register bit define
#define CTRL_RTC_EN                 BIT1(0)         // Enable RTC
#define CTRL_ALARM_WAKEUP           BIT1(1)         // Enable alarm wakeup signal
#define CTRL_ALARM_INT              BIT1(2)         // Enable alarm interrupt
#define CTRL_DAY                    BIT1(3)         // Enable day interrupt
#define CTRL_HOUR                   BIT1(4)         // Enable hour interrupt
#define CTRL_MIN                    BIT1(5)         // Enable minute interrupt
#define CTRL_SEC                    BIT1(6)         // Enable second interrupt
#define CTRL_HSEC                   BIT1(7)         // Enable half-second interrupt

// status register bit define
#define STATUS_ALARM_INT            BIT1(2)         // Alarm interrupt status
#define STATUS_DAY                  BIT1(3)         // Day interrupt status
#define STATUS_HOUR                 BIT1(4)         // Hour interrupt status
#define STATUS_MIN                  BIT1(5)         // Minute interrupt status
#define STATUS_SEC                  BIT1(6)         // Second interrupt status
#define STATUS_HSEC                 BIT1(7)         // Half-second interrupt status
#define STATUS_WRITEDONE            BIT1(16)        // the synchronization progress of RTC register updates

typedef struct _VENUS_RTC_RegDef
{
    unsigned int IDREV;                /* 0x00 ID and Revision Register */
    unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
    unsigned int CNTR;                 /* 0x10 Counter Register */
    unsigned int ALARM;                /* 0x14 Alarm Register */
    unsigned int CTRL;                 /* 0x18 Control Register */
    unsigned int STATUS;               /* 0x1C Status Register */
} VENUS_RTC_RegDef;

#define CSK_AON_RTC              ((VENUS_RTC_RegDef *)  AON_RTC_BASE)

#define RTC_FLAG_INITIALIZED        (1UL << 0)
#define RTC_FLAG_POWERED            (1UL << 1)

#define RTC_FLAG_DAY_INT            (1UL << 2)
#define RTC_FLAG_HOUR_INT           (1UL << 3)
#define RTC_FLAG_MIN_INT            (1UL << 4)
#define RTC_FLAG_SEC_INT            (1UL << 5)
#define RTC_FLAG_HSEC_INT           (1UL << 6)

#define RTC_FLAG_ALARM              (1UL << 7)


// PARAMETER MARGIN
#define RTC_SET_TIME_DAY_MARGIN     (32UL)
#define RTC_SET_TIME_HOUR_MARGIN    (24UL)
#define RTC_SET_TIME_MIN_MARGIN     (60UL)
#define RTC_SET_TIME_SEC_MARGIN     (60UL)

#define RTC_SET_ALARM_HOUR_MARGIN   (24UL)
#define RTC_SET_ALARM_MIN_MARGIN    (60UL)
#define RTC_SET_ALARM_SEC_MARGIN    (60UL)

typedef struct {
    CSK_RTC_SignalEvent_t cb_event;
    void* workspace;
    uint32_t alarm_hour;
    uint32_t alarm_min;
    uint32_t alarm_sec;
    uint32_t flag;
} RTC_INFO;

typedef struct
{
    VENUS_RTC_RegDef *reg;
    uint32_t irq_num;
    void (*irq_handler)(void);
    RTC_INFO *info;
} const RTC_RESOURCES;

#endif /* __RTC_VENUS_H */
