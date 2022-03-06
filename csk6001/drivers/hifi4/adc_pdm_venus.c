/*
 * adc_pdm_venus.c
 *
 *  Created on: Dec. 18, 2020
 *
 */

#include "hal/Driver_ADC_PDM.h"
#include "hal/adc_pdm_venus.h"
#include "hal/reg/cmn_sysctrl_reg_venus.h" // for sysctrl regiger (clock etc.)
#include "hal/ClockManager.h" // for CRM_GetSrcFreq()
#include "hal/reg/codec_reg_venus.h"
#include "hal/reg/aon_codec_reg_venus.h"
#include <assert.h>
#include <string.h>

#include <stdio.h>
// #include "hal/venus_log.h"

#define DEBUG_LOG   1 // 0
#if DEBUG_LOG
#define LOGD(format, ...)   printk(format, ##__VA_ARGS__)
#else
#define LOGD(format, ...)   ((void)0)
#endif // DEBUG_LOG

#define CLOGE printk
#define CLOGD printk
#define CLOGW printk
#define CLOGI printk

#define CSK_ADC_PDM_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,0)
#define SYSCTRL_CFG    ((SYSCFG_RegDef*) CMN_SYSCTRL_BASE)

// driver version
static const
CSK_DRIVER_VERSION adc_pdm_driver_version = { CSK_ADC_PDM_API_VERSION, CSK_ADC_PDM_DRV_VERSION };

static void adc_pdm_apc_event(uint32_t event_info, uint32_t usr_param);

//------------------------------------------------------------------------------------------
static SYSCFG_RegDef *g_sysctrl = SYSCTRL_CFG;
static CODEC_RegDef *g_cp_codec = (CODEC_RegDef *)CP_CODEC_BASE;
static AON_CODEC_RegDef *g_aon_codec = (AON_CODEC_RegDef *)AON_CODEC_BASE;

// ADC_PDM01
static ADC_PDM_INFO adc01_info = { 0 };
static ADC_PDM_GRP adc01_grp = {
        0, APC_DCH_ADC01,
        CSK_ADC_PDM_SAMPLE_BITS, 0,
        CSK_ADC01, // FIXME:
        &adc01_info
};

// Get Get ADC/PDM01 device group instance
void* ADC_PDM01() { return &adc01_grp; }


// ADC_PDM23
static ADC_PDM_INFO adc23_info = { 0 };
static ADC_PDM_GRP adc23_grp = {
        1, APC_DCH_ADC23,
        CSK_ADC_PDM_SAMPLE_BITS, 0,
        CSK_ADC23, // FIXME:
        &adc23_info
};

// Get Get ADC/PDM23 device group instance
void* ADC_PDM23() { return &adc23_grp; }


//------------------------------------------------------------------------------------------
static ADC_PDM_GRP * safe_adc_pdm_grp(void *adc_pdm_grp)
{
    // safe check of ADC/PDM device parameter
    if (adc_pdm_grp == &adc01_grp) {
        if (adc01_grp.reg != CSK_ADC01 || adc01_grp.apc_dch != APC_DCH_ADC01 || adc01_grp.dev_idx != 0) {
            CLOGW("ADC/PDM device context has been tampered illegally!!\n");
            return NULL;
        }
    } else if (adc_pdm_grp == &adc23_grp) {
        if (adc23_grp.reg != CSK_ADC23 || adc23_grp.apc_dch != APC_DCH_ADC23 || adc23_grp.dev_idx != 1) {
            CLOGW("ADC/PDM device context has been tampered illegally!!\n");
            return NULL;
        }
    } else {
        return NULL;
    }

    return (ADC_PDM_GRP *)adc_pdm_grp;
}

static inline void enable_adc_pdm(ADC_PDM_GRP *adc, uint8_t dev_bmp)
{
    uint32_t reg_val, reg_org, val;
    assert(adc != NULL);

    // R9: 0x4100C6 if DMIC, default 0x810146 if AMIC
    // R11: 0x0 if DMIC, 0x783 if AMIC

    if (adc->info->use_pdm) { // DMIC
        // R9: enable DMIC
        reg_val = adc->reg->REG_AUD_R9_ADC_CTRL4.all;
        if (!(reg_val & R9_DMIC_EN))
            adc->reg->REG_AUD_R9_ADC_CTRL4.all = reg_val | R9_DMIC_EN;

    } else { // AMIC
        // R9: disable DMIC
        reg_val = adc->reg->REG_AUD_R9_ADC_CTRL4.all;
        if (reg_val & R9_DMIC_EN)
            adc->reg->REG_AUD_R9_ADC_CTRL4.all = reg_val & ~R9_DMIC_EN;

        // R11: ADC-specific analog settings
        reg_org = reg_val = adc->reg->REG_AUD_R11_ADC_CTRL6.all;
        //if (adc->info->rc_ctrl != 0)
        //    reg_val |= R11_ADC_RCCTRL_12MHZ; //12MHz
        //else
        //    reg_val &= ~R11_ADC_RCCTRL_12MHZ; // 4MHz

        val = dev_bmp & ADC_PDM_BMP_STEREO;
        if (val == ADC_PDM_BMP_STEREO)
            reg_val |= R11_ADCLR_EN; // enable ADC L/R
        else if (val == ADC_PDM_BMP_LEFT)
            reg_val |= R11_ADCL_EN; // enable ADC Left
        else if (val == ADC_PDM_BMP_RIGHT)
            reg_val |= R11_ADCR_EN; // enable ADC Right
        else {
            CLOGW("%s: invalid dev_bmp (0x%x)\r\n", __func__, dev_bmp);
            return;
        }
        if (reg_val != reg_org)
            adc->reg->REG_AUD_R11_ADC_CTRL6.all = reg_val;
    }

    // R5: enable ADC internal clock
    reg_val = adc->reg->REG_AUD_R5_ADC_CTRL0.all;
    if (!(reg_val & BIT_VALUE_ADCCLK_EN)) {
        adc->reg->REG_AUD_R5_ADC_CTRL0.all = reg_val | BIT_VALUE_ADCCLK_EN;
    }
}

static inline void disable_adc_pdm(ADC_PDM_GRP *adc) // for both L&R channels
{
    uint32_t reg_val;
    assert(adc != NULL);

    if (adc->info->use_pdm) { // DMIC
        // R9: disable DMIC
        adc->reg->REG_AUD_R9_ADC_CTRL4.all &= ~R9_DMIC_EN;
    } else { // AMIC
        // R11: disable ADC L/R
        adc->reg->REG_AUD_R11_ADC_CTRL6.all = 0;
    }

    reg_val = adc->reg->REG_AUD_R5_ADC_CTRL0.all;
    if (reg_val & BIT_VALUE_ADCCLK_EN) {
        // R5: disable ADC internal clock
        adc->reg->REG_AUD_R5_ADC_CTRL0.all = reg_val & ~BIT_VALUE_ADCCLK_EN;
    }
}

