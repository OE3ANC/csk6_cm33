/*
 * apc_venus.h
 *
 *  Created on: Sept. 21, 2020
 *
 */

#ifndef __APC_VENUS_H
#define __APC_VENUS_H

#include "venus_cp.h"
#include "dma_venus.h"
#include "cmn_sysctrl_reg_venus.h" // for sysctrl regigers (clock etc.)
#include "audpll_ctrl_reg_venus.h" // for audpll registers

// APC data interface: ADC, DAC, I2S(IN & OUT), and Echo
typedef enum {
    APC_INTF_UNSET = 0, // default state
    APC_INTF_VAD,       // [IN] VAD
    APC_INTF_ADC,       // [IN] ADC
    APC_INTF_DAC,       // [OUT] DAC
    APC_INTF_I2S_IN,    // [IN] I2S
    APC_INTF_I2S_OUT,   // [OUT] I2S
    APC_INTF_ECHO,      // [IN] Echo (hidden, from output audio)
    APC_INTF_TYPE_COUNT
} APC_INTF_TYPE;

// APC interface index of each interface type
typedef enum {
    APC_INTF_IDX0 = 0,
    APC_INTF_IDX_VAD = APC_INTF_IDX0,
    APC_INTF_IDX_ECHO = APC_INTF_IDX0,
    APC_INTF_IDX_ADC01 = APC_INTF_IDX0,
    APC_INTF_IDX_DAC = APC_INTF_IDX0,
    APC_INTF_IDX_I2S0 = APC_INTF_IDX0,

    APC_INTF_IDX1,
    //APC_INTF_IDX_VAD_COUNT = APC_INTF_IDX1,
    //APC_INTF_IDX_ECHO_COUNT = APC_INTF_IDX1,
    //APC_INTF_IDX_DAC_COUNT = APC_INTF_IDX1,
    APC_INTF_IDX_ADC23 = APC_INTF_IDX1,
    APC_INTF_IDX_I2S1 = APC_INTF_IDX1,

    APC_INTF_IDX2,
    //APC_INTF_IDX_ADC_COUNT = APC_INTF_IDX2,
    APC_INTF_IDX_I2S2 = APC_INTF_IDX2,

    //APC_INTF_IDX_I2S_COUNT
} APC_INTF_INDEX;

// APC dual_channel: 2 IN Dual_FIFOs (IN0 ~ IN1), 1 IN/ECHO Dual_FIFOs (ECHO), 2 OUT Dual_FIFOs (OUT0 ~ OUT1)
typedef enum {
    APC_DCH_IN0 = 0, // [IN] FIFO 0 & 1
    APC_DCH_VAD = APC_DCH_IN0,
    APC_DCH_ADC01 = APC_DCH_IN0,
    APC_DCH_I2S0_IN = APC_DCH_IN0,
    APC_DCH_ADC_MIN = APC_DCH_IN0,
    APC_DCH_I2S_IN_MIN = APC_DCH_IN0,
    APC_DCH_IN_MIN = APC_DCH_IN0,

    APC_DCH_IN1,     // [IN] FIFO 2 & 3
    APC_DCH_ADC23 = APC_DCH_IN1,
    APC_DCH_I2S1_IN = APC_DCH_IN1,
    APC_DCH_ADC_MAX = APC_DCH_IN1,

    APC_DCH_IN2,     // [IN/ECHO] FIFO 4 & 5
    APC_DCH_ECHO = APC_DCH_IN2,
    APC_DCH_I2S2_IN = APC_DCH_IN2,
    APC_DCH_I2S1_IN_ALT = APC_DCH_IN2,
    APC_DCH_ECHO_MIN = APC_DCH_IN2,
    APC_DCH_ECHO_MAX = APC_DCH_IN2,
    APC_DCH_I2S_IN_MAX = APC_DCH_IN2,
    APC_DCH_IN_MAX = APC_DCH_IN2,

    APC_DCH_OUT0,    // [OUT] FIFO 6 & 7
    APC_DCH_I2S1_OUT = APC_DCH_OUT0,
    APC_DCH_I2S_OUT_MIN = APC_DCH_OUT0,
    APC_DCH_OUT_MIN = APC_DCH_OUT0,

    APC_DCH_OUT1,    // [OUT] FIFO 8 & 9
    APC_DCH_I2S2_OUT = APC_DCH_OUT1,
    APC_DCH_DAC01 = APC_DCH_OUT1,
    APC_DCH_I2S_OUT_MAX = APC_DCH_OUT1,
    APC_DCH_DAC_MIN = APC_DCH_OUT1,
    APC_DCH_DAC_MAX = APC_DCH_OUT1,
    APC_DCH_OUT_MAX = APC_DCH_OUT1,

    APC_DCH_COUNT
} APC_DCH; // DCH = Dual CHannel (Left + Right)

