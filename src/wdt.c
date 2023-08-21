#include "wdt.h"

#define CSK_WDT_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

// Driver version
static const CSK_DRIVER_VERSION
wdt_driver_version = { CSK_WDT_API_VERSION, CSK_WDT_DRV_VERSION };

static WDT_INFO wdt0_info = { 0 };
static WDT_INFO aon_wdt_info = { 0 };

static void WDT_IRQ_Handler(void);
const WDT_RESOURCES wdt0_resources = {
        VENUS_WDT0,
        IRQ_WDT_VECTOR,
        WDT_IRQ_Handler,
        &wdt0_info
};

static void AON_WDT_IRQ_Handler(void);
static const WDT_RESOURCES aon_wdt_resources = {
        VENUS_AON_WDT,
        IRQ_AON_IWDT_VECTOR,
        AON_WDT_IRQ_Handler,
        &aon_wdt_info,
};

static CP_WDT_RESOURCES cp_wdt_resources = {
        .cp_wdt_clbk = NULL,
        .cp_wdt_para = NULL,
};

#define CHECK_RESOURCES(res)  do{\
        if((res != &wdt0_resources) && (res != &aon_wdt_resources)){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

void* WDT(void){
    return (void*)&wdt0_resources;
}

//void* AON_WDT(void){
//    return (void*)&aon_wdt_resources;
//}

CSK_DRIVER_VERSION
WDT_GetVersion(void)
{
    return wdt_driver_version;
}


int32_t
WDT_Initialize(void* res)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    if (wdt->info->flags & WDT_FLAG_INITIALIZED) {
        // Driver is already initialized
        return CSK_DRIVER_OK;
    }

    wdt->info->int_stage = 0x0;
    wdt->info->reset_stage = 0x0;
    wdt->info->flags = WDT_FLAG_INITIALIZED;
    return CSK_DRIVER_OK;
}

int32_t
WDT_Uninitialize(void* res)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    // Reset WDT status flags
    wdt->info->int_stage = 0x0;
    wdt->info->reset_stage = 0x0;
    wdt->info->flags = 0U;

    return CSK_DRIVER_OK;
}

/**
 \fn          int32_t wdt_power_control (CSK_POWER_STATE state)
 \brief       Control WDT Interface Power.
 \param[in]   state  Power state
 \return      \ref execution_status
 */
int32_t
WDT_PowerControl(void* res, CSK_POWER_STATE state)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    switch (state) {
    case CSK_POWER_OFF:
        if ((wdt->info->flags & WDT_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }
        // Disable IRQ and clear pending interrupts
        disable_IRQ(wdt->irq_num);

        // Clear driver variables

        // Disable peripheral interrupt
        register_ISR(wdt->irq_num, NULL, NULL);

        // Disable WDT
        wdt->reg->REG_CTRL.bit.EN = 0x0;

        // Disable peripheral clock

        wdt->info->flags &= ~WDT_FLAG_POWERED;
        break;
    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    case CSK_POWER_FULL:
        // check API protocol
        if ((wdt->info->flags & WDT_FLAG_INITIALIZED) == 0U) {
            return CSK_DRIVER_ERROR;
        }
        if ((wdt->info->flags & WDT_FLAG_POWERED) != 0U) {
            return CSK_DRIVER_OK;
        }

        // Enable peripheral clock

        // Reset peripheral

        wdt->info->flags = WDT_FLAG_POWERED | WDT_FLAG_INITIALIZED;
        register_ISR(wdt->irq_num, wdt->irq_handler, NULL);
        enable_IRQ(wdt->irq_num);

        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }
    return CSK_DRIVER_OK;
}

/**
 \brief             Control WDT Interface.
 *
 * @param control   This must include all three parts of CSK_WDT_CLKSRC_X,
 *                  CSK_WDT_INTTIME_X and CSK_WDT_RSTTIME_X.
 */
