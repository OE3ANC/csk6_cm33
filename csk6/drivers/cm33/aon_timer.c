/*
 * aon_timer.c
 *
 *  Created on: Mar 29, 2022
 *      Author: USER
 */

#include "aon_timer.h"
#include "aon_wdt.h"

static uint32_t aon_wdt_timer_share_use_cnt = 0;

void AON_TIMER_WDT_SHARE_Handler(void);

/// AON TIMER
static AON_TIMER_INFO aon_timer_info = {0};
static AON_TIMER_RESOURCES aon_timer_resources = {
        IP_AON_TIMER,
        IRQ_AON_TIMER_VECTOR,
        AON_TIMER_WDT_SHARE_Handler,
        &aon_timer_info,
        0,
};

#define AON_TIMER_CHECK_RESOURCES(res)  do{\
        if(res != &aon_timer_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

void* AON_TIMER(){
    return (void*)&aon_timer_resources;
}

/// AON WDT
static AON_WDT_INFO aon_wdt_info = {0};
static AON_WDT_RESOURCES aon_wdt_resources = {
        IP_AON_TIMER,
        IRQ_AON_TIMER_VECTOR,
        AON_TIMER_WDT_SHARE_Handler,
        &aon_wdt_info,
        0
};

#define AON_WDT_CHECK_RESOURCES(res)  do{\
        if(res != &aon_wdt_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

void* AON_WDT(void){
    return (void*)&aon_wdt_resources;
}

/// AON TIMER FUNCTION CONFIGURE
int32_t AON_TIMER_Initialize(void* res, HAL_AON_TIMER_SignalEvent_t cb_event, void* workspace){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    if (aon_timer->flags & AON_TIMER_FLAG_INITIALIZED){
        return CSK_DRIVER_OK;
    }

    aon_timer->info->cb_event = cb_event;
    aon_timer->info->workspace = workspace;

    aon_timer->info->mode = HAL_AON_TIMER_MODE_Normal;
    aon_timer->info->reload_cnt = 0;

    aon_timer->flags = AON_TIMER_FLAG_INITIALIZED;

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_Uninitialize(void* res){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    aon_timer->info->cb_event = NULL;
    aon_timer->info->mode = HAL_AON_TIMER_MODE_Normal;
    aon_timer->info->reload_cnt = 0;

    aon_timer->flags = 0;

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_PowerControl(void* res, CSK_POWER_STATE state){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    switch (state){
    case CSK_POWER_OFF:
        if ((aon_timer->flags & AON_TIMER_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }

        aon_wdt_timer_share_use_cnt--;

        if (aon_wdt_timer_share_use_cnt == 0){
            // Disable UART IRQ
            disable_IRQ(aon_timer->irq_num);

            aon_timer->flags &= ~AON_TIMER_FLAG_POWERED;

            // Uninstall IRQ Handler
            register_ISR(aon_timer->irq_num, NULL, NULL);
        }

        break;

    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if(!(aon_timer->flags & AON_TIMER_FLAG_INITIALIZED)){
            return CSK_DRIVER_ERROR;
        }

        if(aon_timer->flags & AON_TIMER_FLAG_POWERED){
            return CSK_DRIVER_OK;
        }

        // TODO reset peripheral

        if (aon_wdt_timer_share_use_cnt == 0){
            register_ISR(aon_timer->irq_num, aon_timer->irq_handler, NULL);
            enable_IRQ(aon_timer->irq_num);
        }

        aon_wdt_timer_share_use_cnt++;

        aon_timer->flags = AON_TIMER_FLAG_POWERED;

        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_Control(void* res, uint32_t control){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    if(!(aon_timer->flags & AON_TIMER_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    aon_timer->info->mode = control & HAL_AON_TIMER_MODE_Msk;
    switch (control & HAL_AON_TIMER_MODE_Msk){
    case HAL_AON_TIMER_MODE_Wrapping:
        aon_timer->reg->REG_OSTIMER_CTRL.bit.WRAP_MODE = 0x1;
        aon_timer->reg->REG_OSTIMER_CTRL.bit.REPEAT_MODE = 0x0;
        break;
    case HAL_AON_TIMER_MODE_Repeat:
        aon_timer->reg->REG_OSTIMER_CTRL.bit.WRAP_MODE = 0x0;
        aon_timer->reg->REG_OSTIMER_CTRL.bit.REPEAT_MODE = 0x1;
        break;
    case HAL_AON_TIMER_MODE_Normal:
        aon_timer->reg->REG_OSTIMER_CTRL.bit.WRAP_MODE = 0x0;
        aon_timer->reg->REG_OSTIMER_CTRL.bit.REPEAT_MODE = 0x0;
        break;
    }

    switch (control & HAL_AON_TIMER_INTERRUPT_Msk){
    case HAL_AON_TIMER_INTERRUPT_Enabled:
        aon_timer->reg->REG_AON_TIMER_IRQ_MASK.bit.OSTIMER_MASK = 0x1;
        break;
    case HAL_AON_TIMER_INTERRUPT_Disabled:
        aon_timer->reg->REG_AON_TIMER_IRQ_MASK.bit.OSTIMER_MASK = 0x0;
        break;
    }

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_SetTimerPeriodByCount(void* res, uint32_t count){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    if(!(aon_timer->flags & AON_TIMER_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    aon_timer->reg->REG_OSTIMER_CTRL.bit.LOADVAL = count & AON_TIMER_LOAD_VALUE_MASK;
    aon_timer->reg->REG_OSTIMER_CTRL.bit.LOAD = 0x1;

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_StartTimer(void* res){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    if(!(aon_timer->flags & AON_TIMER_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    aon_timer->reg->REG_OSTIMER_CTRL.bit.ENABLE = 0x1;
    while(!aon_timer->reg->REG_OSTIMER_CTRL.bit.ENABLED);

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_ReadTimerCount(void* res, uint32_t *count){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    *count = aon_timer->reg->REG_OSTIMER_CURVAL.all & AON_TIMER_LOAD_VALUE_MASK;

    return CSK_DRIVER_OK;
}

int32_t AON_TIMER_StopTimer(void* res){
    AON_TIMER_CHECK_RESOURCES(res);
    AON_TIMER_RESOURCES *aon_timer = (AON_TIMER_RESOURCES*) res;

    if(!(aon_timer->flags & AON_TIMER_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    aon_timer->reg->REG_OSTIMER_CTRL.bit.ENABLE = 0x0;
    while(aon_timer->reg->REG_OSTIMER_CTRL.bit.ENABLED);

    return CSK_DRIVER_OK;
}

static void __AON_TIMER_Handler__(AON_TIMER_RESOURCES* aon_timer){
    aon_timer->reg->REG_AON_TIMER_IRQ_CLR.bit.OSTIMER_CLR = 0x1;
//    while(aon_timer->reg->REG_OSTIMER_CTRL.bit.LOADED);

    if (aon_timer->info->cb_event){
        aon_timer->info->cb_event(HAL_AON_TIMER_EVENT_COMPLETE, aon_timer->info->workspace);
    }
}

/// AON WDT FUNCTION CONFIGURE

int32_t AON_WDT_Initialize(void* res, HAL_AON_WDT_SignalEvent_t callback, void* workspace){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    if (aon_wdt->flags & AON_WDT_FLAG_INITIALIZED){
        return CSK_DRIVER_OK;
    }

    aon_wdt->info->cb_event = callback;
    aon_wdt->info->workspace = workspace;

    aon_wdt->flags = AON_WDT_FLAG_INITIALIZED;

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_Uninitialize(void* res){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    aon_wdt->info->cb_event = NULL;

    aon_wdt->flags = 0;

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_PowerControl(void* res, CSK_POWER_STATE state){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    switch (state){
    case CSK_POWER_OFF:
        if ((aon_wdt->flags & AON_WDT_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }

        aon_wdt_timer_share_use_cnt--;

        if (aon_wdt_timer_share_use_cnt == 0){
            // Disable UART IRQ
            // disable_IRQ(aon_wdt->irq_num);

            aon_wdt->flags &= ~AON_WDT_FLAG_POWERED;

            // Uninstall IRQ Handler
            // register_ISR(aon_wdt->irq_num, NULL, NULL);
        }

        break;

    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if(!(aon_wdt->flags & AON_WDT_FLAG_INITIALIZED)){
            return CSK_DRIVER_ERROR;
        }

        if(aon_wdt->flags & AON_WDT_FLAG_POWERED){
            return CSK_DRIVER_OK;
        }

        // TODO reset peripheral

        if (aon_wdt_timer_share_use_cnt == 0){
            // register_ISR(aon_wdt->irq_num, aon_wdt->irq_handler, NULL);
            // enable_IRQ(aon_wdt->irq_num);
        }

        aon_wdt_timer_share_use_cnt++;

        aon_wdt->flags = AON_TIMER_FLAG_POWERED;

        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_Control(void* res, uint32_t control, uint32_t arg){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    if(!(aon_wdt->flags & AON_WDT_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    switch (control) {
    case HAL_AON_WDT_TIME_CFG:
        aon_wdt->reg->REG_AONWDTTIMER_LOADVAL.all = arg & AON_WDT_LOAD_VALUE_MASK;
        break;

    case HAL_AON_WDT_INTERRUPT_EN:
        if (arg){
            aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.WDT_INT_MASK = 0x1;
        } else {
            aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.WDT_INT_MASK = 0x0;
        }
        break;

    case HAL_AON_WDT_RESET_PMU_CTRL:
        if (arg == reset_pmu_ctrl){
            aon_wdt->reg->REG_RESET_PMU_EN.all = AON_WDT_RESET_PMU_CTRL;
        } else if (arg == reset_dbb_domain){
            aon_wdt->reg->REG_RESET_PMU_EN.all = AON_WDT_RESET_DBB;
        }
    }

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_Enable(void* res){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    if(!(aon_wdt->flags & AON_WDT_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    // Start
    aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.START = 0x1;
    while(!aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.WDENABLED);

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_Refresh(void* res){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    if(!(aon_wdt->flags & AON_WDT_FLAG_POWERED)){
        return CSK_DRIVER_ERROR;
    }

    aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.RELOAD = 0x1;

    return CSK_DRIVER_OK;
}

int32_t AON_WDT_Disable(void* res){
    AON_WDT_CHECK_RESOURCES(res);
    AON_WDT_RESOURCES *aon_wdt = (AON_WDT_RESOURCES*) res;

    // Release protect
    while(aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.STOP_PROTECTED){
        aon_wdt->reg->REG_STOP_PROTECT.all = AON_WDT_STOP_PROTECT_RELEASE;
    }

    // Stop
    aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.STOP = 0x1;
    while(aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.WDENABLED);

    // Lock protect
    while(!aon_wdt->reg->REG_AON_WDTTIMER_CTRL.bit.STOP_PROTECTED){
        aon_wdt->reg->REG_STOP_PROTECT.all = AON_WDT_STOP_PROTECT_LOCK;
    }

    return CSK_DRIVER_OK;
}

uint32_t AON_WDT_GetMaxValue(void){
    return AON_TIMER_LOAD_VALUE_MASK;
}

static void __AON_WDT_Handler__(AON_WDT_RESOURCES* aon_wdt){
    aon_wdt->reg->REG_AON_TIMER_IRQ_CLR.bit.AONWDT_CLR = 0x1;

    if (aon_wdt->info->cb_event){
        aon_wdt->info->cb_event(aon_wdt->info->workspace);
    }
    while(aon_wdt->reg->REG_AON_TIMER_IRQ_CAUSE.bit.WDT_WAKEUP_STATUS);
}

void AON_TIMER_WDT_SHARE_Handler(void){
    if (IP_AON_TIMER->REG_AON_TIMER_IRQ_CAUSE.bit.IRQ_CAUSE &\
            IP_AON_TIMER->REG_AON_TIMER_IRQ_CAUSE.bit.OSTIMER_STATUS){
        // AON TIMER
        __AON_TIMER_Handler__(&aon_timer_resources);
    }

    if (IP_AON_TIMER->REG_AON_TIMER_IRQ_CAUSE.bit.IRQ_CAUSE &\
            IP_AON_TIMER->REG_AON_TIMER_IRQ_CAUSE.bit.WDT_WAKEUP_STATUS){
        // AON WDT
        __AON_WDT_Handler__(&aon_wdt_resources);
    }
}
