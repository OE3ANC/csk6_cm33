#ifndef __PWR_IC_CSK6001_H
#define __PWR_IC_CSK6001_H

#include "cmn_sysctrl_reg_venus.h"
#include "aon_pmuctrl_reg_venus.h"

#include "venus_ap.h"
#include "Driver_PWR.h"


#define VENUS_PWR             ((AON_RegDef*)  AON_PMUCTRL_BASE)


// PWR flags
#define PWR_FLAG_INITIALIZED          (1U << 0)
#define PWR_FLAG_POWERED              (1U << 1)
#define PWR_FLAG_CONFIGURED           (1U << 2)


#define ENABLE 1
#define DISABLE 0


#define CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE_Pos		16
#define CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE_Msk		(0xFF<<CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE_Pos)
#define CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE			CSK_PWR_REG_AON_STATUS_3_SYSRST_CAUSE_Msk

#define CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION_Pos		12
#define CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION_Msk		(0x03<<CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION_Pos)
#define CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION			CSK_PWR_REG_AON_STATUS_3_BOOT_OPTION_Msk

#define CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSE_Pos			3
#define CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSEN_Msk		(0x07<<CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSE_Pos)
#define CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSE				CSK_PWR_REG_AON_STATUS_3_WAKE_CAUSEN_Msk

#define CSK_PWR_REG_WAKE_SELECT_Pos						0
#define CSK_PWR_REG_WAKE_SELECT_Msk						(0xFFF<<CSK_PWR_WAKE_SELECT_Pos)
#define CSK_PWR_REG_WAKE_SELECT							CSK_PWR_WAKE_SELECT_Msk

#define CSK_PWR_REG_GPIOB_WAKEUP_Pos					0

#define CSK_PWR_REG_GPIOB_POLARITY_Pos					12
#define CSK_PWR_REG_GPIOB_POLARITY_Msk					(0x3F<<CSK_PWR_REG_GPIOB_POLARITY_Pos)
#define CSK_PWR_REG_GPIOB_POLARITY						CSK_PWR_REG_GPIOB_POLARITY_Msk



typedef struct _PWR_INFO
{
	uint32_t flags;         		   	// PWR driver flags
	CSK_PWR_SignalEvent_t cb_event; 	// Event callback
	uint32_t clk_source[2];
	void (*origin_IRQ_handler)(void);
} PWR_INFO;



// PWR Resource Configuration
typedef const struct
{
	AON_RegDef* reg;                	  // PWR register interface
    uint32_t irq_num;
    void (*irq_handler)(void);
    PWR_INFO* info;               	  // Run-Time control information
    uint32_t user_param;
}PWR_RESOURCES;



#endif /* __PWR_IC_CSK6001_H */