// APC channel: 4 IN FIFOs (IN0_L, IN0_R, IN1_L, IN1_R),
//      2 IN/ECHO FIFOs (IN2_L/ECHO_L, IN2_R/ECHO_R),
//      4 OUT FIFOs (OUT0_L, OUT0_R, OUT1_L, OUT1_R)
typedef enum {
    APC_CH_IN0_L = 0, // [IN] FIFO 0
    APC_CH_ADC0 = APC_CH_IN0_L,
    APC_CH_I2S0_IN_L = APC_CH_IN0_L,
    APC_CH_ADC_MIN = APC_CH_IN0_L, // Min. ADC channel
    APC_CH_I2S_IN_MIN = APC_CH_IN0_L, // Min. I2S IN channel
    APC_CH_IN_MIN = APC_CH_IN0_L,

    APC_CH_IN0_R,     // [IN] FIFO 1
    APC_CH_ADC1 = APC_CH_IN0_R,
    APC_CH_I2S0_IN_R = APC_CH_IN0_R,

    APC_CH_IN1_L,     // [IN] FIFO 2
    APC_CH_ADC2 = APC_CH_IN1_L,
    APC_CH_I2S1_IN_L = APC_CH_IN1_L,

    APC_CH_IN1_R,     // [IN] FIFO 3
    APC_CH_ADC3 = APC_CH_IN1_R,
    APC_CH_I2S1_IN_R = APC_CH_IN1_R,
    APC_CH_ADC_MAX = APC_CH_IN1_R, // Max. ADC channel

    APC_CH_IN2_L,     // [IN/ECHO] FIFO 4
    APC_CH_ECHO_L = APC_CH_IN2_L, // src = APC_CH_OUT1_L
    APC_CH_I2S2_IN_L = APC_CH_IN2_L,
    APC_CH_I2S1_IN_L_ALT = APC_CH_IN2_L,
    //APC_CH_ECHO_MIN = APC_CH_IN2_L, // Min. ECHO channel

    APC_CH_IN2_R,     // [IN/ECHO] FIFO 5
    APC_CH_ECHO_R = APC_CH_IN2_R, // src = APC_CH_OUT1_R
    APC_CH_I2S2_IN_R = APC_CH_IN2_R,
    APC_CH_I2S1_IN_R_ALT = APC_CH_IN2_R,
    //APC_CH_ECHO_MAX = APC_CH_IN2_R, // Max. ECHO channel
    APC_CH_I2S_IN_MAX = APC_CH_IN2_R, // Max. I2S IN channel
    APC_CH_IN_MAX = APC_CH_IN2_R,

    APC_CH_OUT0_L,    // [OUT] FIFO 6
    APC_CH_I2S1_OUT_L = APC_CH_OUT0_L,
    APC_CH_I2S_OUT_MIN = APC_CH_OUT0_L, // Min. I2S OUT channel
    APC_CH_OUT_MIN = APC_CH_OUT0_L,

    APC_CH_OUT0_R,    // [OUT] FIFO 7
    APC_CH_I2S1_OUT_R = APC_CH_OUT0_R,

    APC_CH_OUT1_L,    // [OUT] FIFO 8
    APC_CH_DAC0 = APC_CH_OUT1_L,
    APC_CH_I2S2_OUT_L = APC_CH_OUT1_L,
    APC_CH_DAC_MIN = APC_CH_OUT1_L, // Min. DAC channel

    APC_CH_OUT1_R,    // [OUT] FIFO 9
    APC_CH_DAC1 = APC_CH_OUT1_R,
    APC_CH_I2S2_OUT_R = APC_CH_OUT1_R,
    APC_CH_I2S_OUT_MAX = APC_CH_OUT1_R, // Max. I2S OUT channel
    APC_CH_DAC_MAX = APC_CH_OUT1_R, // Max. DAC channel
    APC_CH_OUT_MAX = APC_CH_OUT1_R,

    APC_CH_COUNT
} APC_CH;

