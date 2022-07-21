#include "rtc.h"

#define CSK_RTC_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,01)

// Driver Version
static const CSK_DRIVER_VERSION rtc_driver_version =
{
    CSK_RTC_API_VERSION,
    CSK_RTC_DRV_VERSION
};

static RTC_INFO info = {0};

static void RTC_Handler(void);
static RTC_RESOURCES rtc_resources = {
    CSK_AON_RTC,
    IRQ_AON_RTC_VECTOR,
    RTC_Handler,
    &info,
};

#define CHECK_RESOURCES(res)  do{\
        if(res != &rtc_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

void* RTC(){
    return (void*)&rtc_resources;
}

CSK_DRIVER_VERSION
RTC_GetVersion(void)
{
    return rtc_driver_version;
}

int32_t
RTC_Initialize(void *res, CSK_RTC_SignalEvent_t cb_event, void* workspace)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;
    if(rtc->info->flag & RTC_FLAG_INITIALIZED){
        return CSK_DRIVER_OK;
    }

    rtc->info->alarm_hour = 0;
    rtc->info->alarm_min = 0;
    rtc->info->alarm_sec = 0;

    rtc->info->cb_event = cb_event;
    rtc->info->workspace = workspace;

    rtc->info->flag = RTC_FLAG_INITIALIZED;

    //硬件上rtc和keysense使用同一个寄存器配置时钟源
    IP_PMUCTRL->REG_AON_CRM_CTRL1.bit.ENA_KEYSENSE_CLK = 1;
    IP_PMUCTRL->REG_AON_CRM_CTRL1.bit.SEL_KEYSENSE_CLK = 0;

    return CSK_DRIVER_OK;
}

int32_t
RTC_Uninitialize(void *res)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    rtc->info->alarm_hour = 0;
    rtc->info->alarm_min = 0;
    rtc->info->alarm_sec = 0;

    rtc->info->cb_event = NULL;
    rtc->info->workspace = NULL;

    rtc->info->flag = 0;

    return CSK_DRIVER_OK;
}