int32_t
WDT_Control(void* res, uint32_t control)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    uint32_t ctrl_val = 0U;
    uint32_t compound_mask = 0U;

    // clock source
    switch (control & CSK_WDT_CLKSRC_Msk) {
    case CSK_WDT_CLKSRC_APB:
        ctrl_val |= (0x1 << 1);
        break;
    case CSK_WDT_CLKSRC_EXT:
        ctrl_val |= (0x0 << 1);
        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // interrupt interval time
    switch (control & CSK_WDT_INTTIME_Msk) {
    case CSK_WDT_INTTIME_6:
        ctrl_val |= (0x0 << 4);
        break;
    case CSK_WDT_INTTIME_8:
        ctrl_val |= (0x1 << 4);
        break;
    case CSK_WDT_INTTIME_10:
        ctrl_val |= (0x2 << 4);
        break;
    case CSK_WDT_INTTIME_11:
        ctrl_val |= (0x3 << 4);
        break;
    case CSK_WDT_INTTIME_12:
        ctrl_val |= (0x4 << 4);
        break;
    case CSK_WDT_INTTIME_13:
        ctrl_val |= (0x5 << 4);
        break;
    case CSK_WDT_INTTIME_14:
        ctrl_val |= (0x6 << 4);
        break;
    case CSK_WDT_INTTIME_15:
        ctrl_val |= (0x7 << 4);
        break;
    case CSK_WDT_INTTIME_17:
        ctrl_val |= (0x8 << 4);
        break;
    case CSK_WDT_INTTIME_19:
        ctrl_val |= (0x9 << 4);
        break;
    case CSK_WDT_INTTIME_21:
        ctrl_val |= (0xa << 4);
        break;
    case CSK_WDT_INTTIME_23:
        ctrl_val |= (0xb << 4);
        break;
    case CSK_WDT_INTTIME_25:
        ctrl_val |= (0xc << 4);
        break;
    case CSK_WDT_INTTIME_27:
        ctrl_val |= (0xd << 4);
        break;
    case CSK_WDT_INTTIME_29:
        ctrl_val |= (0xe << 4);
        break;
    case CSK_WDT_INTTIME_31:
        ctrl_val |= (0xf << 4);
        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // interval time of reset stage
    switch (control & CSK_WDT_RSTTIME_Msk) {
    case CSK_WDT_RSTTIME_7:
        ctrl_val |= (0x0 << 8);
        break;
    case CSK_WDT_RSTTIME_8:
        ctrl_val |= (0x1 << 8);
        break;
    case CSK_WDT_RSTTIME_9:
        ctrl_val |= (0x2 << 8);
        break;
    case CSK_WDT_RSTTIME_10:
        ctrl_val |= (0x3 << 8);
        break;
    case CSK_WDT_RSTTIME_11:
        ctrl_val |= (0x4 << 8);
        break;
    case CSK_WDT_RSTTIME_12:
        ctrl_val |= (0x5 << 8);
        break;
    case CSK_WDT_RSTTIME_13:
        ctrl_val |= (0x6 << 8);
        break;
    case CSK_WDT_RSTTIME_14:
        ctrl_val |= (0x7 << 8);
        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    compound_mask =  BITSET_0X010_RSTTIME | BITSET_0X010_INTTIME | BITSET_0X010_CLKSEL;

    wdt->info->int_stage = (control & CSK_WDT_INTTIME_Msk);
    wdt->info->reset_stage = (control & CSK_WDT_RSTTIME_Msk);

    // enable write and write register
    wdt->reg->REG_WREN.all = WDT_MAGIC_WRITE_PROTECTION;
    wdt->reg->REG_CTRL.all = (wdt->reg->REG_CTRL.all & (~compound_mask)) | ctrl_val;

    // Set configured flag
    wdt->info->flags |= WDT_FLAG_CONFIGURED;
    return CSK_DRIVER_OK;
}

/*!
 * @brief Enable watch dog timer.
 */
int32_t
WDT_Enable(void* res)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    uint32_t ctrl_val = 0U;
    uint32_t compound_mask = 0U;

    // check API protocol
    if ((wdt->info->flags & WDT_FLAG_POWERED) == 0U) {
        // WDT not powered
        return CSK_DRIVER_ERROR;
    }

    // rst enable
    ctrl_val |= (0x1 << 3);
    // int enable
    ctrl_val |= (0x1 << 2);
    // enable
    ctrl_val |= 0x1;

    compound_mask = BITSET_0X010_RSTEN | BITSET_0X010_INTEN | BITSET_0X010_EN;

    // enable write and write control register
    wdt->reg->REG_WREN.all = WDT_MAGIC_WRITE_PROTECTION;
    wdt->reg->REG_CTRL.all = (wdt->reg->REG_CTRL.all & (~compound_mask)) | ctrl_val;

    return CSK_DRIVER_OK;
}

/*!
 * @brief Enable watch dog timer.
 */
int32_t
WDT_Refresh(void* res)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    // check API protocol
    if ((wdt->info->flags & WDT_FLAG_POWERED) == 0U) {

        // WDT not powered
        return CSK_DRIVER_ERROR;
    }

    // enable write
    wdt->reg->REG_WREN.all = WDT_MAGIC_WRITE_PROTECTION;
    // write restart register
    wdt->reg->REG_RESTART.all = WDT_MAGIC_RESTARTING;
    return CSK_DRIVER_OK;
}

/*!
 * @brief Enable watch dog timer.
 */
int32_t
WDT_Disable(void* res)
{
    CHECK_RESOURCES(res);
    WDT_RESOURCES* wdt = (WDT_RESOURCES*)res;

    // check API protocol
    if ((wdt->info->flags & WDT_FLAG_POWERED) == 0U) {
        // WDT not powered
        return CSK_DRIVER_ERROR;
    }

    // enable write and write control register
    wdt->reg->REG_WREN.all = WDT_MAGIC_WRITE_PROTECTION;
    wdt->reg->REG_CTRL.bit.EN = 0x0;

    return CSK_DRIVER_OK;
}

// TODO: maybe this should be design to global function,
// this irq handler can be used by other driver if the want to
// get interrupt from cp
static void irq_handler_cp_wdt(){
    cp_wdt_resources.cp_wdt_clbk((void*)cp_wdt_resources.cp_wdt_para);
}

int32_t HAL_CP_WDT_RegisterCallback(__cp_wdt_callback callback, void* para){
    if (callback == NULL){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    cp_wdt_resources.cp_wdt_clbk = callback;
    cp_wdt_resources.cp_wdt_para = para;

    HAL_CP_Interrupt_Init();
    HAL_CP_Interrupt_RegisterISR(IRQ_CP_WDT_VECTOR, irq_handler_cp_wdt, NULL);
    HAL_CP_Interrupt_Enable(IRQ_CP_WDT_VECTOR);

    return CSK_DRIVER_OK;
}

static void
WDT_IRQ_Handler(void){
    while(1);
}

static void
AON_WDT_IRQ_Handler(void){
    while(1);
}
