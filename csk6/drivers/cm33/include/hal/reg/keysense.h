#ifndef __KEYSENSE_IC_CSK6001_H
#define __KEYSENSE_IC_CSK6001_H

#include "cmn_sysctrl_reg_venus.h"
#include "keysense_reg_venus.h"

#include "venus_ap.h"
#include "Driver_KEYSENSE.h"

#include "dma.h"


#define VENUS_KEYSENSE             ((KEYSENSE_RegDef*)  AON_KEYSENSE_BASE)


// KEYSENSE flags
#define KEYSENSE_FLAG_INITIALIZED          (1U << 0)
#define KEYSENSE_FLAG_POWERED              (1U << 1)
#define KEYSENSE_FLAG_CONFIGURED           (1U << 2)



typedef struct _KEYSENSE_INFO
{
	uint32_t flags;         		   	// KEYSENSE driver flags
	CSK_KEYSENSE_SignalEvent_t cb_event[CSK_KEYSENSE_MODE_NUM]; 	// Event callback
	uint32_t clk_source[2];
	void (*origin_IRQ_handler)(void);
} KEYSENSE_INFO;



// KEYSENSE Resource Configuration
typedef const struct
{
    KEYSENSE_RegDef* reg;                	  // KEYSENSE register interface
    uint32_t irq_num;
    void (*irq_handler)(void);
    KEYSENSE_INFO* info;               	  // Run-Time control information
    uint32_t user_param;
}KEYSENSE_RESOURCES;



#endif /* __KEYSENSE_IC_CSK6001_H */
