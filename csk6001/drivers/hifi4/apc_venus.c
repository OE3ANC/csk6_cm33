/*
 * apc_venus.c
 *
 *  Created on:  Sept. 22, 2020
 *
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hal/Driver_Common.h"
#include "cache.h"

#include "hal/reg/cmn_sysctrl_reg_venus.h"
#include "hal/apc_inner_venus.h"
// #include "venus_log.h"

#define DEBUG_LOG   1 // 0
#if DEBUG_LOG
#define LOGD(format, ...)   printf(format, ##__VA_ARGS__)
// #define LOGD(format, ...)   CLOGD(format, ##__VA_ARGS__)
#else
#define LOGD(format, ...)   ((void)0)
#endif // DEBUG_LOG

//=============================================================================
// APC channel information (fixed by IC design)
typedef struct _APC_CH_FIXED
{
    uint16_t ch_idx     : 7; // APC channel index: IN 0~3, IN/Echo 4~5, OUT 6~7
    uint16_t ch_dir     : 1; // APC channel direction: 0=OUT, 1=IN
    uint16_t dma_hsid   : 8; // DMA request/handshaking ID of APC channel

    uint16_t fifo_depth; // FIFO depth (in WORD) of APC channel
    uint32_t data_addr; // address of data register to access FIFO

    //TODO: Add more fields for APC channel
} APC_CH_FIXED;

//TODO/FIXME: APC channel transfer information
//typedef struct _APC_CH_XFER {
//    uint8_t dma_ch; // dynamically allocated DMA channel for APC channel read/write
//
//    uint8_t busy : 1; // DMA transfer is ongoing (busy) or not?
//    uint8_t llp : 1; // use a buffer or LLP?
//    uint8_t reserved : 6;
//
//    uint32_t samps_xfered;
//
//    union {
//        uint32_t *sample_data;
//        AUDIO_BUFFER_LLI *bufs;
//    } u_buf;
//
//    union {
//        uint32_t sample_cnt;
//        uint32_t buf_cnt;
//    } u_cnt;
//
//} APC_CH_XFER;

//TODO/FIXME: APC channel attribute (8bits)
//struct DCH_ATTR_8BITS {
//    // L/R channel select - left, right, or both?
//    uint8_t ch_sel : 2; // 01b = left, 10b = right, 11b = both
//    // stereo(1) indicates that L/R channel data are interlaced, as LRLRLR...
//    // mix_mode should be stereo(1) in I2S TDM mode according to IP design
//    uint8_t mix_mode : 1; // 0 = mono (separate), 1 = stereo (mixed)
//    // channel mode indicates sample bit width and decides data sequence when accessed
//    // i.e. 1. mix_mode is mono (L/R channel data are accessed respectively)
//    //  0: 16-bit {L1, L0}, 1: 24-bit{L0, 8'd0}, 2: 32-bit {L0}, 3: 24-bit{8'd0, L0}
//    // 2. mix_mode is stereo (L/R channel data are accessed mixed)
//    //  data sequence is always as L,R,L,R,L,R...
//    uint8_t ch_mode : 2; // 0 = 16-bit, 1 = 24-bit MSB, 2 = 32-bit, 3 = 24-bit LSB
//    uint8_t reserved : 2;
//    uint8_t setup_done : 1; // channel setup has been done?
//};
//
//typedef union {
//    uint8_t                     all;
//    struct DCH_ATTR_8BITS       bit;
//} DCH_ATTR;


// APC dual_channel information (Run-time)
typedef struct _APC_DCH_INFO
{
    CSK_APC_SignalEvent_t cb_event;    // event callback
    uint32_t usr_param; // user parameter of event callback

    uint8_t intf_type; // see definitions of APC_INTF_TYPE
    uint8_t intf_idx; // instance index of intf_type, i.e. I2S 0, 1, 2...

    // L/R channel select - left, right, or both?
    uint8_t ch_sel : 2; // 01b = left, 10b = right, 11b = both
    // stereo(1) indicates that L/R channel data are interlaced, as LRLRLR...
    // mix_mode should be stereo(1) in I2S TDM mode according to IP design
    uint8_t mix_mode : 1; // 0 = mono (separate), 1 = stereo (mixed)
    uint8_t mix_target : 1; // if mixed, data path is via left (0) or right (1) channel
    // channel mode indicates sample bit width and decides data sequence when accessed
    // i.e. 1. mix_mode is mono (L/R channel data are accessed respectively)
    //  0: 16-bit {L1, L0}, 1: 24-bit{L0, 8'd0}, 2: 32-bit {L0}, 3: 24-bit{8'd0, L0}
    // 2. mix_mode is stereo (L/R channel data are accessed mixed)
    //  data sequence is always as L,R,L,R,L,R...
    uint8_t ch_mode : 2; // 0 = 16-bit, 1 = 24-bit MSB, 2 = 32-bit, 3 = 24-bit LSB
    uint8_t reserved : 1;
    uint8_t setup_done : 1; // channel setup has been done?

    // option values: 16, 24, 32, it decides channel mode setting!
    uint8_t samp_bits; // length of each sample data in bits
    uint8_t dma_width_bits; // bits of dma width, 0: byte, 1: halfword, 2: word
    uint8_t dma_bsize_bits; // bits of dma burst size

    uint8_t dma_ch_lr[2]; // dynamically allocated DMA channel for APC channel read/write
    uint8_t busy_lr[2]; // DMA transfer is ongoing (busy) or not?
    uint32_t samps_lr[2]; // count of samples actually transfered
    //APC_CH_XFER xfer_lr[2]; //TODO: move to the struct array?

    APC_DCH_CFG_REG * cfg_reg_p; // dual_channel configuration register
    const APC_CH_FIXED * fixed_lr[2]; // L, R channels' fixed configuration

    //TODO: Add more fields for APC channel
} APC_DCH_INFO;

// APC device structure definitions
typedef struct
{
    CSK_APC_RegDef *reg;  // pointer to APC peripheral
    //uint32_t irq_num; // APC IRQ number
    //ISR irq_handler; // APC Interrupt handler

    APC_DCH_INFO dch_array[APC_DCH_COUNT]; // APC dual_channel array
} APC_DEV;

//-----------------------------------------------------------------------------

static const APC_CH_FIXED g_chs_fixed[APC_CH_COUNT] = {
        // APC RX (IN) channel
        { 0, 1, DMA_HSID_APC_IN_CH0, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH0L_DATA_OFFSET },
        { 1, 1, DMA_HSID_APC_IN_CH1, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH0R_DATA_OFFSET },
        { 2, 1, DMA_HSID_APC_IN_CH2, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH1L_DATA_OFFSET },
        { 3, 1, DMA_HSID_APC_IN_CH3, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH1R_DATA_OFFSET },
        { 4, 1, DMA_HSID_APC_IN_CH4, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH2L_DATA_OFFSET },
        { 5, 1, DMA_HSID_APC_IN_CH5, APC_IN_FIFO_DEPTH_DEF, APC_RX_CH2R_DATA_OFFSET },
        // APC TX (OUT) channel
        { 6, 0, DMA_HSID_APC_OUT_CH0, APC_OUT_FIFO_DEPTH_DEF, APC_TX_CH0L_DATA_OFFSET },
        { 7, 0, DMA_HSID_APC_OUT_CH1, APC_OUT_FIFO_DEPTH_DEF, APC_TX_CH0R_DATA_OFFSET },
        { 8, 0, DMA_HSID_APC_OUT_CH2, APC_OUT_FIFO_DEPTH_DEF, APC_TX_CH1L_DATA_OFFSET },
        { 9, 0, DMA_HSID_APC_OUT_CH3, APC_OUT_FIFO_DEPTH_DEF, APC_TX_CH1R_DATA_OFFSET },
};

static const uint32_t g_dch_cfg_regs[APC_DCH_COUNT] = {
        APC_BASE + OFFSET_APC_APC_RX_CH0_CFG,
        APC_BASE + OFFSET_APC_APC_RX_CH1_CFG,
        APC_BASE + OFFSET_APC_APC_RX_CH2_CFG,
        APC_BASE + OFFSET_APC_APC_TX_CH0_CFG,
        APC_BASE + OFFSET_APC_APC_TX_CH1_CFG,
};

static APC_DEV g_apc_dev;

//#define IS_APC_CH_IN(ch)    (g_apc_dev.chs_info[ch].fixed->ch_dir)
//#define IS_APC_CH_OUT(ch)    !(g_apc_dev.chs_info[ch].fixed->ch_dir)

static volatile uint32_t g_init_cnt = 0U;
static volatile CSK_APC_RegDef * gApcReg = NULL;

#define IS_APC_INITIALIZED()        (g_init_cnt > 0)
#define IS_APC_DCH_INITIALIZED(dch)   (g_apc_dev.dch_array[dch].intf_type != APC_INTF_UNSET && \
                                        g_apc_dev.dch_array[dch].setup_done == 1)
#define IS_APC_DCH_OCCUPIED(dch)      (g_apc_dev.dch_array[dch].intf_type != APC_INTF_UNSET)

static void apc_dma_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param);
static void apc_irq_handler (void);

//=============================================================================

// APC interrupt-related operations for APC channel

__inline static void apc_ch_intr_enable(APC_CH ch, uint32_t intr_bits)
{
    if (ch >= APC_CH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (ch >= APC_CH_IN_COUNT) {
        intr_bits <<= (ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_TX_MSK.all &= ~intr_bits;
    } else {
        intr_bits <<= ch * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_RX_MSK.all &= ~intr_bits;
    }
}

__inline static void apc_ch_intr_disable(APC_CH ch, uint32_t intr_bits)
{
    if (ch >= APC_CH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (ch >= APC_CH_IN_COUNT) {
        intr_bits <<= (ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_TX_MSK.all |= intr_bits;
    } else {
        intr_bits <<= ch * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_RX_MSK.all |= intr_bits;
    }
}

__inline static void apc_ch_intr_clear(APC_CH ch, uint32_t intr_bits)
{
    if (ch >= APC_CH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (ch >= APC_CH_IN_COUNT) {
        intr_bits <<= (ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_TX_CLR.all |= intr_bits;
    } else {
        intr_bits <<= ch * APC_FIFO_INTR_CNT;
        CSK_APC->REG_APC_INTR_RX_CLR.all |= intr_bits;
    }
}

__inline static uint32_t apc_ch_intr_status(APC_CH ch)
{
    if (ch >= APC_CH_COUNT) return 0;
    uint32_t val;
    if (ch >= APC_CH_IN_COUNT) {
        val = CSK_APC->REG_APC_INTR_TX_ISR.all;
        val >>= (ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT;
        //return (( CSK_APC->APC_REG_APC_INTR_TX_ISR >> \
        //        ((ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT) ) & \
        //        APC_FIFO_INTR_MASK);
    } else {
        val = CSK_APC->REG_APC_INTR_RX_ISR.all;
        val >>= ch * APC_FIFO_INTR_CNT;
        //return (( CSK_APC->APC_REG_APC_INTR_RX_ISR >> \
        //        (ch * APC_FIFO_INTR_CNT) ) & APC_FIFO_INTR_MASK);
    }
    return (val & APC_FIFO_INTR_MASK);
}

__inline static uint32_t apc_ch_intr_raw_status(APC_CH ch)
{
    if (ch >= APC_CH_COUNT) return 0;
    uint32_t val;
    if (ch >= APC_CH_IN_COUNT) {
        val = CSK_APC->REG_APC_INTR_TX_IRSR.all;
        val >>= (ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT;
        //return (( CSK_APC->APC_REG_APC_INTR_TX_IRSR >> \
        //        ((ch - APC_CH_IN_COUNT) * APC_FIFO_INTR_CNT) ) & \
        //        APC_FIFO_INTR_MASK);
    } else {
        val = CSK_APC->REG_APC_INTR_RX_IRSR.all;
        val >>= ch * APC_FIFO_INTR_CNT;
        //return (( CSK_APC->APC_REG_APC_INTR_RX_IRSR >> \
        //        (ch * APC_FIFO_INTR_CNT) ) & APC_FIFO_INTR_MASK);
    }
    return (val & APC_FIFO_INTR_MASK);
}

// APC interrupt-related operations for APC dual_channel

__inline static void apc_dch_intr_enable(APC_DCH dch, uint32_t intr_bits)
{
    if (dch >= APC_DCH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (dch >= APC_DCH_IN_COUNT) {
        intr_bits <<= ((dch - APC_DCH_IN_COUNT) << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_TX_MSK.all &= ~intr_bits;
    } else {
        intr_bits <<= (dch << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_RX_MSK.all &= ~intr_bits;
    }
}

__inline static void apc_dch_intr_disable(APC_DCH dch, uint32_t intr_bits)
{
    if (dch >= APC_DCH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (dch >= APC_DCH_IN_COUNT) {
        intr_bits <<= ((dch - APC_DCH_IN_COUNT) << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_TX_MSK.all |= intr_bits;
    } else {
        intr_bits <<= (dch << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_RX_MSK.all |= intr_bits;
    }
}

__inline static void apc_dch_intr_clear(APC_DCH dch, uint32_t intr_bits)
{
    if (dch >= APC_DCH_COUNT) return;
    intr_bits &= APC_FIFO_INTR_MASK;
    if (dch >= APC_DCH_IN_COUNT) {
        intr_bits <<= ((dch - APC_DCH_IN_COUNT) << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_TX_CLR.all = intr_bits;
    } else {
        intr_bits <<= (dch << 1) * APC_FIFO_INTR_CNT;
        intr_bits |= (intr_bits << APC_FIFO_INTR_CNT);
        CSK_APC->REG_APC_INTR_RX_CLR.all = intr_bits;
    }
}

// status of interrupts: Left channel @ bit[4:0], Right channel @ bit[9:5]
__inline static uint32_t apc_dch_intr_status(APC_DCH dch)
{
    if (dch >= APC_DCH_COUNT) return 0;
    uint32_t val, mask;
    if (dch >= APC_DCH_IN_COUNT) {
        val = CSK_APC->REG_APC_INTR_TX_ISR.all;
        val >>= ((dch - APC_DCH_IN_COUNT) << 1) * APC_FIFO_INTR_CNT;
    } else {
        val = CSK_APC->REG_APC_INTR_RX_ISR.all;
        val >>= (dch << 1) * APC_FIFO_INTR_CNT;
    }
    mask = APC_FIFO_INTR_MASK;
    mask |= (mask << APC_FIFO_INTR_CNT);
    return (val & mask);
}

// raw status of interrupts: Left channel @ bit[4:0], Right channel @ bit[9:5]
__inline static uint32_t apc_dch_intr_raw_status(APC_DCH dch)
{
    if (dch >= APC_DCH_COUNT) return 0;
    uint32_t val, mask;
    if (dch >= APC_DCH_IN_COUNT) {
        val = CSK_APC->REG_APC_INTR_TX_IRSR.all;
        val >>= ((dch - APC_DCH_IN_COUNT) << 1) * APC_FIFO_INTR_CNT;
    } else {
        val = CSK_APC->REG_APC_INTR_RX_IRSR.all;
        val >>= (dch << 1) * APC_FIFO_INTR_CNT;
    }
    mask = APC_FIFO_INTR_MASK;
    mask |= (mask << APC_FIFO_INTR_CNT);
    return (val & mask);
}

// static inline void apc_pclk_enable() {
//     SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_APC_PCLK = 1;
// }

static inline void apc_pclk_disable() {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_APC_PCLK = 0;
}

//initialize AUDPLL if necessary, and set audio post div
void init_audpll_audio_clk(uint32_t req_freq)
{
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_POSTDIV          = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_PFD              = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_CP               = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_REFDIV           = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_FBDIV            = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_VCO              = 0x0;
    //AUDPLL_CFG->REG_0X000.bit.PD_AUDPLL_LKDET            = 0x0;

    uint32_t reg_val;
    reg_val = AUDPLL_CFG->REG_0X000.all;
    if ((reg_val & 0x7FUL) != 0) { // 7 bits set to 0
        reg_val &= ~0x7FUL;
        AUDPLL_CFG->REG_0X000.all = reg_val;
    }

    //BSD: why is AUDPLL_CFG->REG_0X004.bit.RST_AUDPLL_FBSDM absent?
    //AUDPLL_CFG->REG_0X004.bit.RST_AUDPLL_REFDIV          = 0x0;
    //AUDPLL_CFG->REG_0X004.bit.RST_AUDPLL_FBDIV           = 0x0;
    //AUDPLL_CFG->REG_0X004.bit.RST_AUDPLL_POSTDIV         = 0x0;
    //AUDPLL_CFG->REG_0X004.bit.RST_AUDPLL_LKDET           = 0x0;

    reg_val = AUDPLL_CFG->REG_0X004.all;
    if ((reg_val & 0x1FUL) != 0) { // 5 bits set to 0
        reg_val &= ~0x1FUL;
        AUDPLL_CFG->REG_0X004.all = reg_val;
    }

    // 27 indicates div = 54 (AUDPLL_AUDIO_FREQ = VCO_FREQ / div)
    //AUDPLL_CFG->REG_0X018.bit.POSTDIV_AUDCLK_SEL = 27;
    if (req_freq == 0)
        req_freq = DEF_AUDPLL_AUDIO_FREQ;
    reg_val = (DEF_AUDPLL_VCO_FREQ + (req_freq >> 1)) / req_freq;
    reg_val >>= 1;
    if (AUDPLL_CFG->REG_0X018.bit.POSTDIV_AUDCLK_SEL != reg_val)
        AUDPLL_CFG->REG_0X018.bit.POSTDIV_AUDCLK_SEL = reg_val;
}

//-------------------------------------------------------------------------------

/**
  \fn          int32_t apc_initialize (void)
  \brief       Initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_initialize (void)
{
    uint32_t i;
    APC_DCH_INFO *pdci;

    // Check if already initialized
    gApcReg = CSK_APC;
    g_init_cnt++;
    if (g_init_cnt > 1U) { return CSK_DRIVER_OK; }

    //FIXME: enable APC pclk?
    apc_pclk_enable();

    // Reset APC RX & TX (disable APC at first)
    CSK_APC->REG_APC_CFG.all = APC_RX_PATH_RESET | APC_TX_PATH_RESET;

    // initialize all APC channels' information
    memset(&g_apc_dev, 0, sizeof(APC_DEV));
    /* 总共5个通道，3个IN，2个OUT */
    for (i=0; i<APC_DCH_COUNT; i++) {
        pdci = &g_apc_dev.dch_array[i];
        pdci->cfg_reg_p = (APC_DCH_CFG_REG *)g_dch_cfg_regs[i];
        pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN); // disable each L/R channel

        pdci->fixed_lr[0] = &g_chs_fixed[i*2];
        pdci->fixed_lr[1] = &g_chs_fixed[i*2+1];
        pdci->dma_ch_lr[0] = DMA_CHANNEL_ANY;
        pdci->dma_ch_lr[1] = DMA_CHANNEL_ANY;
    }

    g_apc_dev.reg = CSK_APC;

    // disable / clear all interrupts
    CSK_APC->REG_APC_INTR_TX_MSK.all = 0xFFFFF; // 1 = disable
    CSK_APC->REG_APC_INTR_RX_MSK.all = 0x3FFFFFFF; // 1 = disable
    if (CSK_APC->REG_APC_INTR_TX_IRSR.all)
        //CSK_APC->REG_APC_INTR_TX_CLR.all = CSK_APC->REG_APC_INTR_TX_IRSR.all; // write 1 to clear
        CSK_APC->REG_APC_INTR_TX_CLR.all = 0xFFFFF; // write 1 to clear
    if (CSK_APC->REG_APC_INTR_RX_IRSR.all)
        //CSK_APC->REG_APC_INTR_RX_CLR.all = CSK_APC->REG_APC_INTR_RX_IRSR.all; // write 1 to clear
        CSK_APC->REG_APC_INTR_RX_CLR.all = 0x3FFFFFFF; // write 1 to clear

    // Register APC ISR
    register_ISR(IRQ_APC_VECTOR, (ISR)apc_irq_handler, NULL);

    // Enable APC IRQ
    enable_IRQ(IRQ_APC_VECTOR);

    // Initialize DMA Controller
    dma_initialize();

    // Enable APC
    CSK_APC->REG_APC_CFG.all = APC_ENABLE; //  | APC_AUTO_CLK_GATING

    return CSK_DRIVER_OK;
}