static inline void enable_adc_pdm_clk(ADC_PDM_GRP *adc)
{
    assert(adc != NULL);
    //ADC01 in Always ON power domain, ADC23 in main domain
    switch (adc->dev_idx) {
    case 0: // AON CODEC
        AON_PMUCTRL->REG_AON_CRM_CTRL1.bit.ENA_VAD_CLK = 0x1;
    case 1:
        adc_clk_enable();
        return;
    default:
        CLOGW("%s: no ADC couple %d\n", __func__, adc->dev_idx);
        return;
    }
}

static inline void disable_adc_pdm_clk(ADC_PDM_GRP *adc)
{
    assert(adc != NULL);
    //ADC01 in Always ON power domain, ADC23 in main domain
    switch (adc->dev_idx) {
    case 0: // AON CODEC
        AON_PMUCTRL->REG_AON_CRM_CTRL1.bit.ENA_VAD_CLK = 0x0;
    case 1:
        adc_clk_disable();
        return;
    default:
        CLOGW("%s: no ADC couple %d\n", __func__, adc->dev_idx);
        return;
    }
}

static inline void set_adc_low_power(ADC_PDM_GRP *adc)
{
    assert(adc != NULL);
    uint32_t reg_val, reg_org;
    reg_org = reg_val = adc->reg->REG_AUD_R12_ADC_CTRL7.all;
    reg_val |= R12_PGA_BUF_LP | R12_RPGA_LP | R12_LPGA_LP;
    if (reg_val != reg_org)
        adc->reg->REG_AUD_R12_ADC_CTRL7.all = reg_val;
}

static inline void clear_adc_low_power(ADC_PDM_GRP *adc)
{
    assert(adc != NULL);
    uint32_t reg_val, reg_org;
    reg_org = reg_val = adc->reg->REG_AUD_R12_ADC_CTRL7.all;
    reg_val &= ~(R12_PGA_BUF_LP | R12_RPGA_LP | R12_LPGA_LP);
    if (reg_val != reg_org)
        adc->reg->REG_AUD_R12_ADC_CTRL7.all = reg_val;
}

//------------------------------------------------------------------------------------------

/**
 \fn          CSK_DRIVER_VERSION ADC_PDM_GetVersion (void)
 \brief       Get driver version.
 \return      \ref CSK_DRIVER_VERSION
*/
CSK_DRIVER_VERSION
ADC_PDM_GetVersion()
{
    return adc_pdm_driver_version;
}