// APC Dual_channel count & channel count
#define APC_DCH_IN_COUNT    (APC_DCH_IN_MAX - APC_DCH_IN_MIN + 1)
#define APC_DCH_OUT_COUNT   (APC_DCH_OUT_MAX - APC_DCH_OUT_MIN + 1)
#define APC_CH_IN_COUNT     (APC_CH_IN_MAX - APC_CH_IN_MIN + 1)
#define APC_CH_OUT_COUNT    (APC_CH_OUT_MAX - APC_CH_OUT_MIN + 1)

// APC channel bitmap
#define APC_DCH_BMP_LEFT                (0x1 << 0)
#define APC_DCH_BMP_RIGHT               (0x1 << 1)
#define APC_DCH_BMP_STEREO              (APC_DCH_BMP_LEFT | APC_DCH_BMP_RIGHT)
#define APC_DCH_BMP_MASK                APC_DCH_BMP_STEREO // 0x3

// APC channel <=> APC dual_channel
#define APC_CH_TO_DCH(ch)           (ch >> 1)
#define APC_CH_LR_IDX(ch)           (ch & 0x1)
#define APC_CH_LR_BMP(ch)           ((ch & 0x1) ? APC_DCH_BMP_RIGHT : APC_DCH_BMP_LEFT)
#define APC_DCH_TO_CH(dch, lr_idx)  ((dch << 1) + (lr_idx & 0x1))
#define APC_IS_LEFT_CH(ch)          ((ch & 0x1) ? 0 : 1)
#define APC_IS_RIGHT_CH(ch)         ((ch & 0x1) ? 1 : 0)

// channel mode
//#define APC_CHMODE_16BITS       0 // 16-bit mono mode, {L1, L0} or {R1, R0} or {R0, L0}
//#define APC_CHMODE_24BITS_HIGH  1 // 24-bit mono mode, {L0, 8'd0} or {R0, 8'd0}
//#define APC_CHMODE_32BITS       2 // 32-bit mono mode, L0 or R0
//#define APC_CHMODE_24BITS_LOW   3 // 24-bit mono mode, {8'd0, L0} or {8'd0, R0}
typedef enum {
    APC_CHMODE_16BITS = 0,      // 16-bit mono mode, {L1, L0} or {R1, R0} or {R1, L0}
    APC_CHMODE_24BITS_LOW = 1,  // 24-bit mono mode, {8'd0, L0} or {8'd0, R0}
    APC_CHMODE_32BITS = 2,      // 32-bit mono mode, L0 or R0
    APC_CHMODE_24BITS_HIGH = 3, // 24-bit mono mode, {L0, 8'd0} or {R0, 8'd0}
    APC_CHMODE_COUNT
} APC_CHMODE;

// Audio buffer used in data transfer (read/write) based on DMA LLP
// NOTE: AUDIO_BUFFER_LLI (array) SHOULD NOT in the stack, and SHOULD NOT
//      be released until data transfer is completed or aborted!
// See Also: AUDIO_BUFFER_USER (defined in Driver_Common.h, for driver's caller)
typedef struct {
    uint32_t* sample_data;
    uint32_t  sample_cnt; // SHOULD be EVEN when 16-bit sample!!
    DMA_LLI  dma_lli; // DON'T TOUCH IT! reserved for DMA driver only
} AUDIO_BUFFER_LLI;


// APC Event Type, used by APC callback CSK_APC_SignalEvent_t (see below)
// an APC channel read/write is completed
#define APC_EVENT_TRANSFER_COMPLETE     (0x1UL << 0)
// RX FIFO overflow is found during an APC channel read/write
#define APC_EVENT_RX_FIFO_OVERRUN       (0x1UL << 1)
// TX FIFO underflow is found during an APC channel read/write
#define APC_EVENT_TX_FIFO_UNDERRUN      (0x1UL << 2)
// RX FIFO full is found during an APC channel read/write
#define APC_EVENT_RX_FIFO_FULL          (0x1UL << 3)
// TX FIFO empty is found during an APC channel read/write
#define APC_EVENT_TX_FIFO_EMPTY         (0x1UL << 4)

// a DMA error is found during an APC channel read/write
#define APC_EVENT_DMA_ERROR             (0x1UL << 7)