/**
  \fn          int32_t apc_uninitialize (void)
  \brief       De-initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_uninitialize (void)
{
    // Check if DMA is initialized
    if (g_init_cnt == 0U) { return CSK_DRIVER_ERROR; }

    g_init_cnt--;
    if (g_init_cnt != 0U) { return CSK_DRIVER_OK; }

    // Abort all transfers if any
    uint32_t i;
    for (i=0; i<APC_DCH_COUNT; i++)
        apc_dual_channel_abort(i);

    // Disable APC
    CSK_APC->REG_APC_CFG.all &= ~APC_ENABLE;

    //FIXME: disable APC pclk?
    apc_pclk_disable();

    // Disable APC IRQ
    disable_IRQ(IRQ_APC_VECTOR);

    // Unregister APC ISR
    register_ISR(IRQ_APC_VECTOR, NULL, NULL);

    // Uninitialize DMA Controller
    dma_uninitialize();

    return CSK_DRIVER_OK;
}


/**
  \fn          int32_t apc_channel_setup ();
  \brief       Configure APC channel to transfer data for ADC(AMIC, DMIC), DAC, and I2S etc.
               NOTE: the last setup is vaild if the function is called many times
                    for the same APC channel.
  \param[in]   ch           The selected APC Channel, see definitions of APC Channel.
  \param[in]   itf_type     one of interface type, ADC, DAC, or I2S.
  \param[in]   usr_param    the parameter, whose meaning varies with the interface type.
  \param[in]   cb_event  Pointer to \ref CSK_APC_SignalEvent_t
  \param[in]   usr_param  User-defined value, acts as last parameter of cb_event
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/

// generally called in XXX_Initialize()
int32_t apc_dual_channel_acquire (APC_DCH    dch,
                           APC_INTF_TYPE    itf_type,
                           APC_INTF_INDEX   itf_idx,
                           CSK_APC_SignalEvent_t    cb_event,
                           uint32_t         usr_param)
{
    int32_t ret = CSK_DRIVER_OK;

    // Check if APC is initialized
    if ( !IS_APC_INITIALIZED() )
        return CSK_DRIVER_ERROR;

    // Check if parameters are valid, i.e. APC dual_channel is valid
    if (dch >= APC_DCH_COUNT || itf_type >= APC_INTF_TYPE_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC channel is occupied
    if ( IS_APC_DCH_OCCUPIED(dch) ) {
        LOGD("%s: APC dual_channel %d has been used for Interface type %d !!\r\n",
                __func__, dch, g_apc_dev.dch_array[dch].intf_type);
        return CSK_DRIVER_ERROR;
    }

    uint8_t gie = GINT_enabled();

    // disable Global interrupt (NO interrupts, NO task scheduling)
    if (gie)    disable_GINT();

    // Check the correspondence between dual_channel and interface type & index
    switch (dch) {
    case APC_DCH_IN0: {
        struct APC_REG_APC_RX_CH0_CFG_BITS *pCfg;
        pCfg = (struct APC_REG_APC_RX_CH0_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_RX_CH0_CFG);
        switch (itf_type) {
        case APC_INTF_VAD:
            if (itf_idx != APC_INTF_IDX_VAD) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH0_SRC_SEL = RX_CH0_SRC_VAD;
            break;
        case APC_INTF_ADC:
            if (itf_idx != APC_INTF_IDX_ADC01) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH0_SRC_SEL = RX_CH0_SRC_ADC01;
            break;
        case APC_INTF_I2S_IN:
            if (itf_idx != APC_INTF_IDX_I2S0) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH0_SRC_SEL = RX_CH0_SRC_I2S0;
            break;
        default:
            ret = CSK_DRIVER_ERROR_PARAMETER;
            break;
            //return CSK_DRIVER_ERROR_PARAMETER;
        }
        break;
    }

    case APC_DCH_IN1: {
        struct APC_REG_APC_RX_CH1_CFG_BITS *pCfg;
        pCfg = (struct APC_REG_APC_RX_CH1_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_RX_CH1_CFG);
        switch (itf_type) {
        case APC_INTF_ADC:
            if (itf_idx != APC_INTF_IDX_ADC23) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH1_SRC_SEL = RX_CH1_SRC_ADC23;
            break;
        case APC_INTF_I2S_IN:
            if (itf_idx != APC_INTF_IDX_I2S1) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH1_SRC_SEL = RX_CH1_SRC_I2S1;
            break;
        default:
            ret = CSK_DRIVER_ERROR_PARAMETER;
            break;
            //return CSK_DRIVER_ERROR_PARAMETER;
        }
        break;
    }

    case APC_DCH_IN2: {
        struct APC_REG_APC_RX_CH2_CFG_BITS *pCfg;
        pCfg = (struct APC_REG_APC_RX_CH2_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_RX_CH2_CFG);
        switch (itf_type) {
        case APC_INTF_ECHO:
            if (itf_idx != APC_INTF_IDX_ECHO) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->RX_CH2_SRC_SEL = RX_CH2_SRC_ECHO;
            break;
        case APC_INTF_I2S_IN:
            if (itf_idx == APC_INTF_IDX_I2S2)
                pCfg->RX_CH2_SRC_SEL = RX_CH2_SRC_I2S2;
            else if (itf_idx == APC_INTF_IDX_I2S1)
                pCfg->RX_CH2_SRC_SEL = RX_CH2_SRC_I2S1;
            else
                ret = CSK_DRIVER_ERROR_PARAMETER;
                //return CSK_DRIVER_ERROR_PARAMETER;
            break;
        default:
            ret = CSK_DRIVER_ERROR_PARAMETER;
            //return CSK_DRIVER_ERROR_PARAMETER;
        }
        break;
    }

    case APC_DCH_OUT0: {
        //struct APC_REG_APC_TX_CH0_CFG_BITS *pCfg;
        //pCfg = (struct APC_REG_APC_TX_CH0_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_TX_CH0_CFG);
        if (itf_type != APC_INTF_I2S_OUT || itf_idx != APC_INTF_IDX_I2S1) {
            ret = CSK_DRIVER_ERROR_PARAMETER;
            //return CSK_DRIVER_ERROR_PARAMETER;
        }
        // Support I2S output only and NO dst_sel register field is configured...
        break;
    }

    case APC_DCH_OUT1: {
        struct APC_REG_APC_TX_CH1_CFG_BITS *pCfg;
        pCfg = (struct APC_REG_APC_TX_CH1_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_TX_CH1_CFG);
        switch (itf_type) {
        case APC_INTF_DAC:
            if (itf_idx != APC_INTF_IDX_DAC) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->TX_CH1_DST_SEL = TX_CH1_DST_DAC01;
            break;
        case APC_INTF_I2S_OUT:
            if (itf_idx != APC_INTF_IDX_I2S2) {
                ret = CSK_DRIVER_ERROR_PARAMETER;
                break;
                //return CSK_DRIVER_ERROR_PARAMETER;
            }
            pCfg->TX_CH1_DST_SEL = TX_CH1_DST_I2S2;
            break;
        default:
            ret = CSK_DRIVER_ERROR_PARAMETER;
            //return CSK_DRIVER_ERROR_PARAMETER;
        }
        break;
    }

    default:
        ret = CSK_DRIVER_ERROR_PARAMETER;
        //return CSK_DRIVER_ERROR_PARAMETER;

    } // end switch (dch)

    if (ret == CSK_DRIVER_OK) {
        APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
        pdci->intf_type = itf_type;
        pdci->intf_idx = itf_idx;
        pdci->cb_event = cb_event;
        pdci->usr_param = usr_param;
    }

    // enable Global interrupt
    if (gie)    enable_GINT();

    return ret; // CSK_DRIVER_OK
}


// generally called in XXX_Uninitialize()
int32_t apc_dual_channel_release (APC_DCH    dch)
{
    // Check if APC is initialized
    if ( !IS_APC_INITIALIZED() )
        return CSK_DRIVER_ERROR;

    // Check if parameters are valid, i.e. APC dual_channel is valid
    if (dch >= APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC channel is occupied
    if ( !IS_APC_DCH_OCCUPIED(dch) ) {
        LOGD("%s: APC dual_channel %d has NOT been occupied, do nothing...\r\n",
                __func__, dch);
        return CSK_DRIVER_OK;
    }

    // Abort current transfer if necessary
    apc_dual_channel_abort(dch);

    // Disable all types of interrupts
    apc_dch_intr_disable(dch, APC_FIFO_INTR_ALL);

    // Disable L/R channel of dual_channel
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN);

    //TODO: other dual_channel cleanup operations?

    uint8_t gie = GINT_enabled();

    // disable Global interrupt (NO interrupts, NO task scheduling)
    if (gie)    disable_GINT();

    // Clear current dual_channel settings
    memset(pdci, 0, sizeof(APC_DCH_INFO));

    // enable Global interrupt
    if (gie)    enable_GINT();

    return CSK_DRIVER_OK;
}


// retrieve interface type, channel no. of the type, and channel direction for APC channel
int32_t apc_dual_channel_owner (APC_DCH dch,
                            APC_INTF_TYPE *itf_type_p,
                            APC_INTF_INDEX *itf_idx_p,
                            uint8_t *ch_dir_p)
{
    // Check if APC is initialized
    if ( !IS_APC_INITIALIZED() )
        return CSK_DRIVER_ERROR;

    // Check if parameters are valid, i.e. APC channel is valid
    if (dch >= APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if (itf_type_p != NULL)
        *itf_type_p = pdci->intf_type;
    if (itf_idx_p != NULL)
        *itf_idx_p = pdci->intf_idx;
    if (ch_dir_p != NULL)
        *ch_dir_p = pdci->fixed_lr[0]->ch_dir;

    return CSK_DRIVER_OK;
}


// generally called in XXX_Control()
// ch_mix   0: no mix, 1: mix @ left channel (by default), 2: mix @ right channel
int32_t apc_dual_channel_setup (APC_DCH     dch,
                           APC_CHMODE       ch_mode, // 16, 24 LSB, 32 or 24 MSB?
                           uint8_t          ch_sel, // select APC_DCH_BMP_LEFT, APC_DCH_BMP_RIGHT, or APC_DCH_BMP_STEREO
                           uint8_t          ch_mix, // read L/R channel as a whole, or read L/R channel respectively
                           uint8_t          trim_16bits) // only valid for 24 MSB and 32 channel mode, when set to 1:
                                                         // record: trim low 16bits, get 16bits audio data
                                                         // playback: 16bits audio data is placed at high 16bits of WORD
{
    // Check if APC is initialized
    if ( !IS_APC_INITIALIZED() )
        return CSK_DRIVER_ERROR;

    // Check if parameters are valid, i.e. APC channel is valid
    if (dch >= APC_DCH_COUNT || ch_mode >= APC_CHMODE_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC channel is occupied
    if ( !IS_APC_DCH_OCCUPIED(dch) ) {
        LOGD("%s: APC dual_channel %d has NOT been occupied, "
                "SHOULD call apc_channel_acquire() first!!\r\n",
                __func__, dch);
        return CSK_DRIVER_ERROR;
    }

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    pdci->ch_sel = ch_sel & APC_DCH_BMP_MASK;
    if ((ch_sel & APC_DCH_BMP_MASK) == APC_DCH_BMP_STEREO && ch_mix) {
        pdci->mix_mode = APC_DCH_MIXED_MODE;
        pdci->mix_target = ch_mix & 0x2 ? 1 : 0; // 1 @ right, 0 @ left (or NOT mixed)
    } else {
        pdci->mix_mode = APC_DCH_SEPARATE_MODE;
        pdci->mix_target = 0; // 0 indicates NOT mixed
    }

    pdci->ch_mode = ch_mode;
    switch (ch_mode) {
    case APC_CHMODE_16BITS:
        pdci->samp_bits = 16;
        break;
    case APC_CHMODE_24BITS_LOW:
    case APC_CHMODE_24BITS_HIGH:
        pdci->samp_bits = 24;
        break;
    case APC_CHMODE_32BITS:
        pdci->samp_bits = 32;
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // write setup parameters into register
    APC_DCH_CFG_REG cfg_reg;
    uint32_t fifo_depth, val, entries;

    // bit width is fixed on WORD
    pdci->dma_width_bits = DMA_WIDTH_WORD;
    if (trim_16bits != 0 &&
        (ch_mode == APC_CHMODE_24BITS_HIGH || ch_mode == APC_CHMODE_32BITS))
        pdci->dma_width_bits = DMA_WIDTH_HALFWORD;

    // FIFO depth of left channel is supposed to be same as one of right channel
    fifo_depth = pdci->mix_target ?
                 pdci->fixed_lr[1]->fifo_depth :
                 pdci->fixed_lr[0]->fifo_depth;

    cfg_reg.all = inw(g_dch_cfg_regs[dch]);
    if (pdci->ch_sel & APC_DCH_BMP_LEFT)
       cfg_reg.bit.CH_L_FIFO_FLUSH = 1; // flush FIFO of left channel
    if (pdci->ch_sel & APC_DCH_BMP_RIGHT)
       cfg_reg.bit.CH_R_FIFO_FLUSH = 1; // flush FIFO of right channel

    cfg_reg.bit.CH_L_MODE = pdci->ch_mode;
    cfg_reg.bit.CH_R_MODE = pdci->ch_mode;
    cfg_reg.bit.DCH_STEREO_MODE = pdci->mix_mode;

    if (pdci->mix_mode == APC_DCH_MIXED_MODE) {
        // try setting DMA burst size to FIFO depth of APC channel
        entries = fifo_depth; // 1; // 4; // 8; //
        // For mixed/stereo mode, APC DMA request threshold is the sum of LEFT channel and RIGHT channel.
        // When sample bits is 24 or 32, data of burst size (fifo_depth) are divided equally into two parts,
        // one for LEFT channel and the other for RIGHT channel.
        // When sample bits is 16, one word (32-bit) of data is divided equally into two parts,
        // one 16-bit for LEFT channel and the other 16-bit for RIGHT channel, and each 16bit occupies one FIFO entry,
        // so data words of burst size (fifo_depth) are split equally, 16-bit data of burst size for LEFT channel and
        // 16-bit data of burst size for RIGHT channel.
        val = ITEMS_TO_BSIZE(entries);
        pdci->dma_bsize_bits = (pdci->samp_bits > 16 ? val : ITEMS_TO_BSIZE(entries >> 1));
    } else {
        // try setting DMA burst size to half FIFO depth of APC channel
        entries = fifo_depth >> 1;
        // For separated/mono mode, APC DMA request threshold is set for LEFT channel or RIGHT channel respectively.
        // When sample bits is 16, one word (32-bit) of data includes two 16-bit samples, and occupies one FIFO entry,
        // so length (word count) of burst size is just half of FIFO depth.
        val = ITEMS_TO_BSIZE(entries);
        //pdci->dma_bsize_bits = (pdci->samp_bits > 16 ? val : ITEMS_TO_BSIZE(entries >> 1));
        pdci->dma_bsize_bits = val;
    }

    assert(pdci->dma_bsize_bits <= DMA_BSIZE_16);
    cfg_reg.bit.DCH_DMA_THD_SEL = val & 0x3;

//    pdci->dma_bsize_bits = 0;           // FIXME:
//    cfg_reg.bit.DCH_DMA_THD_SEL = 0;    // FIXME: OK

//    pdci->dma_bsize_bits = 1;           // FIXME:
//    cfg_reg.bit.DCH_DMA_THD_SEL = 2;    // FIXME: OK

    assert(pdci->cfg_reg_p != NULL);
    pdci->cfg_reg_p->all = cfg_reg.all;

#if 0 // CANNOT write data into APC channel before it's enabled with current IP design!!
    // Fill TX FIFO with 0 (L/R channel has the same direction)
    if (pdci->fixed_lr[0]->ch_dir == 0) {
        uint32_t i, addr;
        addr = pdci->mix_target ?
               pdci->fixed_lr[1]->data_addr :
               pdci->fixed_lr[0]->data_addr;
        val = fifo_depth;
        if (pdci->mix_mode == APC_DCH_SEPARATE_MODE)
            val >>= 1;
        for (i=0; i<val; i++)
            outw(addr, 0);
    }

    // Clear all interrupts' status (MUST BE LAST STEP!)
    apc_dch_intr_clear(dch, APC_FIFO_INTR_MASK);
#endif

    // APC channel setup is done
    pdci->setup_done = 1;

    return CSK_DRIVER_OK;
}

// SRC MODE
#define APC_SRC_48K_8KHZ    0 // 48KHz->8KHz
#define APC_SRC_48K_16KHZ   1 // 48KHz->16KHz
// SRC CHANNEL ENABLE (LR channels SHOULD be both enabled or disabled)
#define APC_SRC_DISABLE     0x0
#define APC_SRC_ENABLE      0x3
// Sample Rate
#define APC_SAMP_RATE_48KHZ     48000
#define APC_SAMP_RATE_16KHZ     16000
#define APC_SAMP_RATE_8KHZ      8000

//NOTE: apc_echo_setup() SHOULD be called after apc_dual_channel_setup() for the ECHO dual_channel!!
int32_t apc_echo_setup (APC_DCH  dch,
                        uint32_t tx_samp_rate,
                        uint32_t echo_samp_rate,
                        uint8_t auto_feed)
{
    int32_t ret;
    if (dch != APC_DCH_ECHO) {
        LOGD("%s: Invalid ECHO dual_channel (dch = %d)!\n",  __func__, dch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if (pdci->setup_done == 0) {
        LOGD("%s: SHOULD be called after apc_dual_channel_setup()!\n",  __func__);
        return CSK_DRIVER_ERROR;
    }

    struct APC_REG_APC_RX_CH2_CFG_BITS *rx2_cfg_p;
    rx2_cfg_p = (struct APC_REG_APC_RX_CH2_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_RX_CH2_CFG);
    assert( (uint32_t)pdci->cfg_reg_p == (uint32_t)rx2_cfg_p );

    // setup SRC (Sampling Rate Conversion)
    ret = CSK_DRIVER_OK;
    rx2_cfg_p->SRC_CH_EN = APC_SRC_DISABLE;
    if (echo_samp_rate != tx_samp_rate) { // need SRC
        if (tx_samp_rate == APC_SAMP_RATE_48KHZ) {
            switch (echo_samp_rate) {
            case APC_SAMP_RATE_16KHZ:
                rx2_cfg_p->SRC_MODE = APC_SRC_48K_16KHZ;
                rx2_cfg_p->SRC_CH_EN = APC_SRC_ENABLE;
                break;
            case APC_SAMP_RATE_8KHZ:
                rx2_cfg_p->SRC_MODE = APC_SRC_48K_8KHZ;
                rx2_cfg_p->SRC_CH_EN = APC_SRC_ENABLE;
                break;
            default:
                ret = CSK_DRIVER_ERROR;
                break;
            } // end switch
        } else {
            ret = CSK_DRIVER_ERROR;
        }
    }

    if (ret != CSK_DRIVER_OK) {
        CLOGW("%s: %d => %d Hz SRC is NOT supported!\n",
                __func__, tx_samp_rate, echo_samp_rate);
        return ret;
    }

    // write 1 to generate single pulse to clear SRC data path
    rx2_cfg_p->SRC_CLR = 1;

    struct APC_REG_APC_TX_CH1_CFG_BITS *tx1_cfg_p;
    tx1_cfg_p = (struct APC_REG_APC_TX_CH1_CFG_BITS *)(APC_BASE + OFFSET_APC_APC_TX_CH1_CFG);
    // whether to set Auto feed 0 when reading TX channel?
    tx1_cfg_p->TX_CH1_RD_AUTOFEED = (auto_feed ? 1 : 0);

    return CSK_DRIVER_OK;
}


// sample_cnt SHOULD be EVEN when 16-bit sample!
static int32_t
apc_channel_read_internal (APC_DCH    dch,
                          uint8_t   lr_bmp,
                          uint32_t *sample_data,
                          uint32_t  sample_cnt,
                          uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
                          uint32_t  dst_scat) // destination scatter setting (default 0, NO SG support)
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_IN_MAX || sample_data == NULL || sample_cnt == 0)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    lr_bmp &= APC_DCH_BMP_MASK;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    // 0 = left, 1 = right (use left or right channel for dual_channel)
    uint8_t lr_idx = (lr_bmp & APC_DCH_BMP_LEFT)? 0 : 1;
    if (pdci->mix_target == 1) // mixed @ right channel
        lr_idx = 1;

    // sample_cnt should be even when 16-bit sample
    if (pdci->ch_mode == APC_CHMODE_16BITS && (sample_cnt & 0x1) != 0) {
        CLOGE("%s: sample count (%d) of 16bit sample should be even!!\r\n",
                __func__, sample_cnt);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // for single channel, Only STEREO_MODE (separated L/R channel data) is supported
    // for dual channel, Only MONO_MODE (mixed L/R channel data) is supported
    if ((lr_bmp != APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_MIXED_MODE) ||
        (lr_bmp == APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_SEPARATE_MODE)) {
        CLOGE("%s: 1 channel requires mono mode, 2 channels requires stereo mode!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    // Only ADC / I2S IN / ECHO interface can be read
    if (pdci->intf_type != APC_INTF_VAD &&
        pdci->intf_type != APC_INTF_ADC &&
        pdci->intf_type != APC_INTF_I2S_IN &&
        pdci->intf_type != APC_INTF_ECHO)
        return CSK_DRIVER_ERROR;

    // just return busy if the channel is busy
    if (pdci->busy_lr[lr_idx] != 0) {
        CLOGE("%s: apc dch %d (lr_idx = %d) is busy!!\r\n", __func__, dch, lr_idx);
        return CSK_DRIVER_ERROR_BUSY;
    }

    // select some free DMA channel
    uint8_t *dma_ch_p = &pdci->dma_ch_lr[lr_idx];
    uint32_t usr_param = (dch << 8) | lr_bmp;
    *dma_ch_p = dma_channel_select(dma_ch_p,
                                    apc_dma_event,
                                    usr_param,
                                    DMA_CACHE_SYNC_DST);
    if (*dma_ch_p == DMA_CHANNEL_ANY) {
        LOGD("%s: NO free DMA channel!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    uint32_t control, config_low, config_high;

    control = DMA_CH_CTLL_DST_WIDTH(pdci->dma_width_bits) | DMA_CH_CTLL_SRC_WIDTH(pdci->dma_width_bits) |
            DMA_CH_CTLL_DST_BSIZE(pdci->dma_bsize_bits) | DMA_CH_CTLL_SRC_BSIZE(pdci->dma_bsize_bits) |
            DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_TTFC_P2M |
            DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
    config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(pdci->fixed_lr[lr_idx]->dma_hsid);
//    config_high = DMA_CH_CFGH_PROTCTL(1) | DMA_CH_CFGH_SRC_PER(pdci->fixed_lr[lr_idx]->dma_hsid);   // FIXME:

    // configure DMA channel
    uint32_t src_addr = pdci->fixed_lr[lr_idx]->data_addr;
    if (pdci->dma_width_bits == DMA_WIDTH_HALFWORD) // trim low 16bits
        src_addr += 2; // high 16bits address
    int32_t stat = dma_channel_configure (*dma_ch_p,
                        src_addr, //pdci->fixed_lr[lr_idx]->data_addr
                        (uint32_t) sample_data + buf_offset,
                        pdci->samp_bits > 16 ? sample_cnt : sample_cnt/2,
                        control, config_low, config_high, 0, dst_scat);
    if (stat == -1) {
        LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
        dma_channel_disable(*dma_ch_p, false);
        *dma_ch_p = DMA_CHANNEL_ANY;
        return CSK_DRIVER_ERROR;
    }

    pdci->busy_lr[lr_idx] = 1;
    return CSK_DRIVER_OK;
}


// read APC channel (sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!)
int32_t apc_channel_read (APC_CH    ch,
                          uint32_t *sample_data,
                          uint32_t  sample_cnt,
                          uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                          uint32_t dst_scat) // destination scatter setting (default 0, NO SG support)
{
    return apc_channel_read_internal (APC_CH_TO_DCH(ch),
                                      APC_CH_LR_BMP(ch),
                                      sample_data,
                                      sample_cnt,
                                      buf_offset,
                                      dst_scat);
}


// read APC dual_channel (sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!)
int32_t apc_dual_channel_read (APC_DCH  dch,
                              uint32_t  *sample_data,
                              uint32_t  sample_cnt,
                              uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
                              uint32_t  dst_scat) // destination scatter setting (default 0, NO SG support)
{
    return apc_channel_read_internal (dch,
                                      APC_DCH_BMP_STEREO,
                                      sample_data,
                                      sample_cnt,
                                      buf_offset,
                                      dst_scat);
}


// read mixed data of APC channels 0 ~ 3 (4 IN channels)
// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
int32_t apc_read_quad_channels (uint32_t* sample_data, uint32_t  sample_cnt)
{
    uint32_t dst_scat, dch_bytes, i, j;
    int32_t ret = CSK_DRIVER_OK;

    if (sample_data == NULL || sample_cnt < 4)
        return CSK_DRIVER_ERROR_PARAMETER;

    APC_DCH_INFO *pdci0, *pdci1;
    pdci0 = &g_apc_dev.dch_array[0];
    pdci1 = &g_apc_dev.dch_array[1];

    // check all the APC channel share the same ch_mode, mix_mode, ch_sel
    if ((pdci0->ch_sel & APC_DCH_BMP_MASK) != APC_DCH_BMP_STEREO ||
        (pdci1->ch_sel & APC_DCH_BMP_MASK) != APC_DCH_BMP_STEREO ||
        pdci0->mix_mode != APC_DCH_MIXED_MODE ||
        pdci1->mix_mode != APC_DCH_MIXED_MODE ||
        pdci0->ch_mode != pdci1->ch_mode) {
        return CSK_DRIVER_ERROR;
    }

    // sample_cnt Should be multiple of 4
    if (sample_cnt & 0x3) {
        //return CSK_DRIVER_ERROR_PARAMETER;
        LOGD("%s: sample count should be multiple of 4!\r\n", __func__);
        sample_cnt &= 0x3;
    }

    if (pdci0->ch_mode == APC_CHMODE_16BITS) {
        // count = 1, interval = 1
        dst_scat = (1 << SG_INTERVAL_POS) | (1 << SG_COUNT_POS);
        dch_bytes = 4;
    } else {
        // count = 2, interval = 2
        dst_scat = (2 << SG_INTERVAL_POS) | (2 << SG_COUNT_POS);
        dch_bytes = 8;
    }

    // trigger DMA transfer for each APC dual_channel
    for (i=0; i<2; i++) {
        ret = apc_channel_read_internal(i, APC_DCH_BMP_STEREO, sample_data, sample_cnt/2, i*dch_bytes, dst_scat);
        if (ret != CSK_DRIVER_OK)
            break;
    }

    if (ret == CSK_DRIVER_OK)
        return ret;

    // there's some error, abort all triggered DMA transfer
    for (j=0; j<i; j++) {
        apc_dual_channel_abort(j);
    }

    return ret;
}


// read mixed data of APC channels 0 ~ 5 (4 IN + 2 ECHO)
// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
int32_t apc_read_six_channels (uint32_t* sample_data, uint32_t  sample_cnt)
{
    uint32_t dst_scat, dch_bytes, i, j;
    int32_t ret = CSK_DRIVER_OK;

    if (sample_data == NULL || sample_cnt < 6)
        return CSK_DRIVER_ERROR_PARAMETER;

    APC_DCH_INFO *pdci0, *pdci1, *pdci2;
    pdci0 = &g_apc_dev.dch_array[0];
    pdci1 = &g_apc_dev.dch_array[1];
    pdci2 = &g_apc_dev.dch_array[2];

    // check all the APC channel share the same ch_mode, mix_mode, ch_sel
    if ((pdci0->ch_sel & APC_DCH_BMP_MASK) != APC_DCH_BMP_STEREO ||
        (pdci1->ch_sel & APC_DCH_BMP_MASK) != APC_DCH_BMP_STEREO ||
        (pdci2->ch_sel & APC_DCH_BMP_MASK) != APC_DCH_BMP_STEREO ||
        pdci0->mix_mode != APC_DCH_MIXED_MODE ||
        pdci1->mix_mode != APC_DCH_MIXED_MODE ||
        pdci2->mix_mode != APC_DCH_MIXED_MODE ||
        pdci0->ch_mode != pdci1->ch_mode ||
        pdci0->ch_mode != pdci2->ch_mode) {
        return CSK_DRIVER_ERROR;
    }

    // sample_cnt Should be multiple of 6
    if (sample_cnt % 6) {
        //return CSK_DRIVER_ERROR_PARAMETER;
        LOGD("%s: sample count should be multiple of 6!\r\n", __func__);
        sample_cnt = (sample_cnt / 6) * 6;
    }

    if (pdci0->ch_mode == APC_CHMODE_16BITS) {
        // count = 1, interval = 2
        dst_scat = (2 << SG_INTERVAL_POS) | (1 << SG_COUNT_POS);
        dch_bytes = 4;
    } else {
        // count = 2, interval = 4
        dst_scat = (4 << SG_INTERVAL_POS) | (2 << SG_COUNT_POS);
        dch_bytes = 8;
    }

    // trigger DMA transfer for each APC dual_channel
    for (i=0; i<3; i++) {
        ret = apc_channel_read_internal(i, APC_DCH_BMP_STEREO, sample_data, sample_cnt/3, i*dch_bytes, dst_scat);
        if (ret != CSK_DRIVER_OK)
            break;
    }

    if (ret == CSK_DRIVER_OK)
        return ret;

    // there's some error, abort all triggered DMA transfer
    for (j=0; j<i; j++) {
        apc_dual_channel_abort(j);
    }

    return ret;

}


// sample_cnt SHOULD be EVEN when 16-bit sample!
static int32_t
apc_channel_write_internal (APC_DCH dch,
                           uint8_t  lr_bmp,
                           const uint32_t *sample_data,
                           uint32_t sample_cnt,
                           uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                           uint32_t src_gath) // source gather setting (default 0, NO SG support)

{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch < APC_DCH_OUT_MIN || dch > APC_DCH_OUT_MAX || sample_data == NULL || sample_cnt == 0)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    lr_bmp &= APC_DCH_BMP_MASK;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    // 0 = left, 1 = right (use left or right channel for dual_channel)
    uint8_t lr_idx = (lr_bmp & APC_DCH_BMP_LEFT)? 0 : 1;
    if (pdci->mix_target == 1) // mixed @ right channel
        lr_idx = 1;

    // sample_cnt should be even when 16-bit sample
    if (pdci->ch_mode == APC_CHMODE_16BITS && (sample_cnt & 0x1) != 0)
        return CSK_DRIVER_ERROR_PARAMETER;

    // for single channel, Only MONO_MODE (separated L/R channel data) is supported
    // for dual channel, Only STEREO_MODE (mixed L/R channel data) is supported
    if ((lr_bmp != APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_MIXED_MODE) ||
        (lr_bmp == APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_SEPARATE_MODE))
        return CSK_DRIVER_ERROR;

    // Only DAC / I2S OUT interface can be written
    if (pdci->intf_type != APC_INTF_DAC &&
        pdci->intf_type != APC_INTF_I2S_OUT)
        return CSK_DRIVER_ERROR;

    // just return busy if the channel is busy
    if (pdci->busy_lr[lr_idx] != 0)
        return CSK_DRIVER_ERROR_BUSY;

    // select some free DMA channel
    uint8_t *dma_ch_p = &pdci->dma_ch_lr[lr_idx];
    uint32_t usr_param = (dch << 8) | lr_bmp;
    *dma_ch_p = dma_channel_select(dma_ch_p,
                                    apc_dma_event,
                                    usr_param,
                                    DMA_CACHE_SYNC_SRC);
    if (*dma_ch_p == DMA_CHANNEL_ANY) {
        LOGD("%s: NO free DMA channel!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    uint32_t control, config_low, config_high;

    control = DMA_CH_CTLL_DST_WIDTH(pdci->dma_width_bits) | DMA_CH_CTLL_SRC_WIDTH(pdci->dma_width_bits) |
            DMA_CH_CTLL_DST_BSIZE(pdci->dma_bsize_bits) | DMA_CH_CTLL_SRC_BSIZE(pdci->dma_bsize_bits) |
            DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
            DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
    config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(pdci->fixed_lr[lr_idx]->dma_hsid);
//    config_high = DMA_CH_CFGH_PROTCTL(1) | DMA_CH_CFGH_DST_PER(pdci->fixed_lr[lr_idx]->dma_hsid);   // FIXME:

    // configure DMA channel
    uint32_t dst_addr = pdci->fixed_lr[lr_idx]->data_addr;
    if (pdci->dma_width_bits == DMA_WIDTH_HALFWORD) // trim low 16bits
        dst_addr += 2; // high 16bits address
    int32_t stat = dma_channel_configure (*dma_ch_p,
                        (uint32_t) sample_data + buf_offset,
                        dst_addr, //pdci->fixed_lr[lr_idx]->data_addr,
                        pdci->samp_bits > 16 ? sample_cnt : sample_cnt/2,
                        control, config_low, config_high, src_gath, 0);
    if (stat == -1) {
        LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
        dma_channel_disable(*dma_ch_p, false);
        *dma_ch_p = DMA_CHANNEL_ANY;
        return CSK_DRIVER_ERROR;
    }

    pdci->busy_lr[lr_idx] = 1;
    return CSK_DRIVER_OK;
}


// write APC channel (sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!)
int32_t apc_channel_write (APC_CH   ch,
                           const uint32_t *sample_data,
                           uint32_t sample_cnt,
                           uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                           uint32_t src_gath) // source gather setting (default 0, NO SG support)
{
    return apc_channel_write_internal (APC_CH_TO_DCH(ch),
                                       APC_CH_LR_BMP(ch),
                                       sample_data,
                                       sample_cnt,
                                       buf_offset,
                                       src_gath);
}


// write APC dual_channel (sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!)
int32_t apc_dual_channel_write (APC_DCH dch,
                               const uint32_t *sample_data,
                               uint32_t sample_cnt,
                               uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                               uint32_t src_gath) // source gather setting (default 0, NO SG support)
{
    return apc_channel_write_internal (dch,
                                       APC_DCH_BMP_STEREO,
                                       sample_data,
                                       sample_cnt,
                                       buf_offset,
                                       src_gath);
}


// read channel data based on DMA Linked List Pointer (multiple buffers)
static int32_t
apc_channel_read_LLP_internal (APC_DCH dch,
                            uint8_t lr_bmp,
                            AUDIO_BUFFER_LLI * bufs,
                            uint32_t buf_cnt,
                            uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                            uint32_t dst_scat) // destination scatter setting (default 0, NO SG support)
{
    // Check if parameters are valid, i.e. APC dual_channel is valid
    if (dch < APC_DCH_OUT_MIN || dch > APC_DCH_OUT_MAX || bufs == NULL || buf_cnt == 0)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    lr_bmp &= APC_DCH_BMP_MASK;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint32_t i;

    // 0 = left, 1 = right (use left or right channel for dual_channel)
    uint8_t lr_idx = (lr_bmp & APC_DCH_BMP_LEFT)? 0 : 1;
    if (pdci->mix_target == 1) // mixed @ right channel
        lr_idx = 1;

    // sample_cnt should be even when 16-bit sample
    if (pdci->ch_mode == APC_CHMODE_16BITS) {
        for (i=0; i<buf_cnt; i++) {
            if ((bufs[i].sample_cnt & 0x1) != 0)
                return CSK_DRIVER_ERROR_PARAMETER;
        }
    }

    // for single channel, Only MONO_MODE (separated L/R channel data) is supported
    // for dual channel, Only STEREO_MODE (mixed L/R channel data) is supported
    if ((lr_bmp != APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_STEREO_MODE) ||
        (lr_bmp == APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_SEPARATE_MODE))
        return CSK_DRIVER_ERROR;

    // Only ADC / I2S IN / ECHO interface can be read
    if (pdci->intf_type != APC_INTF_VAD &&
        pdci->intf_type != APC_INTF_ADC &&
        pdci->intf_type != APC_INTF_I2S_IN &&
        pdci->intf_type != APC_INTF_ECHO)
        return CSK_DRIVER_ERROR;

    if (pdci->busy_lr[lr_idx] != 0)
        return CSK_DRIVER_ERROR_BUSY;

    // select some free DMA channel
    uint8_t *dma_ch_p = &pdci->dma_ch_lr[lr_idx];
    uint32_t usr_param = (dch << 8) | lr_bmp;
    *dma_ch_p = dma_channel_select(dma_ch_p,
                                    apc_dma_event,
                                    usr_param,
                                    DMA_CACHE_SYNC_DST);
    if (*dma_ch_p == DMA_CHANNEL_ANY) {
        LOGD("%s: NO free DMA channel!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    uint32_t control, config_low, config_high;
    DMA_LLP ip_pre, ip;

    control = DMA_CH_CTLL_DST_WIDTH(pdci->dma_width_bits) | DMA_CH_CTLL_SRC_WIDTH(pdci->dma_width_bits) |
            DMA_CH_CTLL_DST_BSIZE(pdci->dma_bsize_bits) | DMA_CH_CTLL_SRC_BSIZE(pdci->dma_bsize_bits) |
            DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_TTFC_P2M |
            DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
    config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(pdci->fixed_lr[lr_idx]->dma_hsid);
//    config_high = DMA_CH_CFGH_PROTCTL(1) | DMA_CH_CFGH_SRC_PER(pdci->fixed_lr[lr_idx]->dma_hsid); // FIXME:

    uint32_t src_addr = pdci->fixed_lr[lr_idx]->data_addr;
    if (pdci->dma_width_bits == DMA_WIDTH_HALFWORD) // trim low 16bits
        src_addr += 2; // high 16bits address

    ip_pre = NULL;
    for (i=0; i<buf_cnt; i++) {
        ip = &bufs[i].dma_lli;
        memset(ip, 0, sizeof(DMA_LLI));
        ip->SAR = src_addr; //pdci->fixed_lr[lr_idx]->data_addr;
        ip->DAR = (uint32_t)(bufs[i].sample_data) + buf_offset;
        if (ip_pre != NULL)
            ip_pre->LLP = (uint32_t)ip;
        ip->CTL_LO = control;
        ip->u.SIZE = (pdci->samp_bits > 16 ? bufs[i].sample_cnt : bufs[i].sample_cnt/2);
        ip_pre = ip; // save current DMA_LLI pointer
    }
    // DMA_LLI list (stored in AUDIO_BUFFER_LLI array) is to be accessed by DMAC,
    // So the AUDIO_BUFFER_LLI array memory space SHOULD be do cache sync operation
    dcache_flush_range((unsigned long)bufs, (unsigned long)(bufs + buf_cnt));

    // configure DMA channel
    int32_t stat = dma_channel_configure_LLP (*dma_ch_p, &bufs->dma_lli, config_low, config_high, 0, dst_scat);
    if (stat == -1) {
        LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
        dma_channel_disable(*dma_ch_p, false);
        *dma_ch_p = DMA_CHANNEL_ANY;
        return CSK_DRIVER_ERROR;
    }

    pdci->busy_lr[lr_idx] = 1;
    return CSK_DRIVER_OK;
}


int32_t apc_channel_read_LLP(APC_CH ch,
                            AUDIO_BUFFER_LLI * bufs,
                            uint32_t buf_cnt,
                            uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                            uint32_t dst_scat) // destination scatter setting (default 0, NO SG support)
{
    return apc_channel_read_LLP_internal (APC_CH_TO_DCH(ch),
                                          APC_CH_LR_BMP(ch),
                                          bufs,
                                          buf_cnt,
                                          buf_offset,
                                          dst_scat);
}


int32_t apc_dual_channel_read_LLP (APC_DCH dch,
                                AUDIO_BUFFER_LLI * bufs,
                                uint32_t buf_cnt,
                                uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                                uint32_t dst_scat) // destination scatter setting (default 0, NO SG support)
{
    return apc_channel_read_LLP_internal (dch,
                                          APC_DCH_BMP_STEREO,
                                          bufs,
                                          buf_cnt,
                                          buf_offset,
                                          dst_scat);
}


// write channel data based on DMA Linked List Pointer (multiple buffers)
static int32_t
apc_channel_write_LLP_internal (APC_DCH dch,
                              uint8_t lr_bmp,
                              AUDIO_BUFFER_LLI * bufs,
                              uint32_t buf_cnt,
                              uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                              uint32_t src_gath) // source gather setting (default 0, NO SG support)
{
    // Check if parameters are valid, i.e. APC dual_channel is valid
    if (dch < APC_DCH_OUT_MIN || dch > APC_DCH_OUT_MAX || bufs == NULL || buf_cnt == 0)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    lr_bmp &= APC_DCH_BMP_MASK;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint32_t i;
    // 0 = left, 1 = right (use left or right channel for dual_channel)
    uint8_t lr_idx = (lr_bmp & APC_DCH_BMP_LEFT)? 0 : 1;
    if (pdci->mix_target == 1) // mixed @ right channel
        lr_idx = 1;

    // sample_cnt should be even when 16-bit sample
    if (pdci->ch_mode == APC_CHMODE_16BITS) {
        for (i=0; i<buf_cnt; i++) {
            if ((bufs[i].sample_cnt & 0x1) != 0)
                return CSK_DRIVER_ERROR_PARAMETER;
        }
    }

    // for single channel, Only MONO_MODE (separated L/R channel data) is supported
    // for dual channel, Only STEREO_MODE (mixed L/R channel data) is supported
    if ((lr_bmp != APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_MIXED_MODE) ||
        (lr_bmp == APC_DCH_BMP_STEREO && pdci->mix_mode == APC_DCH_SEPARATE_MODE))
        return CSK_DRIVER_ERROR;

    // Only DAC / I2S OUT interface can be written
    if (pdci->intf_type != APC_INTF_DAC &&
        pdci->intf_type != APC_INTF_I2S_OUT)
        return CSK_DRIVER_ERROR;

    if (pdci->busy_lr[lr_idx] != 0)
        return CSK_DRIVER_ERROR_BUSY;

    // select some free DMA channel
    uint8_t *dma_ch_p = &pdci->dma_ch_lr[lr_idx];
    uint32_t usr_param = (dch << 8) | lr_bmp;
    *dma_ch_p = dma_channel_select(dma_ch_p,
                                    apc_dma_event,
                                    usr_param,
                                    DMA_CACHE_SYNC_DST);
    if (*dma_ch_p == DMA_CHANNEL_ANY) {
        LOGD("%s: NO free DMA channel!!\r\n", __func__);
        return CSK_DRIVER_ERROR;
    }

    uint32_t control, config_low, config_high;
    DMA_LLP ip_pre, ip;

    control = DMA_CH_CTLL_DST_WIDTH(pdci->dma_width_bits) | DMA_CH_CTLL_SRC_WIDTH(pdci->dma_width_bits) |
            DMA_CH_CTLL_DST_BSIZE(pdci->dma_bsize_bits) | DMA_CH_CTLL_SRC_BSIZE(pdci->dma_bsize_bits) |
            DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |
            DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN;
    config_low = DMA_CH_CFGL_CH_PRIOR(1); // channel priority is higher than 0
    config_high = DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(pdci->fixed_lr[lr_idx]->dma_hsid);
//    config_high = DMA_CH_CFGH_PROTCTL(1) | DMA_CH_CFGH_DST_PER(pdci->fixed_lr[lr_idx]->dma_hsid);   // FIXME:

    uint32_t dst_addr = pdci->fixed_lr[lr_idx]->data_addr;
    if (pdci->dma_width_bits == DMA_WIDTH_HALFWORD) // trim low 16bits
        dst_addr += 2; // high 16bits address

    ip_pre = NULL;
    for (i=0; i<buf_cnt; i++) {
        ip = &bufs[i].dma_lli;
        memset(ip, 0, sizeof(DMA_LLI));
        ip->SAR = (uint32_t)(bufs[i].sample_data) + buf_offset;
        ip->DAR = dst_addr; //pdci->fixed_lr[lr_idx]->data_addr;
        if (ip_pre != NULL)
            ip_pre->LLP = (uint32_t)ip;
        ip->CTL_LO = control;
        ip->u.SIZE = (pdci->samp_bits > 16 ? bufs[i].sample_cnt : bufs[i].sample_cnt/2);
        ip_pre = ip; // save current DMA_LLI pointer
    }
    // DMA_LLI list (stored in AUDIO_BUFFER_LLI array) is to be accessed by DMAC,
    // So the AUDIO_BUFFER_LLI array memory space SHOULD be do cache sync operation
    dcache_flush_range((unsigned long)bufs, (unsigned long)(bufs + buf_cnt));

    // configure DMA channel
    int32_t stat = dma_channel_configure_LLP (*dma_ch_p, &bufs->dma_lli, config_low, config_high, src_gath, 0);
    if (stat == -1) {
        LOGD("%s: Failed to call dma_channel_configure!!\r\n", __func__);
        dma_channel_disable(*dma_ch_p, false);
        *dma_ch_p = DMA_CHANNEL_ANY;
        return CSK_DRIVER_ERROR;
    }

    pdci->busy_lr[lr_idx] = 1;
    return CSK_DRIVER_OK;
}


// write channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_channel_write_LLP (APC_CH ch,
                              AUDIO_BUFFER_LLI * bufs,
                              uint32_t buf_cnt,
                              uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                              uint32_t src_gath) // source gather setting (default 0, NO SG support)
{
    return apc_channel_write_LLP_internal (APC_CH_TO_DCH(ch),
                                            APC_CH_LR_BMP(ch),
                                            bufs,
                                            buf_cnt,
                                            buf_offset,
                                            src_gath);
}


// write channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_dual_channel_write_LLP (APC_DCH dch,
                                  AUDIO_BUFFER_LLI * bufs,
                                  uint32_t buf_cnt,
                                  uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                                  uint32_t src_gath) // source gather setting (default 0, NO SG support)
{
    return apc_channel_write_LLP_internal (dch,
                                            APC_DCH_BMP_STEREO,
                                            bufs,
                                            buf_cnt,
                                            buf_offset,
                                            src_gath);
}

int32_t apc_channel_enable (APC_CH ch) // enable APC channel only
{
    APC_DCH dch = APC_CH_TO_DCH(ch);
    uint8_t lr_idx = APC_CH_LR_IDX(ch);

    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    uint32_t cfg_val, intr_bits = 0;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if ((pdci->ch_sel & APC_CH_LR_BMP(ch)) == 0) {
        LOGD("%s: APC channel %d is not selected!", __func__, ch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (pdci->fixed_lr[lr_idx]->ch_dir) // IN (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
        intr_bits = APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
    else // OUT (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;
        intr_bits = APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;

    cfg_val = pdci->cfg_reg_p->all;
    if (lr_idx == 0) { // Left channel
        if ((cfg_val & APC_DCH_L_EN) == APC_DCH_L_EN) {
            //LOGD("%s: Left channel already enabled!\n", __func__);
            return CSK_DRIVER_OK;
        }
        //pdci->cfg_reg_p->bit.CH_L_EN = 1;
        pdci->cfg_reg_p->all = cfg_val | APC_DCH_L_EN;
    } else { // Right channel
        if ((cfg_val & APC_DCH_R_EN) == APC_DCH_R_EN) {
            //LOGD("%s: Right channel already enabled!\n", __func__);
            return CSK_DRIVER_OK;
        }
        //pdci->cfg_reg_p->bit.CH_R_EN = 1;
        pdci->cfg_reg_p->all = cfg_val | APC_DCH_R_EN;
    }

    // clear all interrupts' status
    apc_ch_intr_clear(ch, APC_FIFO_INTR_MASK);
    // enable all interrupts of APC channel
    apc_ch_intr_enable(ch, intr_bits);

    return CSK_DRIVER_OK;
}

int32_t apc_channel_disable (APC_CH ch) // disable APC channel only
{
    APC_DCH dch = APC_CH_TO_DCH(ch);
    uint8_t lr_idx = APC_CH_LR_IDX(ch);

    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if ((pdci->ch_sel & APC_CH_LR_BMP(ch)) == 0) {
        LOGD("%s: APC channel %d is not selected!", __func__, ch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // disable all interrupts of APC channel
    apc_ch_intr_disable(ch, APC_FIFO_INTR_MASK);

    if (lr_idx == 0) { // Left channel
        pdci->cfg_reg_p->bit.CH_L_EN = 0;
    } else { // Right channel
        pdci->cfg_reg_p->bit.CH_R_EN = 0;
    }
    return CSK_DRIVER_OK;
}

int32_t apc_channel_abort (APC_CH ch) // abort APC channel data transfer only
{
    int32_t ret = CSK_DRIVER_OK;
    APC_DCH dch = APC_CH_TO_DCH(ch);
    uint8_t lr_idx = APC_CH_LR_IDX(ch);

    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if ((pdci->ch_sel & APC_CH_LR_BMP(ch)) == 0) {
        LOGD("%s: APC channel %d is not selected!", __func__, ch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (pdci->busy_lr[lr_idx] != 0) {
        // abort and wait until done
        ret = dma_channel_disable(pdci->dma_ch_lr[lr_idx], true);
        pdci->busy_lr[lr_idx] = 0;

        // disable all interrupts of APC channel
        apc_ch_intr_disable(ch, APC_FIFO_INTR_MASK);

        if (lr_idx == 0) { // Left channel
            pdci->cfg_reg_p->bit.CH_L_EN = 0;
        } else { // Right channel
            pdci->cfg_reg_p->bit.CH_R_EN = 0;
        }
    } // channel busy

    return ret;
}


#if 0  // unmask desired interrupts => enable APC channel

int32_t apc_dual_channel_enable (APC_DCH dch) // enable APC dual_channel only
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    uint32_t intr_bits = 0;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint8_t ch_sel = pdci->ch_sel & APC_DCH_BMP_MASK;

    if (ch_sel == 0) {
        LOGD("%s: neither of APC dual channels (dch=%d) is not selected!",
                __func__, dch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (pdci->fixed_lr[0]->ch_dir) // IN (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
        intr_bits = APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
    else // OUT (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;
        intr_bits = APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;

    if (ch_sel == APC_DCH_BMP_STEREO) { // both channels
        if (pdci->mix_mode == 0) // mono, separated
            apc_dch_intr_enable(dch, intr_bits); // enable interrupts of dual_channel
        else if (pdci->mix_target == 0) // stereo, mixed (data via LEFT channel by default)
            apc_ch_intr_enable(APC_DCH_TO_CH(dch, 0), intr_bits); // enable interrupts of LEFT channel
        else
            apc_ch_intr_enable(APC_DCH_TO_CH(dch, 1), intr_bits); // enable interrupts of RIGHT channel
        pdci->cfg_reg_p->all |= APC_DCH_R_EN | APC_DCH_L_EN; // enable dual_channel
    } else if (ch_sel == APC_DCH_BMP_LEFT) { // Left channel
        apc_ch_intr_enable(APC_DCH_TO_CH(dch, 0), intr_bits); // enable interrupts
        pdci->cfg_reg_p->bit.CH_L_EN = 1;
    } else if (ch_sel == APC_DCH_BMP_RIGHT) { // Right channel
        apc_ch_intr_enable(APC_DCH_TO_CH(dch, 1), intr_bits); // enable interrupts
        pdci->cfg_reg_p->bit.CH_R_EN = 1;
    }
    return CSK_DRIVER_OK;
}

#else   // enable APC channel => clear interrupts => unmask desired interrupts

int32_t apc_dual_channel_enable (APC_DCH dch) // enable APC dual_channel only
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    uint32_t cfg_val, intr_bits = 0;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint8_t ch_sel = pdci->ch_sel & APC_DCH_BMP_MASK;
    APC_CH ch;

    if (ch_sel == 0) {
        LOGD("%s: neither of APC dual channels (dch=%d) is not selected!",
                __func__, dch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (pdci->fixed_lr[0]->ch_dir) // IN (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
        intr_bits = APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_FULL;
    else // OUT (left and right are same)
        //intr_bits = APC_INTR_READY_TO_XFER | APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;
        intr_bits = APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_EMPTY;

    cfg_val = pdci->cfg_reg_p->all;
    if (ch_sel == APC_DCH_BMP_STEREO) { // both channels
        uint32_t val = APC_DCH_R_EN | APC_DCH_L_EN;
        if ((cfg_val & val) == val) {
            //LOGD("%s: L/R channel already enabled!\n", __func__);
            return CSK_DRIVER_OK;
        }
        //pdci->cfg_reg_p->all |= APC_DCH_R_EN | APC_DCH_L_EN; // enable dual_channel
        pdci->cfg_reg_p->all = cfg_val | APC_DCH_R_EN | APC_DCH_L_EN;

        //TODO: wait until there's at least one data in the TX FIFO ?...
        apc_dch_intr_clear(dch, APC_FIFO_INTR_MASK); // Clear all interrupts' status
        if (pdci->mix_mode == 0) // mono, separated
            apc_dch_intr_enable(dch, intr_bits); // enable interrupts of dual_channel
        else if (pdci->mix_target == 0) // stereo, mixed (data via LEFT channel by default)
            apc_ch_intr_enable(APC_DCH_TO_CH(dch, 0), intr_bits); // enable interrupts of LEFT channel
        else
            apc_ch_intr_enable(APC_DCH_TO_CH(dch, 1), intr_bits); // enable interrupts of RIGHT channel

    } else if (ch_sel == APC_DCH_BMP_LEFT) { // Left channel
        if ((cfg_val & APC_DCH_L_EN) == APC_DCH_L_EN) {
            //LOGD("%s: Left channel already enabled!\n", __func__);
            return CSK_DRIVER_OK;
        }
        //pdci->cfg_reg_p->bit.CH_L_EN = 1;
        pdci->cfg_reg_p->all = cfg_val | APC_DCH_L_EN;

        //TODO: wait until there's at least one data in the TX FIFO ?...
        ch = APC_DCH_TO_CH(dch, 0);
        apc_ch_intr_clear(ch, APC_FIFO_INTR_MASK); // Clear all interrupts' status
        apc_ch_intr_enable(ch, intr_bits); // enable interrupts

    } else if (ch_sel == APC_DCH_BMP_RIGHT) { // Right channel
        if ((cfg_val & APC_DCH_R_EN) == APC_DCH_R_EN) {
            //LOGD("%s: Right channel already enabled!\n", __func__);
            return CSK_DRIVER_OK;
        }
        //pdci->cfg_reg_p->bit.CH_R_EN = 1;
        pdci->cfg_reg_p->all = cfg_val | APC_DCH_R_EN;

        //TODO: wait until there's at least one data in the TX FIFO ?...
        ch = APC_DCH_TO_CH(dch, 1);
        apc_ch_intr_clear(ch, APC_FIFO_INTR_MASK); // Clear all interrupts' status
        apc_ch_intr_enable(ch, intr_bits); // enable interrupts
    }

    return CSK_DRIVER_OK;
}

#endif


int32_t apc_dual_channel_disable (APC_DCH dch) // disable APC dual_channel only
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint8_t ch_sel = pdci->ch_sel & APC_DCH_BMP_MASK;

    if (ch_sel == 0) {
        LOGD("%s: neither of APC dual channels (dch=%d) is not selected!",
                __func__, dch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (ch_sel == APC_DCH_BMP_STEREO) { // both channels
        apc_dch_intr_disable(dch, APC_FIFO_INTR_MASK); // disable all interrupts
        pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN);
        apc_dch_intr_clear(dch, APC_FIFO_INTR_MASK); // clear all interrupts
    } else if (ch_sel == APC_DCH_BMP_LEFT) { // Left channel
        apc_ch_intr_disable(APC_DCH_TO_CH(dch, 0), APC_FIFO_INTR_MASK); // disable all interrupts
        pdci->cfg_reg_p->bit.CH_L_EN = 0;
        apc_ch_intr_clear(APC_DCH_TO_CH(dch, 0), APC_FIFO_INTR_MASK); // clear all interrupts
    } else if (ch_sel == APC_DCH_BMP_RIGHT) { // Right channel
        apc_ch_intr_disable(APC_DCH_TO_CH(dch, 1), APC_FIFO_INTR_MASK); // disable all interrupts
        pdci->cfg_reg_p->bit.CH_R_EN = 0;
        apc_ch_intr_clear(APC_DCH_TO_CH(dch, 1), APC_FIFO_INTR_MASK); // clear all interrupts
    }
    return CSK_DRIVER_OK;
}

int32_t apc_dual_channel_abort (APC_DCH dch)
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    if ((pdci->ch_sel & APC_DCH_BMP_MASK) == 0) {
        LOGD("%s: neither of APC L/R channels (dch=%d) is not selected!",
                __func__, dch);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    int32_t ret0, ret = CSK_DRIVER_OK;

    if (pdci->busy_lr[0] != 0 || pdci->busy_lr[1] != 0) {
        if (pdci->busy_lr[0] != 0) { // left channel
            // abort and wait until done
            ret0 = dma_channel_disable(pdci->dma_ch_lr[0], true);
            pdci->busy_lr[0] = 0;
            if (ret0 != CSK_DRIVER_OK)
                ret = ret0;
        }
        if (pdci->busy_lr[1] != 0) { // right channel
            // abort and wait until done
            ret0 = dma_channel_disable(pdci->dma_ch_lr[1], true);
            pdci->busy_lr[1] = 0;
            if (ret0 != CSK_DRIVER_OK)
                ret = ret0;
        }

        // disable both APC channels and their all interrupts
        apc_dch_intr_disable(dch, APC_FIFO_INTR_MASK);
        pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN);
        apc_dch_intr_clear(dch, APC_FIFO_INTR_MASK);

    } // Left or Right busy

    return ret;
}


/**
  \fn          uint32_t apc_channel_get_count (APC_CH ch)
  \brief       Get number of transferred data items
  \param[in]   ch Channel number
  \returns     Number of transferred data items
*/
uint32_t apc_channel_get_count (APC_CH ch)
{
    // Check if parameters are valid, i.e. APC channel is valid
    if (ch > APC_CH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    APC_DCH dch = APC_CH_TO_DCH(ch); // dual_channel
    uint8_t lr_idx = APC_CH_LR_IDX(ch); // 0 = left, 1 = right

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];
    uint32_t count;
    if (pdci->busy_lr[lr_idx] != 0) {
        count = dma_channel_get_count(pdci->dma_ch_lr[lr_idx]);
        if (pdci->samp_bits == 16)
            count <<= 1; // WORD count => HALFWORD count
    } else {
        count = pdci->samps_lr[lr_idx];
    }

    return count;
}