int32_t
RTC_PowerControl(void* res, CSK_POWER_STATE state)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    switch (state)
    {
    case CSK_POWER_OFF:
        if((rtc->info->flag & RTC_FLAG_INITIALIZED) == 0U){
            return CSK_DRIVER_ERROR;
        }

        disable_IRQ(rtc->irq_num);

        register_ISR(rtc->irq_num, NULL, NULL);

        // clear RTC interrupt status
        rtc->reg->STATUS |= 0xff;

        rtc->info->flag &= (~RTC_FLAG_POWERED);

        break;

    case CSK_POWER_LOW:
        break;

    case CSK_POWER_FULL:
        if(!(rtc->info->flag & RTC_FLAG_INITIALIZED)){
            return CSK_DRIVER_ERROR;
        }
        if(rtc->info->flag & RTC_FLAG_POWERED){
            return CSK_DRIVER_OK;
        }

        // TODO reset pheripheral (importance)
        register_ISR(rtc->irq_num, rtc->irq_handler, NULL);
        enable_IRQ(rtc->irq_num);

        rtc->info->flag |= RTC_FLAG_POWERED;

        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t
RTC_SetTime(void* res, CSK_RTC_TIME* stime)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    if((rtc->info->flag & RTC_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    // check the parameter
    if((stime->day > RTC_SET_TIME_DAY_MARGIN) ||\
            (stime->hour > RTC_SET_TIME_HOUR_MARGIN) ||\
            (stime->min > RTC_SET_TIME_MIN_MARGIN) ||\
            (stime->sec > RTC_SET_TIME_SEC_MARGIN)){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // wait register update
    while(!(rtc->reg->STATUS & STATUS_WRITEDONE));

    rtc->reg->CNTR = ((stime->day << 17) |\
            (stime->hour << 12) |\
            (stime->min << 6) |\
            (stime->sec));

    return CSK_DRIVER_OK;
}

int32_t
RTC_GetTime(void *res, CSK_RTC_TIME* stime)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    stime->day = (rtc->reg->CNTR & CNTR_DAY) >> 17;
    stime->hour = (rtc->reg->CNTR & CNTR_HOUR) >> 12;
    stime->min = (rtc->reg->CNTR & CNTR_MIN) >> 6;
    stime->sec = (rtc->reg->CNTR & CNTR_SEC);

    return CSK_DRIVER_OK;
}

int32_t
RTC_SetAlarm(void* res, CSK_RTC_ALARM* salarm)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    if((rtc->info->flag & RTC_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    // check the parameter
    if((salarm->hour > RTC_SET_ALARM_HOUR_MARGIN) ||\
            (salarm->min > RTC_SET_ALARM_MIN_MARGIN) ||\
            (salarm->sec > RTC_SET_ALARM_SEC_MARGIN)){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    rtc->info->alarm_hour = salarm->hour;
    rtc->info->alarm_min = salarm->min;
    rtc->info->alarm_sec = salarm->sec;

    // wait register update
    while(!(rtc->reg->STATUS & STATUS_WRITEDONE));

    rtc->reg->ALARM = ((salarm->hour << 12) |\
            (salarm->min << 6) |\
            (salarm->sec));

    return CSK_DRIVER_OK;
}

int32_t
RTC_GetAlarm(void* res, CSK_RTC_ALARM* salarm)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    salarm->hour = rtc->info->alarm_hour;
    salarm->min = rtc->info->alarm_min;
    salarm->sec = rtc->info->alarm_sec;

    return CSK_DRIVER_OK;
}

int32_t
RTC_Control(void* res, uint32_t control, uint32_t arg)
{
    CHECK_RESOURCES(res);
    RTC_RESOURCES *rtc = (RTC_RESOURCES*)res;

    // wait register update
    while(!(rtc->reg->STATUS & STATUS_WRITEDONE));
    switch (control)
    {
    case CSK_RTC_CTRL_EN:
        // RTC ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_RTC_EN;
        }
        // RTC OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_RTC_EN);
        }
        break;
    case CSK_RTC_CTRL_ALARM_WAKEUP:
    	CSK_AON_RTC->CTRL |= 0x02;
        break;
    case CSK_RTC_CTRL_ALARM_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= CTRL_ALARM_WAKEUP;
        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_ALARM_INT;
            rtc->info->flag |= RTC_FLAG_ALARM;
        }
        // INT OFF
        else if (arg == 0)
        {

            CSK_AON_RTC->CTRL &= (~CTRL_ALARM_INT);
            rtc->info->flag &= (~RTC_FLAG_ALARM);
        }
        break;
    case CSK_RTC_CTRL_DAY_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= STATUS_DAY;

        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_DAY;
            rtc->info->flag |= RTC_FLAG_DAY_INT;
        }
        // INT OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_DAY);
            rtc->info->flag &= (~RTC_FLAG_DAY_INT);
        }
        break;
    case CSK_RTC_CTRL_HOUR_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= STATUS_HOUR;

        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_HOUR;
            rtc->info->flag |= RTC_FLAG_HOUR_INT;
        }
        // INT OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_HOUR);
            rtc->info->flag &= (~RTC_FLAG_HOUR_INT);
        }
        break;
    case CSK_RTC_CTRL_MIN_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= STATUS_MIN;

        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_MIN;
            rtc->info->flag |= RTC_FLAG_MIN_INT;
        }
        // INT OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_MIN);
            rtc->info->flag &= (~RTC_FLAG_MIN_INT);
        }
        break;
    case CSK_RTC_CTRL_SEC_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= STATUS_SEC;

        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_SEC;
            rtc->info->flag |= RTC_FLAG_SEC_INT;
        }
        // INT OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_SEC);
            rtc->info->flag &= (~RTC_FLAG_SEC_INT);
        }
        break;
    case CSK_RTC_CTRL_HSEC_INT:
        // write 1 clear
        CSK_AON_RTC->STATUS |= STATUS_HSEC;

        // INT ON
        if (arg == 1)
        {
            CSK_AON_RTC->CTRL |= CTRL_HSEC;
            rtc->info->flag |= RTC_FLAG_HSEC_INT;
        }
        // INT OFF
        else if (arg == 0)
        {
            CSK_AON_RTC->CTRL &= (~CTRL_HSEC);
            rtc->info->flag &= (~RTC_FLAG_HSEC_INT);
        }
        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

static void
__RTC_Handler__(RTC_RESOURCES *rtc){
    uint32_t status;
    uint32_t event = 0;
    status = rtc->reg->STATUS;
    rtc->reg->STATUS = (status & 0xff);

    // Status register hsec set 1 and user enable current interrupt
    if((status & STATUS_HSEC) && (rtc->info->flag & RTC_FLAG_HSEC_INT)){
        event |= CSK_RTC_EVENT_HSEC_INT;
    }

    if((status & STATUS_SEC) && (rtc->info->flag & RTC_FLAG_SEC_INT)){
        event |= CSK_RTC_EVENT_SEC_INT;
    }

    if((status & STATUS_MIN) && (rtc->info->flag & RTC_FLAG_MIN_INT)){
        event |= CSK_RTC_EVENT_MIN_INT;
    }

    if((status & STATUS_HOUR) && (rtc->info->flag & RTC_FLAG_HOUR_INT)){
        event |= CSK_RTC_EVENT_HOUR_INT;
    }

    if((status & STATUS_DAY) && (rtc->info->flag & RTC_FLAG_DAY_INT)){
        event |= CSK_RTC_EVENT_DAY_INT;
    }

    if((status & STATUS_ALARM_INT) && (rtc->info->flag & RTC_FLAG_ALARM)){
        event |= CSK_RTC_EVENT_ALARM_INT;
    }

    if(rtc->info->cb_event && event != 0){
        rtc->info->cb_event(event, rtc->info->workspace);
    }
}

static void
RTC_Handler(void){
    __RTC_Handler__(&rtc_resources);
}
