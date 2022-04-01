#ifndef __WDT_AE210P_H
#define __WDT_AE210P_H

#include "wdt_reg_venus.h"

#include "Driver_WDT.h"

#include "venus_cp.h"

#define VENUS_WDT0                         ((WDT_RegDef *)CP_WATCHDOG_BASE)

// WDT flags
#define WDT_FLAG_INITIALIZED                (1U << 0)
#define WDT_FLAG_POWERED                    (1U << 1)
#define WDT_FLAG_CONFIGURED                 (1U << 2)

#define WDT_MAGIC_WRITE_PROTECTION          (0x5aa5)
#define WDT_MAGIC_RESTARTING                (0xcafe)

// WDT Information (Run-Time)
typedef struct _WDT_INFO
{
    uint32_t flags;         // WDT driver flags
    uint32_t int_stage;
    uint32_t reset_stage;
} WDT_INFO;

// WDT Resources definitions
typedef struct
{
    WDT_RegDef *reg;          // Pointer to WDT peripheral
    uint32_t irq_num;
    void (*irq_handler)(void);
    WDT_INFO *info;         // Run-Time Information
}const WDT_RESOURCES;

#endif /* __WDT_AE210P_H */