/**
 \fn          void CSK_APC_SignalEvent_t (uint32_t event, uint32_t usr_param)
 \brief       Signal APC Events.
 \param[in]   event_info APC event and channel information
              bit[7:0] is event type, bit[15:8] is APC channel no.
 \param[in]   usr_param    user parameter specified in read / write operation.
 \return      none
*/
typedef void
(*CSK_APC_SignalEvent_t)(uint32_t event_info, uint32_t usr_param);


/**
  \fn          int32_t apc_initialize (void)
  \brief       Initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_initialize (void);


/**
  \fn          int32_t apc_uninitialize (void)
  \brief       De-initialize Audio Processing Center
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t apc_uninitialize (void);


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
int32_t apc_dual_channel_acquire (APC_DCH    dch,
                           APC_INTF_TYPE    itf_type,
                           APC_INTF_INDEX   itf_idx,
                           CSK_APC_SignalEvent_t    cb_event,
                           uint32_t         usr_param);

// generally called in XXX_Uninitialize()
int32_t apc_dual_channel_release (APC_DCH    dch);

// retrieve interface type, device instance of the type, and channel direction for APC channel
int32_t apc_dual_channel_owner (APC_DCH dch,
                            APC_INTF_TYPE *itf_type_p,
                            APC_INTF_INDEX *itf_idx_p,
                            uint8_t *ch_dir_p); // 0=OUT, 1=IN

// generally called in XXX_Control()
int32_t apc_dual_channel_setup (APC_DCH     dch,
                           APC_CHMODE       ch_mode, // 16, 24 MSB, 32 or 24 LSB?
                           uint8_t          ch_sel, // select APC_LEFT_CHANNEL, APC_RIGHT_CHANNEL, or APC_STEREO_CHANNEL?
                           uint8_t          ch_mix, // read L/R channel as a whole, or read L/R channel respectively
                           uint8_t          trim_16bits); // only valid for 24 MSB and 32 channel mode, when set to 1:
                                                         // record: trim low 16bits, get 16bits audio data
                                                         // playback: 16bits audio data is placed at high 16bits of WORD
// echo setting for TX (only TX2 dual_channel supports echo currently)
int32_t apc_echo_setup (APC_DCH  dch,
                        uint32_t tx_samp_rate,
                        uint32_t echo_samp_rate,
                        uint8_t auto_feed);

// apc_channel_xxxxx functions are used to access single L/R channel only if only one channel
// is enabled or mono (mixed) mode is set for L/R channel.
// apc_dual_channels_xxxxx functions are used to access L/R channels as a whole if
// mono (mixed) mode is set.
// NOTE: For following apc_channel_xxxxx and apc_dual_channels_xxxxx Read/Write API functions,
//      all sample count SHOULD be EVEN for 16-bit sample.
int32_t apc_channel_read (APC_CH    ch,
                          uint32_t  *sample_data,
                          uint32_t  sample_cnt,
                          uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
                          uint32_t  dst_scat); // destination scatter setting (default 0, NO SG support)

int32_t apc_channel_write (APC_CH    ch,
                           const uint32_t* sample_data,
                           uint32_t  sample_cnt,
                           uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                           uint32_t src_gath); // source gather setting (default 0, NO SG support)

// read channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_channel_read_LLP (APC_CH ch,
                            AUDIO_BUFFER_LLI * bufs,
                            uint32_t buf_cnt,
                            uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                            uint32_t dst_scat); // destination scatter setting (default 0, NO SG support)

// write channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_channel_write_LLP (APC_CH ch,
                              AUDIO_BUFFER_LLI * bufs,
                              uint32_t buf_cnt,
                              uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                              uint32_t src_gath); // source gather setting (default 0, NO SG support)

int32_t apc_dual_channel_read (APC_DCH  dch,
                              uint32_t  *sample_data,
                              uint32_t  sample_cnt,
                              uint32_t  buf_offset, // byte offset to each buffer (default 0, NO SG support)
                              uint32_t  dst_scat); // destination scatter setting (default 0, NO SG support)

int32_t apc_dual_channel_write (APC_DCH dch,
                               const uint32_t *sample_data,
                               uint32_t sample_cnt,
                               uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                               uint32_t src_gath); // source gather setting (default 0, NO SG support)

// read channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_dual_channel_read_LLP (APC_DCH dch,
                                AUDIO_BUFFER_LLI * bufs,
                                uint32_t buf_cnt,
                                uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                                uint32_t dst_scat); // destination scatter setting (default 0, NO SG support)

// write channel data based on DMA Linked List Pointer (multiple buffers)
int32_t apc_dual_channel_write_LLP (APC_DCH dch,
                                  AUDIO_BUFFER_LLI * bufs,
                                  uint32_t buf_cnt,
                                  uint32_t buf_offset, // byte offset to each buffer (default 0, NO SG support)
                                  uint32_t src_gath); // source gather setting (default 0, NO SG support)

// read mixed data of APC channels 0 ~ 3 (4 IN channels)
// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
typedef enum {
     QCH_ADC01_ADC23 = 0,
     QCH_ADC01_ECHO,
     QCH_ADC23_ECHO,
     QCH_TYPE_COUNT
} QCH_TYPE;

int32_t apc_read_quad_channels (QCH_TYPE qch_type, uint32_t* sample_data, uint32_t  sample_cnt);

// read mixed data of APC channels 0 ~ 5 (4 IN + 2 ECHO)
// sample_cnt is the count of uint16 or uint32, and SHOULD be EVEN when 16-bit sample!
int32_t apc_read_six_channels (uint32_t* sample_data, uint32_t  sample_cnt);

/**
  \fn          uint32_t apc_channel_get_count (APC_CH ch)
  \brief       Get number of transferred data items
  \param[in]   ch Channel number
  \returns     Number of transferred data items
*/
uint32_t apc_channel_get_count (APC_CH ch);
uint32_t apc_dual_channel_get_count (APC_DCH dch);