uint32_t apc_dual_channel_get_count (APC_DCH dch)
{
    // Check if parameters are valid, i.e. APC dual_channel is valid
    if (dch > APC_DCH_COUNT)
        return CSK_DRIVER_ERROR_PARAMETER;

    // Check if APC and specified dual_channel are both initialized
    if (!IS_APC_INITIALIZED() || !IS_APC_DCH_INITIALIZED(dch))
        return CSK_DRIVER_ERROR;

    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];

    // Only STEREO_MODE (mixed L/R channel data) is supported for dual_channel
    if (pdci->mix_mode != APC_DCH_MIXED_MODE) {
        LOGD("%s: NOT mixed mode, CANNOT get count of dual_channel!\r\n", __func__);
        return 0;
    }

    // mixed @ left or right channel
    uint8_t lr_idx = pdci->mix_target == 1 ? 1 : 0;
    uint32_t count;

    if (pdci->busy_lr[lr_idx] != 0) { // via left or right channel
        count = dma_channel_get_count(pdci->dma_ch_lr[lr_idx]);
        if (pdci->samp_bits == 16)
            count <<= 1; // WORD count => HALFWORD count
    } else {
        return pdci->samps_lr[lr_idx];
    }

    return count;
}


#define EQ_COEF_MAX_COUNT       50
#define EQ_STAGE_MAX_COUNT      10
int32_t apc_eq_set_coef_array(APC_DCH dch, uint32_t *eqcoefs, uint32_t num)
{
//   if (dch != APC_DCH_OUT1) {
//       CLOGW("%s: ONLY APC_TX_CH1 supports EQ function on VENUS6001!\n", __func__);
//       return CSK_DRIVER_ERROR_PARAMETER;
//   }

    assert(dch == APC_DCH_OUT1);
    if (num == 0 || num > EQ_COEF_MAX_COUNT) {
        CLOGW("%s: at most %d (cur=%d) EQ coefficients are supported!\n",
                __func__, EQ_COEF_MAX_COUNT, num);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    uint32_t i;
    uint32_t eqcoef_addr = APC_TX_CH1_EQCOEF_BASE;

    for (i = 0; i < EQ_COEF_MAX_COUNT; i++, eqcoef_addr += 4) {
        outw(eqcoef_addr, eqcoefs[i] & 0xFFFFFF);
    } // end for i

    return CSK_DRIVER_OK;
}

int32_t apc_eq_set_coef(APC_DCH dch, uint32_t index, uint32_t eqcoef)
{
//    if (dch != APC_DCH_OUT1) {
//        CLOGW("%s: ONLY APC_TX_CH1 supports EQ function on VENUS6001!\n", __func__);
//        return CSK_DRIVER_ERROR_PARAMETER;
//    }

    assert(dch == APC_DCH_OUT1);
    if (index >= EQ_COEF_MAX_COUNT) {
        CLOGW("%s: at most %d (index=%d) EQ coefficients are supported!\n",
                __func__, EQ_COEF_MAX_COUNT, index);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    outw(APC_TX_CH1_EQCOEF_BASE + (index << 2), eqcoef);
    return CSK_DRIVER_OK;
}

int32_t apc_eq_enable(APC_DCH dch, uint32_t stages)
{
//    if (dch != APC_DCH_OUT1) {
//        CLOGW("%s: ONLY APC_TX_CH1 supports EQ function on VENUS6001!\n", __func__);
//        return CSK_DRIVER_ERROR_PARAMETER;
//    }

    assert(dch == APC_DCH_OUT1);
    if (stages == 0 || stages > EQ_STAGE_MAX_COUNT) {
        CLOGW("%s: at most %d (cur=%d) EQ stages are supported!\n",
                __func__, EQ_STAGE_MAX_COUNT, stages);
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    union APC_REG_APC_TX_CH1_CFG * tx_ch1_cfg;
    tx_ch1_cfg = (union APC_REG_APC_TX_CH1_CFG *)(APC_BASE + OFFSET_APC_APC_TX_CH1_CFG);
    tx_ch1_cfg->bit.EQ_STAGE = stages;
    tx_ch1_cfg->bit.EQ_BYPASS_REG = 0;
    tx_ch1_cfg->bit.EQ_CH_EN = 0x3; // enable both LEFT & RIGHT channel
    return CSK_DRIVER_OK;
}

int32_t apc_eq_disble(APC_DCH dch)
{
//    if (dch != APC_DCH_OUT1) {
//        CLOGW("%s: ONLY APC_TX_CH1 supports EQ function on VENUS6001!\n", __func__);
//        return CSK_DRIVER_ERROR_PARAMETER;
//    }

    assert(dch == APC_DCH_OUT1);
    union APC_REG_APC_TX_CH1_CFG * tx_ch1_cfg;
    tx_ch1_cfg = (union APC_REG_APC_TX_CH1_CFG *)(APC_BASE + OFFSET_APC_APC_TX_CH1_CFG);
    tx_ch1_cfg->bit.EQ_BYPASS_REG = 1;
    tx_ch1_cfg->bit.EQ_CH_EN = 0x0; // disable both LEFT & RIGHT channel
    return CSK_DRIVER_OK;
}

int32_t apc_eq_clear(APC_DCH dch, uint8_t wait_done)
{
//    if (dch != APC_DCH_OUT1) {
//        CLOGW("%s: ONLY APC_TX_CH1 supports EQ function on VENUS6001!\n", __func__);
//        return CSK_DRIVER_ERROR_PARAMETER;
//    }

    assert(dch == APC_DCH_OUT1);
    union APC_REG_APC_TX_CH1_CFG * tx_ch1_cfg;
    tx_ch1_cfg = (union APC_REG_APC_TX_CH1_CFG *)(APC_BASE + OFFSET_APC_APC_TX_CH1_CFG);
    tx_ch1_cfg->bit.EQ_CLR = 1;
    if (wait_done) {
        while (tx_ch1_cfg->bit.EQ_CLR_DONE == 0);
    }
    return CSK_DRIVER_OK;
}


static _FAST_FUNC_RO void
apc_dma_event(uint32_t event_info, uint32_t xfer_bytes, uint32_t usr_param)
{
    uint8_t event_type = event_info & 0xFF;
    APC_DCH dch = (usr_param >> 8) & 0xFF;
    APC_DCH_INFO *pdci = &g_apc_dev.dch_array[dch];

    // use Left channel if mixed L/R channel data
    uint8_t lr_idx = ((usr_param & APC_DCH_BMP_MASK) & APC_DCH_BMP_LEFT) ? 0 : 1;
    if (pdci->mix_target == 1) // mixed @ right channel
        lr_idx = 1;

    APC_CH ch = APC_DCH_TO_CH(dch, lr_idx);
    uint32_t tmp;

    switch (event_type) {
    case DMA_EVENT_TRANSFER_COMPLETE:
        //FIXME: is it OK to replace dma_channel_get_count() call with xfer_bytes?
        assert(pdci->dma_ch_lr[lr_idx] < DMA_NUMBER_OF_CHANNELS);
        tmp = dma_channel_get_count(pdci->dma_ch_lr[lr_idx]);
        if (pdci->samp_bits == 16)
            tmp <<= 1; // WORD count => HALFWORD count
        pdci->samps_lr[lr_idx] = tmp;
        pdci->busy_lr[lr_idx] = 0;

        // disable  APC channel and its interrupts
        //apc_ch_intr_disable(ch, APC_FIFO_INTR_MASK);
        //if (lr_idx == 0) { // Left channel
        //    pdci->cfg_reg_p->bit.CH_L_EN = 0;
        //} else { // Right channel
        //    pdci->cfg_reg_p->bit.CH_R_EN = 0;
        //}

        if (pdci->cb_event != NULL) {
            tmp = (ch << 8) | APC_EVENT_TRANSFER_COMPLETE;
            pdci->cb_event(tmp, pdci->usr_param);
        }

        break;

    case DMA_EVENT_ERROR:
        //TODO: do something here?
        pdci->busy_lr[lr_idx] = 0;
        if (pdci->cb_event != NULL) {
            tmp = (APC_DCH_TO_CH(dch, lr_idx) << 8) | APC_EVENT_DMA_ERROR;
            pdci->cb_event(tmp, pdci->usr_param);
        }
        break;

    default:
        break;
    }
}


/**
  \fn          void apc_irq_handler (void)
  \brief       APC interrupt handler
*/
static _FAST_FUNC_RO void apc_irq_handler (void)
{
    //TODO: Read interrupt status register, and check if
    // there are following interrupt conditions:
    // 1) FIFO is ready to transfer (TX below threshold or RX above threshold)
    // 2) TX FIFO Empty
    // 3) RX FIFO Full
    // 4) TX FIFO Underflow (no data in FIFO, and send is ongoing)
    // 5) RX FIFO Overflow (FIFO is full, and recv is ongoing)

    uint32_t i, status, event_info;
    uint32_t rx_status, tx_status, rx_status_raw, tx_status_raw;
    APC_DCH_INFO *pdci;
    uint8_t lr_idx, mixed;

    rx_status_raw = APC_RX_INT_RAW_STATUS();
    tx_status_raw = APC_TX_INT_RAW_STATUS();
    rx_status = APC_RX_INT_STATUS();
    tx_status = APC_TX_INT_STATUS();
    for (i = 0; i < APC_CH_COUNT; i++) {
        status = APC_CH_INT_STATUS(rx_status, tx_status, i);
        if (status == 0)    continue;

        pdci = &g_apc_dev.dch_array[APC_CH_TO_DCH(i)];
        mixed = pdci->mix_mode;
        lr_idx = APC_CH_LR_IDX(i);
        //TODO: do the following only if PIO read/write APC channel...
        //if ( IS_SET_READY_TO_XFER (status, i) ) {
        //    //TODO:
        //}

        // RX FIFO full
        if ( IS_SET_RX_FULL (status) ) {
            apc_ch_intr_disable(i, APC_INTR_FIFO_FULL);
            CLOGW("APC CH %d FULL! mask INT\n", i);
            //CLOGW("APC CH %d Fu!\n", i);

            if (pdci->cb_event != NULL) {
                event_info = (i << 8) | APC_EVENT_RX_FIFO_FULL;
                pdci->cb_event(event_info, pdci->usr_param);
            }
        }

        // RX FIFO overflow
        if ( IS_SET_RX_OVERRUN (status) ) {
            if (mixed) { // both Left & Right channels
                pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN);
            } else if (lr_idx == 0) { // Left channel
                pdci->cfg_reg_p->bit.CH_L_EN = 0;
            } else { // Right channel
                pdci->cfg_reg_p->bit.CH_R_EN = 0;
            }
            apc_ch_intr_disable(i, APC_INTR_FIFO_OVERRUN);
            apc_ch_intr_clear(i, APC_INTR_FIFO_OVERRUN | APC_INTR_FIFO_DMA_REQ);
            CLOGW("APC CH %d OVERRUN! disable CH(mixed=%d)\n", i, mixed);
            //CLOGW("APC CH %d OV!\n", i);

            if (pdci->cb_event != NULL) {
                event_info = (i << 8) | APC_EVENT_RX_FIFO_OVERRUN;
                pdci->cb_event(event_info, pdci->usr_param);
            }
        }

        // TX FIFO empty
        if ( IS_SET_TX_EMPTY (status) ) {
            apc_ch_intr_disable(i, APC_INTR_FIFO_EMPTY);
            CLOGW("APC CH %d EMPTY! mask INT\n", i);
            //CLOGW("APC CH %d Em!\n",i);

            if (pdci->cb_event != NULL) {
                event_info = (i << 8) | APC_EVENT_TX_FIFO_EMPTY;
                pdci->cb_event(event_info, pdci->usr_param);
            }
        }

        // TX FIFO underflow
        if ( IS_SET_TX_UNDERRUN (status) ) {
            // According to ZhaoRui, for mixed mode, all interrupts occur on the specified channel,
            // that is, no interrupts NEVER occur on the other channel
            if (mixed) { // both Left & Right channels
                pdci->cfg_reg_p->all &= ~(APC_DCH_R_EN | APC_DCH_L_EN);
            } else if (lr_idx == 0) { // Left channel
                pdci->cfg_reg_p->bit.CH_L_EN = 0;
            } else { // Right channel
                pdci->cfg_reg_p->bit.CH_R_EN = 0;
            }
            apc_ch_intr_disable(i, APC_INTR_FIFO_UNDERRUN);
            apc_ch_intr_clear(i, APC_INTR_FIFO_UNDERRUN | APC_INTR_FIFO_DMA_REQ);
            CLOGW("APC CH %d UNDERRUN! disable CH(mixed=%d)\n", i, mixed);
            //CLOGW("APC CH %d UN!\n", i);

            if (pdci->cb_event != NULL) {
                event_info = (i << 8) | APC_EVENT_TX_FIFO_UNDERRUN;
                pdci->cb_event(event_info, pdci->usr_param);
            }
        }

        //TODO:

    } // end for i (each APC channel)

    // Clear specified interrupts
    CSK_APC->REG_APC_INTR_RX_CLR.all = rx_status_raw;
    CSK_APC->REG_APC_INTR_TX_CLR.all = tx_status_raw;
}
