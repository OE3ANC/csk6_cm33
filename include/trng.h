#ifndef __TRNG_IC_CSK6001_H
#define __TRNG_IC_CSK6001_H

#include "cmn_sysctrl_reg_venus.h"
#include "trng_reg_venus.h"

#include "venus_ap.h"
#include "Driver_TRNG.h"


#define VENUS_TRNG             ((TRNG_RegDef*)  TRNG_BASE)


// TRNG flags
#define TRNG_FLAG_INITIALIZED          (1U << 0)
#define TRNG_FLAG_POWERED              (1U << 1)
#define TRNG_FLAG_CONFIGURED           (1U << 2)


#define TRNG_REG_PROTECK_KEY 0xF5000000

#define CSK_TRNG_CONIFG_REG_DELAYTIME_Pos            0
#define CSK_TRNG_CONIFG_REG_HOTTIME_Pos              4
#define CSK_TRNG_CONIFG_REG_COLDTIME_Pos             8

#define ENABLE 1
#define DISABLE 0


typedef struct _TRNG_INFO
{
	uint32_t flags;         		   	// TRNG driver flags
	CSK_TRNG_SignalEvent_t cb_event; 	// Event callback
	uint32_t clk_source[2];
	void (*origin_IRQ_handler)(void);
} TRNG_INFO;



// TRNG Resource Configuration
typedef const struct
{
    TRNG_RegDef* reg;                	  // TRNG register interface
    uint32_t irq_num;
    void (*irq_handler)(void);
    TRNG_INFO* info;               	  // Run-Time control information
    uint32_t user_param;
}TRNG_RESOURCES;



#endif /* __TRNG_IC_CSK6001_H */