int32_t apc_channel_enable (APC_CH ch); // enable APC channel only
int32_t apc_channel_disable (APC_CH ch); // disable APC channel only
int32_t apc_channel_abort (APC_CH ch); // abort APC channel data transfer only

int32_t apc_dual_channel_enable (APC_DCH dch); // enable APC dual_channel only
int32_t apc_dual_channel_disable (APC_DCH dch); // disable APC dual_channel only
int32_t apc_dual_channel_abort (APC_DCH dch); // abort APC dual_channel data transfer only

int32_t apc_eq_set_coef_array(APC_DCH dch, uint32_t *eqcoefs, uint32_t num);
int32_t apc_eq_set_coef(APC_DCH dch, uint32_t index, uint32_t eqcoef);
int32_t apc_eq_enable(APC_DCH dch, uint32_t stages);
int32_t apc_eq_disble(APC_DCH dch);
int32_t apc_eq_clear(APC_DCH dch, uint8_t wait_done);

//------------------------------------------------------------
// adc/dac clock related API
//------------------------------------------------------------

#define SYSCTRL_CFG     ((SYSCFG_RegDef*) CMN_SYSCTRL_BASE)
#define AUDPLL_CFG      ((AUDPLL_CTRL_RegDef*) CMN_AUDPLL_CTRL_BASE)

typedef enum {
    AUDIO_CLK_SRC_XTAL = 0, // from XTAL 24MHz [default]
    AUDIO_CLK_SRC_AUDPLL = 1 // from AUDPLL output
} AUDIO_CLK_SRC;

static inline void adc_clk_enable() {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 1;
}

static inline void adc_clk_disable() {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_ADC = 0;
}

static inline void dac_clk_enable() {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 1;
}

static inline void dac_clk_disable() {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.ENA_CODEC_CLK_DAC = 0;
}

static inline void adc_clk_select(AUDIO_CLK_SRC src) {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC = src & 0x1;
}

static inline void dac_clk_select(AUDIO_CLK_SRC src) {
    SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC = src & 0x1;
}

static inline AUDIO_CLK_SRC adc_clk_src() {
    return (AUDIO_CLK_SRC)SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_ADC;
}

static inline AUDIO_CLK_SRC dac_clk_src() {
    return (AUDIO_CLK_SRC)SYSCTRL_CFG->REG_MISC_CLK_CFG2.bit.SEL_CODEC_CLK_DAC;
}


#define DEF_AUDPLL_VCO_FREQ         1190700000  //Hz, 1.1907GHz
#define DEF_AUDPLL_AUDIO_FREQ       22050000    //Hz, 22.05MHz

//initialize AUDPLL if necessary, and set audio post div
void init_audpll_audio_clk(uint32_t req_freq);


#endif /* __APC_VENUS_H */