/**
 \fn          int32_t ADC_PDM_Initialize(void *adc_pdm_grp, ...)
 \brief       Initialize ADC/PDM device group interface.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   cb_event  Pointer to \ref CSK_ADC_PDM_SignalEvent_t
 \param[in]   usr_param  User-defined value, acts as last parameter of cb_event
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \param[in]   use_flags  see below
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Initialize(void *adc_pdm_grp, CSK_ADC_PDM_SignalEvent_t cb_event, uint32_t usr_param,
                uint8_t dev_bmp, uint8_t use_flags)
{
    g_sysctrl = SYSCTRL_CFG;
    g_cp_codec = (CODEC_RegDef *)CP_CODEC_BASE;
    g_aon_codec = (AON_CODEC_RegDef *)AON_CODEC_BASE;

    int32_t ret;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);

    if (adc == NULL) {
        CLOGW("%s: invalid ADC/PDM device (0x%08x)!", __func__, adc_pdm_grp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }
    if ((dev_bmp & CH_BMP_STEREO) == 0) {
        CLOGW("%s: no valid channel (0x%x)!", __func__, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (adc->info->flags & ADC_PDM_FLAG_INITIALIZED)
        return CSK_DRIVER_OK;

    // initialize APC
    ret = apc_initialize();
    if (ret != CSK_DRIVER_OK) {
        LOGD("%s: failed to call apc_initialize()!", __func__);
        return ret;
    }

    // acquire APC channels
    ret = apc_dual_channel_acquire(adc->apc_dch, APC_INTF_ADC, adc->dev_idx, adc_pdm_apc_event, (uint32_t)adc);
    if (ret != CSK_DRIVER_OK) {
        LOGD("%s: failed to acquire APC channels!", __func__);
        return ret;
    }

    // initialize ADM/PDM run-time resources
    memset(adc->info, 0, sizeof(ADC_PDM_INFO));
    adc->info->cb_event = cb_event;
    adc->info->usr_param = usr_param;
    adc->info->use_pdm = (use_flags & ADC_PDM_USE_PDM) ? 1 : 0;
    adc->info->use_16bits = (use_flags & ADC_PDM_USE_16BITS) ? 1 : 0;
    adc->info->ch_bmp = dev_bmp;
    adc->info->status.all = 0U;

    //TODO: other initialization...

    adc->info->flags = ADC_PDM_FLAG_INITIALIZED; // ADM/PDM is initialized
    LOGD("%s: ADM/PDM version: API = 0x%x, DRV = 0x%x\n",
            __func__, adc_pdm_driver_version.api, adc_pdm_driver_version.drv);

    return CSK_DRIVER_OK;
}


/**
 \fn          int32_t ADC_PDM_Uninitialize(void *adc_pdm_grp)
 \brief       De-initialize ADC/PDM device group interface.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Uninitialize(void *adc_pdm_grp)
{
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    if (adc == NULL) {
        CLOGW("%s: invalid ADC/PDM device (0x%08x)!", __func__, adc_pdm_grp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((adc->info->flags & ADC_PDM_FLAG_INITIALIZED) == 0)
        return CSK_DRIVER_ERROR;

    // Abort current tranfers if any
    //ADM_PDM_Abort_Channels(adc_pdm_grp, adc->info->ch_bmp);

    //TODO: any other clean operations?

    // Power off ADC/PDM if Powered on
    if (adc->info->flags & ADC_PDM_FLAG_POWERED)
        ADC_PDM_PowerControl(adc_pdm_grp, CSK_POWER_OFF);

    // Release APC channels
    assert(adc->info->ch_bmp != 0);
    apc_dual_channel_release(adc->apc_dch);

    // Uninitialize APC
    apc_uninitialize();

    adc->info->flags = 0U; // ADC/PDM is uninitialized
    LOGD("%s is called for ADC/PDM couple %d!\n", __func__, adc->dev_idx);
    return CSK_DRIVER_OK;
}


/**
 \fn          int32_t ADC_PDM_PowerControl(void *adc_pdm_grp, ...)
 \brief       Control ADC/PDM interface's Power.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   state  Power state
 \return      \ref execution_status
*/
int32_t
ADC_PDM_PowerControl(void *adc_pdm_grp, CSK_POWER_STATE state)
{
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    if (adc == NULL) {
        CLOGW("%s: invalid ADC/PDM device (0x%08x)!", __func__, adc_pdm_grp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if ((adc->info->flags & ADC_PDM_FLAG_INITIALIZED) == 0)
        return CSK_DRIVER_ERROR;

    uint32_t val;
    switch (state) {
    case CSK_POWER_OFF:

        // abort current RX and clean state if configured
        if (adc->info->flags & ADC_PDM_FLAG_INITIALIZED)
            ADC_PDM_Abort(adc_pdm_grp, adc->info->ch_bmp);

        // disable ADC/PDM module
        disable_adc_pdm(adc);

        // disable ADC/PDM clock -- the clock is shared with other adc or i2s
        // FIXME: is it correct to disable the clock?
        //disable_adc_pdm_clk(adc);

        //TODO: other power-off configuration...

        adc->info->flags &= ~(ADC_PDM_FLAG_POWERED | ADC_PDM_FLAG_CONFIGURED);
        break;

    case CSK_POWER_LOW:
        if ((adc->info->flags & ADC_PDM_FLAG_POWERED) != 0U) {
            if ((adc->info->flags & ADC_PDM_FLAG_LOW_POWER) == 0) {
                set_adc_low_power(adc);
                adc->info->flags |= ADC_PDM_FLAG_LOW_POWER;
            }
            return CSK_DRIVER_OK;
        }

        // enable ADC/PDM clock here
        enable_adc_pdm_clk(adc);

        //TODO: other power-on configuration...

        // ADC/PDM is powered & in low power state
        adc->info->flags |= ADC_PDM_FLAG_POWERED | ADC_PDM_FLAG_LOW_POWER;
        break;
        //return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if ((adc->info->flags & ADC_PDM_FLAG_POWERED) != 0U) {
            if (adc->info->flags & ADC_PDM_FLAG_LOW_POWER) {
                clear_adc_low_power(adc);
                adc->info->flags &= ~ADC_PDM_FLAG_LOW_POWER;
            }
            return CSK_DRIVER_OK;
        }

        //NOTE: remove disable operation, or else disable_enable operation will cause audio data deformed...
        // disable ADC/PDM initially
        //disable_adc_pdm(adc);

        // enable ADC/PDM clock here
        enable_adc_pdm_clk(adc);

        //TODO: other power-on configuration...

        // ADC/PDM is powered
        adc->info->flags |= ADC_PDM_FLAG_POWERED;
        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}


static int32_t adc_enable_rx_channels(ADC_PDM_GRP *adc, uint8_t dev_bmp)
{
    uint8_t bmp;
    int32_t ret0 = CSK_DRIVER_OK;

    assert(adc != NULL);
    bmp = (dev_bmp & adc->info->ch_bmp);
    if (bmp != 0) { // IN channels
        ret0 = CSK_DRIVER_ERROR;
        if ((bmp & CH_BMP_STEREO) == CH_BMP_STEREO) {
            ret0 = apc_dual_channel_enable(adc->apc_dch);
        } else if (bmp & CH_BMP_LEFT) {
            ret0 = apc_channel_enable(APC_DCH_TO_CH(adc->apc_dch, 0));
        } else if (bmp & CH_BMP_RIGHT) {
            ret0 = apc_channel_enable(APC_DCH_TO_CH(adc->apc_dch, 1));
        } else {
            assert(0);
        }
    }

//    if (dev_bmp != 0) {
//        bmp = adc->info->ch_bmp;
//        bmp = dev_bmp & ~bmp;
//        if ( bmp != 0) {
//            LOGD("%s: There are invalid ADC/PDM couple %d channels (bmp=0x%x)!!",
//                    __func__, adc->dev_idx, bmp);
//        }
//    }

    return ret0;
}


static int32_t adc_disable_rx_channels(ADC_PDM_GRP *adc, uint8_t dev_bmp)
{
    uint8_t bmp;
    int32_t ret0 = CSK_DRIVER_OK;

    assert(adc != NULL);
    bmp = (dev_bmp & adc->info->ch_bmp);
    if (bmp != 0) { // IN channels
        ret0 = CSK_DRIVER_ERROR;
        if ((bmp & CH_BMP_STEREO) == CH_BMP_STEREO) {
            ret0 = apc_dual_channel_disable(adc->apc_dch);
        } else if (bmp & CH_BMP_LEFT) {
            ret0 = apc_channel_disable(APC_DCH_TO_CH(adc->apc_dch, 0));
        } else if (bmp & CH_BMP_RIGHT) {
            ret0 = apc_channel_disable(APC_DCH_TO_CH(adc->apc_dch, 1));
        } else {
            assert(0);
        }
    }

//    if (dev_bmp != 0) {
//        bmp = adc->info->ch_bmp;
//        bmp = dev_bmp & ~bmp;
//        if ( bmp != 0) {
//            LOGD("%s: There are invalid ADC/PDM couple %d channels (bmp=0x%x)!!",
//                    __func__, adc->dev_idx, bmp);
//        }
//    }

    return ret0;
}


/**
 \fn          int32_t ADC_PDM_Receive(void *adc_pdm_grp, ...)
 \brief       Receive data from ADC/PDM interface.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[out]  data  Pointer to buffer to receive data from ADC/PDM interface
 \param[in]   num   Number of data items to receive
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \param[in]   rx_flag   bit flags of receive operation
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Receive(void *adc_pdm_grp, uint32_t *data, uint32_t num,
                uint8_t dev_bmp, uint8_t rx_flag)
{
    int32_t ret;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        CLOGW("%s: ADC/PDM couple %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    if (adc->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    uint8_t start_now = rx_flag & ADC_PDM_RX_FLAG_START_NOW;
    if (!start_now) {
        ret = adc_disable_rx_channels(adc, bmp);
        if (ret != CSK_DRIVER_OK)
            return ret;
    }

    ret = CSK_DRIVER_ERROR;
    if ((bmp & CH_BMP_STEREO) == CH_BMP_STEREO)
        ret = apc_dual_channel_read(adc->apc_dch, data, num, 0, 0);
    else if ((bmp & CH_BMP_STEREO) == CH_BMP_LEFT)
        ret = apc_channel_read(APC_DCH_TO_CH(adc->apc_dch, 0), data, num, 0, 0);
    else if ((bmp & CH_BMP_STEREO) == CH_BMP_RIGHT)
        ret = apc_channel_read(APC_DCH_TO_CH(adc->apc_dch, 1), data, num, 0, 0);

    if (ret != CSK_DRIVER_OK)
        return ret;

    adc->info->status.bit.busy = 1;
    adc->info->status.bit.rx_full = 0;
    adc->info->status.bit.rx_ovf = 0;

    if (start_now) {
        enable_adc_pdm(adc, bmp); // enable ADC/PDM module
        ret = adc_enable_rx_channels(adc, bmp);
    }

    return ret;
}


/**
 \fn          int32_t ADC_PDM_Receive_LLP(void *adc_pdm_grp, ...)
 \brief       Receive data from ADC/PDM interface into several non-continuous buffers.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   bufs  Pointer to list of buffers to receive data from ADC/PDM interface
 \param[in]   buf_cnt  Number of data buffers in the list
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \param[in]   rx_flag   bit flags of receive operation
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Receive_LLP(void *adc_pdm_grp, AUDIO_BUFFER_USER *bufs, uint32_t buf_cnt, uint8_t dev_bmp, uint8_t rx_flag)
{
    int32_t ret;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        CLOGW("%s: ADC/PDM couple %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    if (adc->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    uint8_t start_now = rx_flag & ADC_PDM_RX_FLAG_START_NOW;
    if (!start_now) {
        ret = adc_disable_rx_channels(adc, bmp);
        if (ret != CSK_DRIVER_OK)
            return ret;
    }

    ret = CSK_DRIVER_ERROR;
    if ((bmp & CH_BMP_STEREO) == CH_BMP_STEREO)
        ret = apc_dual_channel_read_LLP(adc->apc_dch, (AUDIO_BUFFER_LLI *)bufs, buf_cnt, 0, 0);
    else if ((bmp & CH_BMP_STEREO) == CH_BMP_LEFT)
        ret = apc_channel_read_LLP(APC_DCH_TO_CH(adc->apc_dch, 0), (AUDIO_BUFFER_LLI *)bufs, buf_cnt, 0, 0);
    else if ((bmp & CH_BMP_STEREO) == CH_BMP_RIGHT)
        ret = apc_channel_read_LLP(APC_DCH_TO_CH(adc->apc_dch, 1), (AUDIO_BUFFER_LLI *)bufs, buf_cnt, 0, 0);

    if (ret != CSK_DRIVER_OK)
        return ret;

    adc->info->status.bit.busy = 1;
    adc->info->status.bit.rx_full = 0;
    adc->info->status.bit.rx_ovf = 0;

    if (start_now) {
        enable_adc_pdm(adc, bmp); // enable ADC/PDM module
       ret = adc_enable_rx_channels(adc, bmp);
     }

    return ret;
}


/**
 \fn          int32_t ADC_PDM_Enable(void *adc_pdm_grp, ...)
 \brief       Enable ADC/PDM interface and data receiving is started from now on
              if ADC_PDM_Receive_XXX is called before.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Enable(void *adc_pdm_grp, uint8_t dev_bmp)
{
    int32_t ret = CSK_DRIVER_OK;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    ret = adc_enable_rx_channels(adc, bmp);
    if (ret == CSK_DRIVER_OK)
        enable_adc_pdm(adc, bmp); // enable ADC/PDM module

    return ret;
}


/**
 \fn          int32_t ADC_PDM_Disable(void *adc_pdm_grp, ...)
 \brief       Disable ADC/PDM interface (and data receiving is suspended if any).
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Disable(void *adc_pdm_grp, uint8_t dev_bmp)
{
    int32_t ret = CSK_DRIVER_OK;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    ret = adc_disable_rx_channels(adc, bmp);
    return ret;
}


/**
 \fn          int32_t ADC_PDM_Abort(void *adc_pdm_grp, ...)
 \brief       Abort ADC/PDM data transfer if any.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   dev_bmp  ADC/PDM device bitmap (bit0 for ADC0 or ADC2, bit1 for ADC1 or ADC3)
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \return      \ref execution_status
*/
int32_t
ADC_PDM_Abort(void *adc_pdm_grp, uint8_t dev_bmp)
{
    int32_t ret = CSK_DRIVER_OK;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    ret = apc_dual_channel_abort(adc->apc_dch);
    if (ret != CSK_DRIVER_OK)
        return ret;

    adc->info->status.bit.busy = 0;
    return ret;
}

/**
 \fn          uint32_t ADC_PDM_GetRxCount(void *adc_pdm_grp, ...)
 \brief       Get received data count from ADC/PDM instance.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   dev_bmp  which ADC/PDM devices (similar to "I2S channels") are used,
              ADC0 or ADC2 (Left Channel, @bit[0]) and ADC1 or ADC3 (Right Channel, @bit[1]),
              and dev_bmp = 0x3 indicates both DAC0(or ADC2) & DAC1(or ADC3)
 \return      number of data items (24-bit samples)transferred if positive, error value if negative.
*/
int32_t
ADC_PDM_GetRxCount(void *adc_pdm_grp, uint8_t dev_bmp)
{
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device: 0x%08x, channel_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        CLOGW("%s: ADC/PDM couple %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    if ((bmp & CH_BMP_STEREO) == CH_BMP_STEREO) {
        if (adc->info->ch_mix)
            return apc_dual_channel_get_count(adc->apc_dch);

        int32_t ret = 0;
        if (adc->info->ch_bmp & CH_BMP_LEFT)
            ret += apc_channel_get_count(APC_DCH_TO_CH(adc->apc_dch, 0));
        if (adc->info->ch_bmp & CH_BMP_RIGHT)
            ret += apc_channel_get_count(APC_DCH_TO_CH(adc->apc_dch, 1));
        return ret;

    } else if ((bmp & CH_BMP_STEREO) == CH_BMP_LEFT)
        return apc_channel_get_count(APC_DCH_TO_CH(adc->apc_dch, 0));
    else if ((bmp & CH_BMP_STEREO) == CH_BMP_RIGHT)
        return apc_channel_get_count(APC_DCH_TO_CH(adc->apc_dch, 1));

    return CSK_DRIVER_ERROR;
}


static int32_t adc_get_samp_rate(ADC_PDM_GRP *adc)
{
    uint32_t i, reg_val;
    assert(adc != NULL);

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        CLOGW("%s: ADC/PDM couple %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    if (adc->info->samp_freq != 0)
        return adc->info->samp_freq;

    reg_val = adc->reg->REG_AUD_R5_ADC_CTRL0.bit.ADCSR_P;
    for (i = 0; i < ARRAY_COUNT(sc_ADC_SR_val_map); i++) {
        if (reg_val == sc_ADC_SR_val_map[i].reg_val)
            break;
    }
    if (i >= ARRAY_COUNT(sc_ADC_SR_val_map)) {
        CLOGW("%s: Sample rate's reg_val (%d) is NOT in built-in supported freq list!!\r\n",
                __func__, reg_val);
        return CSK_DRIVER_ERROR;
    }

    return sc_ADC_SR_val_map[reg_val].real_val;
}


static uint32_t adc_set_alc_params(ADC_PDM_GRP *adc, ALC_PARAMS *alc_p)
{
    union AUD_R8_ADC_CTRL3 reg_r8;
    union AUD_R9_ADC_CTRL4 reg_r9;
    uint8_t set_r9 = 0;
    assert(adc != NULL && alc_p != NULL);

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        CLOGW("%s: ADC/PDM couple %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    reg_r8.all = adc->reg->REG_AUD_R8_ADC_CTRL3.all;
    reg_r9.all = adc->reg->REG_AUD_R9_ADC_CTRL4.all;

    if (alc_p->alc_flags & ALC_FLAG_ALC_SEL_L)
        reg_r8.bit.ALCSEL_L = alc_p->alc_sel_l;
    if (alc_p->alc_flags & ALC_FLAG_ALC_SEL_R)
        reg_r8.bit.ALCSEL_R = alc_p->alc_sel_r;
    if (alc_p->alc_flags & ALC_FLAG_ERR_TOLERANCE) {
        if (alc_p->err_tolerance > R8_ERR_TOLERANCE_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.TOLERANCE = alc_p->err_tolerance;
    }
    if (alc_p->alc_flags & ALC_FLAG_TARGET_L) {
        if (alc_p->target_l > R8_TARGET_LEVEL_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.TARGET_L = alc_p->target_l;
    }
    if (alc_p->alc_flags & ALC_FLAG_TARGET_R) {
        if (alc_p->target_r > R8_TARGET_LEVEL_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.TARGET_R = alc_p->target_r;
    }
    if (alc_p->alc_flags & ALC_FLAG_ALC_MODE)
        reg_r8.bit.ALCMODE = alc_p->alc_mode;
    if (alc_p->alc_flags & ALC_FLAG_NGATE_EN)
        reg_r8.bit.NG_EN = alc_p->ngate_en;
    if (alc_p->alc_flags & ALC_FLAG_NGATE_FLOOR) {
        if (alc_p->ngate_floor > R8_NGATE_FLOOR_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.NG = alc_p->ngate_floor;
    }

    if (alc_p->alc_flags & ALC_FLAG_ALC_MIN) {
        if (alc_p->alc_min > R8_ALCMIN_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.ALCMIN = alc_p->alc_min;
    }
    if (alc_p->alc_flags & ALC_FLAG_ALC_MAX) {
        if (alc_p->alc_max > R8_ALCMAX_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r8.bit.ALCMAX = alc_p->alc_max;
    }
    if (alc_p->alc_flags & (ALC_FLAG_ALC_MIN | ALC_FLAG_ALC_MAX)) {
        if (reg_r8.bit.ALCMIN > reg_r8.bit.ALCMAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
    }

    if (alc_p->alc_flags & ALC_FLAG_ALC_HOLD) {
        if (alc_p->alc_hold > R9_ALC_HOLD_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r9.bit.ALCHLD = alc_p->alc_hold;
        set_r9 = 1;
    }
    if (alc_p->alc_flags & ALC_FLAG_ALC_ATTACK) {
        if (alc_p->alc_attack > R9_ALC_ATTACK_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r9.bit.ALCATK = alc_p->alc_attack;
        set_r9 = 1;
    }
    if (alc_p->alc_flags & ALC_FLAG_ALC_DECAY) {
        if (alc_p->alc_decay > R9_ALC_DECAY_MAX)
            return CSK_ADCPDM_ERROR_ALC_PARAMS;
        reg_r9.bit.ALCDCY = alc_p->alc_decay;
        set_r9 = 1;
    }

    adc->reg->REG_AUD_R8_ADC_CTRL3.all = reg_r8.all;
    if (set_r9)
        adc->reg->REG_AUD_R9_ADC_CTRL4.all = reg_r9.all;

    return CSK_DRIVER_OK;
}


static bool check_sr_osr_pair(ADC_PDM_GRP *adc)
{
    uint32_t i, count;
    assert(adc != NULL);
    if (adc->info->use_pdm) { // DMIC
        count = ARRAY_COUNT(sc_DMIC_SR_OSR);
        for (i=0; i<count; i++) {
            if (sc_DMIC_SR_OSR[i].major == adc->info->samp_freq &&
                sc_DMIC_SR_OSR[i].minor == adc->info->over_samp_ratio) {
                //adc->info->rc_ctrl = sc_DMIC_SR_OSR[i].least & 0x1;
                return true;
            }
        } // end for

    } else { // AMIC
        count = ARRAY_COUNT(sc_AMIC_SR_OSR);
        for (i=0; i<count; i++) {
            if (sc_AMIC_SR_OSR[i].major == adc->info->samp_freq &&
                sc_AMIC_SR_OSR[i].minor == adc->info->over_samp_ratio) {
                adc->info->rc_ctrl = sc_AMIC_SR_OSR[i].least & 0x1;

                // R11: ADC-specific analog settings
                uint32_t reg_val, org_val = 0;
                org_val = reg_val = adc->reg->REG_AUD_R11_ADC_CTRL6.all;
                if (adc->info->rc_ctrl != 0)
                    reg_val |= R11_ADC_RCCTRL_12MHZ; //12MHz
                else
                    reg_val &= ~R11_ADC_RCCTRL_12MHZ; // 4MHz
                if (reg_val != org_val)
                    adc->reg->REG_AUD_R11_ADC_CTRL6.all = reg_val;

                return true;
            }
        } // end for

    }
    return false;
}


/**
 \fn          int32_t ADC_PDM_Control(void *adc_pdm_grp, ...)
 \brief       Control ADC/PDM interface.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   control  Operation
 \param[in]   arg  Argument of operation (optional), i.e. sample rate
 \return      common \ref execution_status and driver specific \ref ADC_PDM execution_status
*/
int32_t
ADC_PDM_Control(void *adc_pdm_grp, uint32_t control, uint32_t arg)
{
    int32_t ret;
    uint32_t val, mclk_flag, reg_val;
    union AUD_R5_ADC_CTRL0 reg_r5;
    union AUD_R6_ADC_CTRL1 reg_r6;
    union AUD_R9_ADC_CTRL4 reg_r9;
    union AUD_R12_ADC_CTRL7 reg_r12;

    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    if (adc == NULL) {
        CLOGW("%s: invalid ADC/PDM device (0x%08x)!", __func__, adc_pdm_grp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_POWERED))
        return CSK_DRIVER_ERROR;

    // exclusive MISC OP
    switch (control & CSK_ADCPDM_EXCL_OP_Msk) {
        // NO MISC OP
        case CSK_ADCPDM_EXCL_OP_UNSET:
            break;

        // abort ADC/PDM transfer
        case CSK_ADCPDM_ABORT_TRANSFER:
            ADC_PDM_Abort(adc, (arg & 0x3));
            return CSK_DRIVER_OK;

        // get sample rate
        case CSK_ADCPDM_GET_SAMP_RATE:
            return adc_get_samp_rate(adc);

        // set ALC parameters
        case CSK_ADCPDM_SET_ALC_PARAMS:
            if (adc->info->status.bit.busy)
                return CSK_DRIVER_ERROR_BUSY;
            return adc_set_alc_params(adc, (ALC_PARAMS*)arg);

        default:
            return CSK_DRIVER_ERROR_UNSUPPORTED;
    } // end exclusive MISC_OP

    if (adc->info->status.bit.busy)
        return CSK_DRIVER_ERROR_BUSY;

    reg_r5.all = adc->reg->REG_AUD_R5_ADC_CTRL0.all;
    reg_r6.all = adc->reg->REG_AUD_R6_ADC_CTRL1.all;

    // 24MHz main clock by default
    reg_r5.bit.AUD_ADC_MCLK_SEL = 0;
    // How to set main clock? mclk_flag options:
    // 0 = use 24MHz, 1 = use 22.05MHz, 2 = not changed
    // NOTE: AON_CODEC(ADC01) supports 24MHz main clock only!!
    mclk_flag = 0;

    // Sample Rate
    switch (control & CSK_ADCPDM_SR_Msk) {
    // Keep Sample Rate unchanged
    case CSK_ADCPDM_SR_UNSET:
        mclk_flag = 2; // not changed
        break;

    case CSK_ADCPDM_SR_8KHZ:
        reg_r5.bit.ADCSR_P = 0x0;
        adc->info->samp_freq = 8000;
        break;

    case CSK_ADCPDM_SR_16KHZ:
        reg_r5.bit.ADCSR_P = 0x3;
        adc->info->samp_freq = 16000;
        break;

    case CSK_ADCPDM_SR_44P1KHZ:
        if (adc->dev_idx == 0) // ADC01 support 24MHz only
            return CSK_ADCPDM_ERROR_SAMP_RATE;
        mclk_flag = 1; // use 22.05MHz
        reg_r5.bit.AUD_ADC_MCLK_SEL = 1; // 22.05MHz main clock
        reg_r5.bit.ADCSR_P = 0x7;
        adc->info->samp_freq = 44100;
        break;

    case CSK_ADCPDM_SR_48KHZ:
        reg_r5.bit.ADCSR_P = 0x8;
        adc->info->samp_freq = 48000;
        break;

    default:
        return CSK_ADCPDM_ERROR_SAMP_RATE;
    }

    if (mclk_flag == 0) { // use 24Mhz
        adc_clk_select(AUDIO_CLK_SRC_XTAL);
    } else if (mclk_flag == 1) { // use 22.05Mhz
        cmn_divider_t div;
        div.divider_N = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_N;
        div.divider_M = BOARD_BOOTCLOCKRUN_AUDPLL_DIV_M;
        AUDIOPLL_InitPLL(div, BOARD_BOOTCLOCKRUN_AUDPLL_DIV_NF);
        init_audpll_audio_clk(0); // DEF_AUDPLL_AUDIO_FREQ (22050000)
        adc_clk_select(AUDIO_CLK_SRC_AUDPLL);
    }

    // Over Sample Ratio
    switch (control & CSK_ADCPDM_OSR_Msk) {
    // Keep Over Sample Ratio unchanged
    case CSK_ADCPDM_OSR_UNSET:
        break;

    case CSK_ADCPDM_OSR_500:
        reg_r5.bit.ADCOSR_P = 0x0;
        adc->info->over_samp_ratio = 500;
        break;

    case CSK_ADCPDM_OSR_250:
        reg_r5.bit.ADCOSR_P = 0x1;
        adc->info->over_samp_ratio = 250;
        break;

    case CSK_ADCPDM_OSR_125:
        reg_r5.bit.ADCOSR_P = 0x2;
        adc->info->over_samp_ratio = 125;
        break;

    case CSK_ADCPDM_OSR_100:
        reg_r5.bit.ADCOSR_P = 0x3;
        adc->info->over_samp_ratio = 100;
        break;

    case CSK_ADCPDM_OSR_50:
        reg_r5.bit.ADCOSR_P = 0x4;
        adc->info->over_samp_ratio = 50;
        break;

    default:
        return CSK_ADCPDM_ERROR_SAMP_RATE;
    }

    // check if the combination of Sample Rate & Over Sample Ratio are supported
    if ((control & CSK_ADCPDM_SR_Msk) != CSK_ADCPDM_SR_UNSET ||
        (control & CSK_ADCPDM_OSR_Msk) != CSK_ADCPDM_OSR_UNSET) {
        if (!check_sr_osr_pair(adc)) {
            CLOGW("%s: unsupported pair of Sample Rate(%d) and Over Sample Ratio(%d)!\n",
                    __func__, adc->info->samp_freq, adc->info->over_samp_ratio);
            return CSK_ADCPDM_ERROR_SR_OSR_PAIR;
        }
    }

    // High Pass Filter Setting
    switch (control & CSK_ADCPDM_HPF_Msk) {
    // HPF1 & HPF2 settings are kept unchanged or default
    case CSK_ADCPDM_HPF_UNSET:
    default:
        break;

    case CSK_ADCPDM_HPF_SET:
        reg_r6.bit.HPF1EN = arg & 0x1; // bit[0]
        reg_r6.bit.HPF2EN = (arg >> 1) & 0x1; // bit[1]
        reg_r6.bit.HPFCUT = (arg >> 2) & 0x7; // bit[4:2]
        break;
    }

    // IN Channel Data Mix
    switch (control & CSK_ADCPDM_RXCFG_Msk) {
    // Keep channel data mix unchanged
    case CSK_ADCPDM_RXCFG_UNSET:
        break;

    case CSK_ADCPDM_RXCFG_SEPA:
        adc->info->ch_mix = 0;
         break;

    case CSK_ADCPDM_RXCFG_MIXED:
        adc->info->ch_mix = 1;
        break;

    default:
        return CSK_ADCPDM_ERROR_RXCFG;
    }


    // call apc_dual_channel_setup to notify APC channels
    // FIXME: should it support APC_CHMODE_24BITS_LOW?
    ret = apc_dual_channel_setup(adc->apc_dch,
                                APC_CHMODE_24BITS_HIGH,
                                adc->info->ch_bmp,
                                adc->info->ch_mix,
                                adc->info->use_16bits ? 1 : 0);
    if (ret != CSK_DRIVER_OK)
        return ret;

    //------------------------------------------------
    // write settings into ADC/PDM registers
    //------------------------------------------------

    val = adc->info->flags & ADC_PDM_FLAG_CONFIGURED;

    // enable 2.5V LDO
    ENABLE_LDO25();

    // CODEC analog settings
    CONFIG_CODEC_ANALOG();

    if (val == 0) { // not configured, Control first called
        reg_r5.bit.LPGA_TOEN_P = 1; // Zero Crossing Time Out enable for ADC Left PGA gain
        reg_r5.bit.RPGA_TOEN_P = 1; // Zero Crossing Time Out enable for ADC Right PGA gain

        //R6: VOL/Gain, HPF Setting etc.
        //adc->reg->REG_AUD_R6_ADC_CTRL1.all =  0xCC6EFDF;
        reg_r6.all |= R6_ADCR_VOL(0x5F) | R6_ADCR_VOL(0x5F) |   // L/R Digital Gain = +10dB
                    R6_ADCR_PGA_VOL(6) | R6_ADCL_PGA_VOL(6);    // L/R PGA Vol = +6dB
                    // R6_HPF2_EN | R6_HPF1_EN | R6_HPF_CUT(3);
        adc->info->d_vol_left = adc->info->d_vol_right = 0x5F;

        // R9: 0x4100C6 if DMIC, default 0x810146 if AMIC
        // R12: 0x8C5 if DMIC, 0x208C5 if AMIC
        reg_r9.all = adc->reg->REG_AUD_R9_ADC_CTRL4.all;
        reg_r12.all = R12_PGA_BUF_EN | R12_RPGA_EN | R12_RPGA_ZCEN | R12_LPGA_EN | R12_LPGA_ZCEN |
                      R12_ADC_VREF_40UA | R12_ADC_VCM_BIAS_30UA;
        if (adc->info->use_pdm) { // DMIC
            // R9: 0x4100C6 if DMIC (except DMIC_EN not set)
            reg_r9.bit.AUTORST_TYPE = 0; //FIXME: 000b = 128us
            reg_r9.bit.DMIC_SRC = 0; // FIXME: 0 = DMIC0
            reg_r9.bit.DMIC_MODE = 1; // FIXME: 1= double edge on DMIC0 or DMIC1
            reg_r9.bit.ALCHLD = 0; // FIXME: why use 0?
            reg_r9.bit.ALCATK = 0xc; // FIXME: why use 0xc?
            reg_r9.bit.ALCDCY = 6; // FIXME: why use 6?
            // R12: 0x8C5 if DMIC
            reg_r12.bit.AUD_ADC_VREF_EN = 0; // VREF disabled

            // R11: remove ADC-specific settings?
            reg_val = adc->reg->REG_AUD_R11_ADC_CTRL6.all;
            if (reg_val != 0)
                adc->reg->REG_AUD_R11_ADC_CTRL6.all = 0;

        } else { // AMIC
            // R9: default 0x810146 if AMIC (DMIC_EN = 0)
            reg_r9.bit.AUTORST_TYPE = 1; // FIXME: 001 = 256us
            reg_r9.bit.ALCHLD = 1; // FIXME: why use 1?
            reg_r9.bit.ALCATK = 4; // FIXME: why use 4?
            reg_r9.bit.ALCDCY = 6; // FIXME: why use 6?
            // R12: 0x208C5 if AMIC
            reg_r12.bit.AUD_ADC_VREF_EN = 1; // VREF enabled
            reg_r12.bit.AUD_ADC_IDAC_OFFSET = 1; // feed back idac DC offset
            reg_r12.bit.RVAL_AUD_IDAC_BIAS = 1; // register value for aud_idac_bias
            reg_r12.bit.RSET_AUD_IDAC_BIAS = 1; // register control enable

            // R1: default 0x0000, set to 0x8000 (ADC analog clock invert)
            //adc->reg->REG_AUD_R1_GLOBAL0.bit.ADC_ANACLK_INV = 1;

            //FIXME: R0: 0xf001 on AON_CODEC, 0xf002 on CP_CODEC?
            if (adc->dev_idx == 0) // AON_CODEC
                adc->reg->REG_AUD_R0_RSVD_REG0.bit.ADC01_DESKEW_CLK_INV = 1;
            if (adc->dev_idx == 1) // CP_CODEC
                adc->reg->REG_AUD_R0_RSVD_REG0.bit.ADC23_DESKEW_CLK_INV = 1;

            // R11: ADC-specific analog settings
            // bit[22]=1, rc calibration setting from register
            // bit[21:16]=0, quar calibration value from register.
            // bit[10]=1, 12MHz FS; bit[9]=1, bias current 10uA;
            // bit[8:7] = llb, Feedback IDAC Control +13%
            // bit[6:3] = 1000b, ADC amp ibias control
            //      0000: 1.5uA, 0001: 2uA, 0010: 2.5uA, 0011: 3uA
            //reg_val = 0x4003C0; // 0x4003C0; // 0x380; // 0x780;
            reg_val = R11_ADC_RSET_RC_CALI | R11_ADC_QUAR_COV(0) | R11_ADC_CBIAS_CUR_10UA |
                      R11_ADC_IDAC_7P5UA_13PERP | R11_ADC_IB_CTRL(8);
            if (adc->info->rc_ctrl != 0)
                reg_val |= R11_ADC_RCCTRL_12MHZ;
            if (adc->reg->REG_AUD_R11_ADC_CTRL6.all != reg_val)
                adc->reg->REG_AUD_R11_ADC_CTRL6.all = reg_val;
        }
        adc->reg->REG_AUD_R9_ADC_CTRL4.all = reg_r9.all;
        adc->reg->REG_AUD_R12_ADC_CTRL7.all = reg_r12.all;
    }

    // R5
    reg_r5.bit.ADC_RESETN_P = 1; // release (NOT RESET)
    adc->reg->REG_AUD_R5_ADC_CTRL0.all = reg_r5.all;
    // R6
    adc->reg->REG_AUD_R6_ADC_CTRL1.all = reg_r6.all;

    // set configured flag if sampling rate is set...
    if ((control & CSK_ADCPDM_SR_Msk) != 0)
        adc->info->flags |= ADC_PDM_FLAG_CONFIGURED;

    return CSK_DRIVER_OK;
}


/**
 \fn          int32_t ADC_PDM_SetVolume(void *adc_pdm_grp, ...)
 \brief       Set analog and/or digital of ADC/PDM device group.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   a_gain    Analog Gain of Left Channel (ADC0/2, @ a_gain[15:0])
                        and Right Channel (ADC1/3, @ a_gain[31:16]), for ADC only (NOT for PDM/DMIC)
                        Analog Gain value: 0x0 (-12dB) ~ 0x18 (+36dB), 2dB each step, default 0x6 (0dB)
 \param[in]   d_gain    Digital gain of Left Channel (ADC0/2, @ d_gain[15:0])
                        and Right Channel (ADC1/3, @ d_gain[31:16]),
                        Digital Gain value: 0x0 (-83dB) ~ 0x7F (+42dB), 1dB each step, default 0x55 (0dB)
 \param[in]   vol_flag  volume flag which indicates which volume items are specified
 \return      common \ref execution_status and driver specific \ref ADC/PDM execution_status
*/
int32_t
ADC_PDM_SetVolume(void *adc_pdm_grp, uint32_t a_gain, uint32_t d_gain, uint32_t vol_flag)
{
    uint32_t val;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    if (adc == NULL) {
        CLOGW("%s: invalid ADC/PDM device group (0x%08x)!", __func__, adc_pdm_grp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(vol_flag & ADC_PDM_BMP_STEREO) && !(vol_flag & (ADC_PDM_BMP_STEREO<<2))) {
        CLOGW("%s: invalid parameter, no volume is set: vol_flag=0x%08x",
                __func__, vol_flag);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        LOGD("%s: ADC/PDM device group %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    union AUD_R6_ADC_CTRL1 reg_r6;
    reg_r6.all = adc->reg->REG_AUD_R6_ADC_CTRL1.all;

    // Analog gain
    if (vol_flag & (ADC_PDM_VOL_FLAG_A_LEFT | ADC_PDM_VOL_FLAG_A_RIGHT)) {
        if (vol_flag & ADC_PDM_VOL_FLAG_A_LEFT) { // Left Analog
            val = a_gain & 0xFFFF;
            if (val > R6_ADC_PGA_VOL_MAX || val < R6_ADC_PGA_VOL_MIN)
                return CSK_DRIVER_ERROR_PARAMETER;
            reg_r6.bit.ADC_PGA_LEVEL_L = val;
        }

        if (vol_flag & ADC_PDM_VOL_FLAG_A_RIGHT) { // Right Analog
            val = (a_gain >> 16) & 0xFFFF;
            if (val > R6_ADC_PGA_VOL_MAX || val < R6_ADC_PGA_VOL_MIN)
                return CSK_DRIVER_ERROR_PARAMETER;
            reg_r6.bit.ADC_PGA_LEVEL_R = val;
        }
    }

    // Digital gain
    if (vol_flag & (ADC_PDM_VOL_FLAG_D_LEFT | ADC_PDM_VOL_FLAG_D_RIGHT)) {
        if (vol_flag & ADC_PDM_VOL_FLAG_D_LEFT) { // Left Digital
            val = d_gain & 0xFFFF;
            if (val > R6_ADC_VOL_MAX || val < R6_ADC_VOL_MIN)
                return CSK_DRIVER_ERROR_PARAMETER;
            reg_r6.bit.ADCVOL_L = val;
            adc->info->d_vol_left = val;
        }

        if (vol_flag & ADC_PDM_VOL_FLAG_D_RIGHT) { // Right Digital
            val = (d_gain >> 16) & 0xFFFF;
            if (val > R6_ADC_VOL_MAX || val < R6_ADC_VOL_MIN)
                return CSK_DRIVER_ERROR_PARAMETER;
            reg_r6.bit.ADCVOL_R = val;
            adc->info->d_vol_right = val;
        }
    }

    adc->reg->REG_AUD_R6_ADC_CTRL1.all = reg_r6.all;
    return CSK_DRIVER_OK;
}


/**
 \fn          int32_t ADC_PDM_SetMute(void *adc_pdm_grp, ...)
 \brief       Set mute/unmute value of ADC/PDM interface.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[in]   mute_val  Mute/UnMute bit of Left Channel (ADC0/2, @ mute_val[0])
                        and Right Channel (ADC1/3, @ mute_val[1]),
 \param[in]   dev_bmp  specify which ADC/PDM devices in the device group
                      ADC0/2 (Left Channel, @bit[0]) and ADC1/3 (Right Channel, @bit[1]),
                      and dev_bmp = 0x3 indicates both ADC0/2 & ADC1/3
 \return      common \ref execution_status and driver specific \ref ADC/PDM execution_status
*/
int32_t
ADC_PDM_SetMute(void *adc_pdm_grp, uint8_t mute_val, uint8_t dev_bmp)
{
    uint8_t mute;
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);
    uint8_t bmp = 0;

    if (adc == NULL || (bmp = adc->info->ch_bmp & dev_bmp) == 0) {
        CLOGW("%s: invalid parameter, ADC/PDM device group: 0x%08x, dev_bmp: %d",
                __func__, adc_pdm_grp, dev_bmp);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (!(adc->info->flags & ADC_PDM_FLAG_CONFIGURED)) {
        LOGD("%s: ADC/PDM device group %d has NOT been configured!!",
                __func__, adc->dev_idx);
        return CSK_DRIVER_ERROR;
    }

    union AUD_R6_ADC_CTRL1 reg_r6;
    union AUD_R12_ADC_CTRL7 reg_r12;

    reg_r6.all = adc->reg->REG_AUD_R6_ADC_CTRL1.all;
    reg_r12.all = adc->reg->REG_AUD_R12_ADC_CTRL7.all;

    // Left channel mute setting
    if (dev_bmp & ADC_PDM_BMP_LEFT) {
        mute = mute_val & ADC_PDM_BMP_LEFT;
        if (mute) {
            reg_r6.bit.ADCVOL_L = 0; // digital mute
            reg_r12.bit.AUD_LPGA_MUTE = 1; // analog PGA mute
        } else {
            reg_r6.bit.ADCVOL_L = adc->info->d_vol_left; // restore original value
            reg_r12.bit.AUD_LPGA_MUTE = 0; // analog PGA unmute
        }
        adc->info->status.bit.l_mute = mute;
    }

    // Right channel mute setting
    if (dev_bmp & ADC_PDM_BMP_RIGHT) {
        mute = (mute_val & ADC_PDM_BMP_RIGHT) >> 1;
        if (mute) {
            reg_r6.bit.ADCVOL_R = 0; // digital mute
            reg_r12.bit.AUD_RPGA_MUTE = 1; // analog PGA mute
        } else {
            reg_r6.bit.ADCVOL_R = adc->info->d_vol_right; // restore original value
            reg_r12.bit.AUD_RPGA_MUTE = 0; // analog PGA unmute
        }
        adc->info->status.bit.r_mute = mute;
    }

    adc->reg->REG_AUD_R6_ADC_CTRL1.all = reg_r6.all;
    adc->reg->REG_AUD_R12_ADC_CTRL7.all = reg_r12.all;
    return CSK_DRIVER_OK;
}


/**
 \fn          int32_t ADC_PDM_GetStatus(void *adc_pdm_grp, ...)
 \brief       Get ADC/PDM status.
 \param[in]   adc_pdm_grp  Pointer to ADC/PDM device group instance
 \param[out]  status  Pointer to CSK_ADC_PDM_STATUS buffer
 \return      \ref execution_status
 */
int32_t
ADC_PDM_GetStatus(void *adc_pdm_grp, CSK_ADCPDM_STATUS *status)
{
    ADC_PDM_GRP *adc = safe_adc_pdm_grp(adc_pdm_grp);

    if (adc == NULL || status == NULL)
        return CSK_DRIVER_ERROR_PARAMETER;

    (*status).all = adc->info->status.all;
    return CSK_DRIVER_OK;
}


static void
adc_pdm_apc_event(uint32_t event_info, uint32_t usr_param)
{
    uint8_t notify = 1;
    uint32_t adc_event_info;

    uint8_t event_type = event_info & 0xFF;
    ADC_PDM_GRP *adc = (ADC_PDM_GRP *)usr_param;

    APC_CH apc_ch = (event_info >> 8) & 0xFF;
    adc_event_info = APC_CH_LR_IDX(apc_ch) << 8;

    assert(adc != NULL);
    switch (event_type) {
    case APC_EVENT_TRANSFER_COMPLETE:
        adc->info->status.bit.busy = 0;
        adc_event_info |= CSK_ADCPDM_EVENT_RECEIVE_COMPLETE;
        break;

    case APC_EVENT_RX_FIFO_OVERRUN:
        //adc->info->status.bit.busy = 0;
        adc->info->status.bit.rx_ovf = 1;
        adc_event_info |= CSK_ADCPDM_EVENT_RX_FIFO_OVERRUN;
        break;

    case APC_EVENT_RX_FIFO_FULL:
        adc->info->status.bit.rx_full = 1;
        adc_event_info |= CSK_ADCPDM_EVENT_RX_FIFO_FULL;
        break;

    case APC_EVENT_DMA_ERROR:
        //adc->info->status.bit.busy = 0;
        adc_event_info |= CSK_ADCPDM_EVENT_OTHER_ERROR;
        break;

    default:
        LOGD("%s: unknown APC event (0x%x)!", __func__, event_type);
        notify = 0;
        break;
    }

    // notify ADC/PDM caller
    if (notify && adc->info->cb_event != NULL) {
        adc->info->cb_event(adc_event_info, adc->info->usr_param);
    }
}
