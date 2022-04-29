/*
 * venus_cp.h
 *
 *  Created on: Jul 7, 2020
 *
 */

#ifndef INCLUDE_VENUS_CP_H_
#define INCLUDE_VENUS_CP_H_

#include <stddef.h> // for NULL etc.
#include <stdint.h>

#include <toolchain/xcc.h>

#ifdef XOS_USED
#define XOS_INCLUDE_INTERNAL // for xos_started() etc.
#include "xtensa/xos.h"
#else
#include <xtensa/xtruntime.h> // for XTOS API
#endif

#ifndef UINT_MAX
#define UINT_MAX ((uint32_t)-1) // 0xFFFFFFFF
#endif // UINT_MAX


/*****************************************************************************
 * Header for All peripherals
 ****************************************************************************/
#include <ana_aon_regfile_reg_venus.h>
#include <ana_core_regfile_reg_venus.h>
#include <aon_codec_reg_venus.h>
#include <aon_efuse_ctrl_reg_venus.h>
#include <aon_iomux_reg_venus.h>
#include <aon_pmuctrl_reg_venus.h>
#include <audpll_ctrl_reg_venus.h>
#include <cbutton_reg_venus.h>
#include <cmn_iomux_reg_venus.h>
#include <cmn_mailbox_reg_venus.h>
#include <cmn_sysctrl_reg_venus.h>
#include <crypto_reg_venus.h>
#include <dual_timer_reg_venus.h>
#include <gpio_reg_venus.h>
#include <gpt_reg_venus.h>
#include <i2c_reg_venus.h>
#include <ir_reg_venus.h>
#include <keysense_reg_venus.h>
#include <psram_mc_reg_venus.h>
#include <syspll_ctrl_reg_venus.h>
#include <trng_reg_venus.h>
// #include <uart_reg_venus.h>
#include <dw_uart_reg_venus.h>
#include <wdt_reg_venus.h>
#include <vad_reg_venus.h>


/*****************************************************************************
 * Macros for Register Access
 ****************************************************************************/
#define inw(reg)               (*((volatile unsigned int *) (reg)))
#define outw(reg, data)        ((*((volatile unsigned int *)(reg)))=(unsigned int)(data))
#define inb(reg)               (*((volatile unsigned char *) (reg)))
#define outb(reg, data)        ((*((volatile unsigned char *)(reg)))=(unsigned char)(data))

#define __I                     volatile const  /* 'read only' permissions      */
#define __O                     volatile        /* 'write only' permissions     */
#define __IO                    volatile        /* 'read / write' permissions   */


/************************************************************************************
 * Linker definitions
 ************************************************************************************/

//fast function
#define _FAST_FUNC_RO           __attribute__ ((section (".iram0.text")))
//fast data value initialize
//#define _FAST_DATA_VI           __attribute__ ((section (".data_localmem.data")))
// #define _FAST_DATA_VI           __attribute__ ((section (".dram0.data")))
#define _FAST_DATA_VI           //FIXME:
//fast data zero initialize
//#define _FAST_DATA_ZI           __attribute__ ((section (".data_localmem.bss")))
// #define _FAST_DATA_ZI           __attribute__ ((section (".dram0.bss")))
#define _FAST_DATA_ZI           //FIXME:

#define _DMA                    _FAST_DATA_ZI
#define _DMA_PRAM               __attribute__((aligned(32)))


/************************************************************************************
 * Cache definitions
 ************************************************************************************/
enum cache_t { ICACHE, DCACHE };

static inline unsigned long ICACHE_WAY(enum cache_t cache)
{
    //Pls refer to <xtensa/XtDevTools/install/builds/RI-2020.4-win32/chipsky_hifi4_try3/config.html>
	return 2;
}

static inline unsigned long DCACHE_WAY(enum cache_t cache)
{
    //Pls refer to <xtensa/XtDevTools/install/builds/RI-2020.4-win32/chipsky_hifi4_try3/config.html>
	return 4;
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache)
{
	//Pls refer to <xtensa/XtDevTools/install/builds/RI-2020.4-win32/chipsky_hifi4_try3/config.html>
	return 32;
}


/************************************************************************************
 * Memory map
 ************************************************************************************/

//#define DMAC_BASE               0x40000000 // size=16MB // AP DMA
#define USBC_BASE               0x41000000 // size=16MB
#define SDIO_BASE               0x42000000 // size=16MB
#define UART0_BASE              0x45000000 // size=1MB
#define UART1_BASE              0x45100000 // size=1MB
#define UART2_BASE              0x45200000 // size=1MB
#define IR_BASE                 0x45300000 // size=1MB
#define SPI0_BASE               0x45400000 // size=1MB
#define SPI1_BASE               0x45500000 // size=1MB
#define I2C0_BASE               0x45600000 // size=1MB
#define I2C1_BASE               0x45700000 // size=1MB
#define GPT_BASE                0x45800000 // size=1MB
#define GPIO0_BASE              0x45900000 // size=1MB
#define GPIO1_BASE              0x45A00000 // size=1MB

#define CMN_SYSCTRL_BASE        0x46000000 // size=64KB
#define CMN_ANACORE_BASE        0x46010000 // size=64KB
#define CMN_SYSPLL_CTRL_BASE    0x46020000 // size=64KB
#define CMN_AUDPLL_CTRL_BASE    0x46030000 // size=(1M-3*64KB)
#define CMN_MAILBOX_BASE        0x46100000 // size=1MB
#define CMN_IOMUX_BASE          0x46200000 // size=1MB
#define CMN_UART_BASE           0x46300000 // size=1MB
#define CMN_FLASHC_BASE         0x46400000 // size=1MB

#define AON_CODEC_BASE          0x46E20000 // size=13*64B? AON_CODEC
#define AON_IOMUX_BASE          0x46F00000 // size=1MB  AON_IOMUX

#define SHMEM_BASE              0x48000000 // size=640KB => 768KB
#define APC_BASE                0x49000000 // size=1MB
#define CP_CODEC_BASE           0x49100000 // size=1MB PDM2PCM
#define VAD_REG_BASE            0x49200000 // size=1MB
#define VAD_RAM_BASE            0x49300000 // size=1MB

#define CP_CFG_BASE             0x4A000000 // size=64KB
#define CP_WATCHDOG_BASE        0x4A010000 // size=64KB
#define CP_DUALTIMER_BASE       0x4A020000 // size=64KB
#define PSRAM_CTRL_BASE         0x4A030000 // size=64KB
#define CP_UART_LOG_BASE        0x4A040000 // size=64KB
#define CP_DVP_BASE             0x4A050000 // size=64KB
#define NPU_CFG_BASE            0x4A100000 // size=1MB
#define DMAC_BASE               0x4A200000 // size=1MB // CP DMA


#define DATA_RAM0_BASE          0x5FDE0000 // size=16KB => 128KB
#define DATA_RAM1_BASE          0x5FE00000 // size=512KB => 1MB
#define INST_RAM0_BASE          0x5FF10000 // size=16KB => 128KB
#define PSRAM_BASE              0x60000000 // max_size=128MB
#define FLASH_BASE              0x68000000 // max_size=128MB

//TODO: other CP-owned device base

#define CMN_SYSCTRL_BASE        0x46000000 // size=64KB
#define CMN_ANA_CORE_BASE       0x46010000 // size=64KB
#define CMN_SYSPLL_CTRL_BASE    0x46020000 // size=64KB
#define CMN_AUDPLL_CTRL_BASE    0x46030000 // size=64KB
#define CMN_MAILBOX_BASE        0x46100000 // size=1MB
#define CMN_IOMUX_BASE          0x46200000 // size=1MB
#define CMN_UART3_BASE          0x46300000 // size=1MB
#define CMN_FLASHC_BASE         0x46400000 // size=1MB

#define AON_PMUCTRL_BASE        0x46800000 // size=1MB
#define AON_AON_BASE            0x46810000 // size=1MB
#define AON_KEYSENSE_BASE       0x46900000 // size=1MB
#define AON_CBUTTON_BASE        0x46A00000 // size=1MB
#define AON_IWDT_BASE           0x46B00000 // size=1MB
#define AON_RTC_BASE            0x46C00000 // size=1MB
#define AON_EFUSECTRL_BASE      0x46D00000 // size=1MB
#define AON_VAD_BASE            0x46E00000 // size=1MB
#define AON_IOMUX_BASE          0x46F00000 // size=1MB

#define CMN_PSRAMC_BASE         0x4A030000 // size=1MB


/** @addtogroup Peripheral_declaration
  * @{
  */
//#define IP_DMAC               	((XXX_TypeDef *) DMAC_BASE)
//#define IP_EFUSE               	((XXX_TypeDef *) EFUSE_CTRL_BASE)
#define IP_TIMER               		((DUAL_TIMER_RegDef *) TIMER_BASE)
#define IP_WDT               		((WDT_RegDef *) WDT_BASE)
#define IP_CRYPTO               	((CRYPTO_RegDef *) CRYPTO_BASE)
#define IP_UART0               		((DW_UART_RegDef *) UART0_BASE)
#define IP_UART1             		((DW_UART_RegDef *) UART1_BASE)
#define IP_UART2                 	((DW_UART_RegDef *) UART2_BASE)
#define IP_IR               		((IR_RegDef *) IR_BASE)
#define IP_SPI0               		((CSK_SPI_RegDef *) SPI0_BASE)
#define IP_SPI1                		((CSK_SPI_RegDef *) SPI1_BASE)
#define IP_I2C0                		((I2C_RegDef *) I2C0_BASE)
#define IP_I2C1                		((I2C_RegDef *) I2C1_BASE)
#define IP_GPT                		((GPT_RegDef *) GPT_BASE)
#define IP_GPIOA                	((GPIO_RegDef *) GPIO0_BASE)
#define IP_GPIOB              		((GPIO_RegDef *) GPIO1_BASE)
//#define IP_GPADC              	((XXX_TypeDef *) GPADC_BASE)
#define IP_TRNG             		((TRNG_RegDef*)  TRNG_BASE)

//#define IP_ABB               		((XXX_TypeDef *) ABB_BASE)

#define IP_SYSCTRL               	((SYSCFG_RegDef *) CMN_SYSCTRL_BASE)
#define IP_ANACORE                 	((ANA_CORE_RegDef *) CMN_ANACORE_BASE)
#define IP_SYSPLL               	((SYSPLL_RegDef *) CMN_SYSPLL_CTRL_BASE)
#define IP_AUDPLL               	((AUDPLL_CTRL_RegDef *) CMN_AUDPLL_CTRL_BASE)
#define IP_MAILBOX                	((MAILBOX_RegDef *) CMN_MAILBOX_BASE)
#define IP_CMN_IOMUX               	((CMN_IOMUX_RegDef *) CMN_IOMUX_BASE)
//#define CMN_UART              	((XXX_TypeDef *) CMN_UART_BASE)
//#define FLASHC                	((XXX_TypeDef *) CMN_FLASHC_BASE)

#define IP_PMUCTRL                	((AON_RegDef *) AON_PMUCTRL_BASE)
#define IP_ANA                		((ANA_RegDef *) AON_AON_BASE)
#define IP_KEYSENSE                	((KEYSENSE_RegDef *) AON_KEYSENSE_BASE)
#define IP_CBUTTON                	((CBUTTON_RegDef *) AON_CBUTTON_BASE)
#define IP_IWDT                		((WDT_RegDef *) AON_IWDT_BASE)
#define IP_RTC                		((VENUS_RTC_RegDef *) AON_RTC_BASE)
#define IP_EFUSECTRL               	((EFUSE_RegDef *) AON_EFUSECTRL_BASE)
#define IP_VAD                		((VAD_RegDef *) AON_VAD_BASE)
#define IP_AON_IOMUX              	((AON_IOMUX_RegDef *) AON_IOMUX_BASE)
#define IP_PSRAMC                	((PSRAM_MC_RegDef *) CMN_PSRAMC_BASE)

#define IP_AON_CODEC                ((AON_CODEC_RegDef *) AON_CODEC_BASE)

/**
  * @}
  */


/************************************************************************************
 * IRQ Vector
 ************************************************************************************/

// The interrupts listed below are Level-triggered and their priorities
// are mostly level 1 except NMI and those specified explicitly.
#define IRQ_MAILBOX_VECTOR          0 // AP-to-CP mailbox
#define IRQ_WDT_VECTOR              1 // CP WatchDog
#define IRQ_DUAL_TIMER_VECTOR       2 // CP Dual-Timer
#define IRQ_DMAC_VECTOR             3 // CP Generic DMA
#define IRQ_APC_VECTOR              4 // APC
#define IRQ_NPU_VECTOR              5 // aka MVA, LUNA, or Accelerator
#define IRQ_INNER_TIMER0_VECTOR     6 // Timer0 inside CP
#define IRQ_SWI_VECTOR              7 // Software
#define IRQ_UART3_VECTOR            8 // UART3 in common sub wrapper (level=2)
#define IRQ_INNER_TIMER1_VECTOR     10 // Timer1 inside CP (level=3)
#define IRQ_SWI2_VECTOR             11 // Software 2 (level=3)
#define IRQ_INNER_TIMER2_VECTOR     13 // Timer2 inside CP (level=5)
#define IRQ_NMI_VECTOR              14 // NMI
#define IRQ_DVP_VECTOR              15 // DVP
#define IRQ_PSRAM_VECTOR            16 // PSRAM
#define IRQ_AP2CP_VECTOR            17 // AP2CP combined interrupt

//main_IRQ for those sub IRQs
#define IRQ_MAIN_VECTOR            IRQ_AP2CP_VECTOR

//sub_IRQ # via IRQ_AP2CP_VECTOR
// (those sub_IRQ numbers are actually IRQ numbers on AP!)
#define SUB_IRQ_DMAC_VECTOR                 0
#define SUB_IRQ_USBC_VECTOR                 1
#define SUB_IRQ_SDIO_VECTOR                 2
#define SUB_IRQ_CRYPTO_VECTOR               3
#define SUB_IRQ_QSPI_FLASHC_VECTOR          4
#define SUB_IRQ_PSRAM_VECTOR                5
#define SUB_IRQ_TIMER_VECTOR                6
#define SUB_IRQ_WDT_VECTOR                  7
#define SUB_IRQ_UART0_VECTOR                8
#define SUB_IRQ_UART1_VECTOR                9
#define SUB_IRQ_UART2_VECTOR                10
#define SUB_IRQ_IR_VECTOR                   11
#define SUB_IRQ_SPI0_VECTOR                 12
#define SUB_IRQ_SPI1_VECTOR                 13
#define SUB_IRQ_I2C0_VECTOR                 14
#define SUB_IRQ_I2C1_VECTOR                 15
#define SUB_IRQ_GPT_VECTOR                  16
#define SUB_IRQ_GPIO0_VECTOR                17
#define SUB_IRQ_GPIO1_VECTOR                18
#define SUB_IRQ_GPADC_VECTOR                19
#define SUB_IRQ_TRNG_VECTOR                 20
#define SUB_IRQ_CMN_MAILBOX_VECTOR          21
#define SUB_IRQ_CMN_UART_VECTOR             22
#define SUB_IRQ_AON_KEYSENSE_VECTOR         23
#define SUB_IRQ_AON_CBUTTON_VECTOR          24
#define SUB_IRQ_AON_RTC_VECTOR              25
#define SUB_IRQ_AON_IWDT_VECTOR             26
#define SUB_IRQ_AON_EFUSE_VECTOR            27
#define SUB_IRQ_AON_WAKEUP_VECTOR           28

/*****************************************************************************
 * System clock
 ****************************************************************************/

// IC_BOARD == 1 ---> ASIC
// IC_BOARD == 0 ---> FPGA

#if IC_BOARD // ASIC

//#define MAIN_FREQUENCE    (400000000) // 400 MHz
#define MAIN_FREQUENCE    (300000000) // 300 MHz

static inline uint32_t XTAL_FREQ() {
    return 24000000;
}

static inline uint32_t CPUFREQ() {
    extern uint32_t CRM_GetCpCoreSysClkFreq(void);
    return CRM_GetCpCoreSysClkFreq();
}

static inline uint32_t HCLKFREQ() {
    extern uint32_t CRM_GetCpHclkClkFreq(void);
    return CRM_GetCpHclkClkFreq();
}

static inline uint32_t PCLKFREQ() {
    extern uint32_t CRM_GetCpPclkPeriClkFreq(void);
    return CRM_GetCpPclkPeriClkFreq();
}

#else // FPGA

#define MAIN_FREQUENCE    (48000000)

static inline uint32_t XTAL_FREQ() {
    return 48000000;
}

static inline uint32_t CPUFREQ() {
    return (MAIN_FREQUENCE);
}

static inline uint32_t HCLKFREQ() {
    return (MAIN_FREQUENCE);
}

static inline uint32_t PCLKFREQ() {
    return (MAIN_FREQUENCE);
}

#endif // IC_BOARD


/************************************************************************************
 * GI(Global Interrupt) and IRQ vector inline functions
 ************************************************************************************/
extern uint32_t gint_flag;

// Is Global Interrupt enabled? 0 = disabled, 1 = enabled
static inline uint8_t GINT_enabled()
{
	//FIXME: is it OK on HiFi4?
	// INTLEVEL @ PS Register on XEA2
	// XCHAL_NUM_INTLEVELS or XCHAL_EXCM_LEVEL indicates Global Interrupt is disabled
#ifdef XOS_USED
    uint32_t intlevel = xos_get_int_pri_level();
    return (intlevel >= XOS_MAX_OS_INTLEVEL ? 0 : 1);
#else
    uint32_t intlevel = xthal_intlevel_get();
    return (intlevel >= XCHAL_NUM_INTLEVELS ? 0 : 1);
#endif
    //return (intlevel == 15 ? 0 : 1);
}

// Enable GINT
void enable_GINT();
//static inline void enable_GINT()

// Disable GINT
static inline void disable_GINT()
{
	//FIXME: is it OK on HiFi4?
#ifdef XOS_USED
	gint_flag = xos_disable_interrupts();
#else
	gint_flag = xthal_disable_interrupts();
//	gint_flag = XTOS_SET_INTLEVEL(15);
	XT_RSYNC();
#endif
}

// ISR function prototype
typedef void (*ISR)(void);

// Register ISR into Interrupt Vector Table
static inline void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old)
{
	//FIXME: call older function, is it OK on HiFi4?
#ifdef XOS_USED
	//no old isr return!!
	xos_register_interrupt_handler(irq_no, (XosIntFunc *)isr, NULL);
#else
	// ISR old = _xtos_set_interrupt_handler(irq_no, isr);
	// if (isr_old != NULL)	*isr_old = old;
#endif
}

static inline void enable_IRQ(uint32_t irq_no)
{
	//FIXME: call older function, is it OK on HiFi4?
#ifdef XOS_USED
	xos_interrupt_enable(irq_no);
#else
	// xtos_interrupt_enable(irq_no);
#endif
}

static inline void disable_IRQ(uint32_t irq_no)
{
	//FIXME: call older function, is it OK on HiFi4?
#ifdef XOS_USED
	xos_interrupt_disable(irq_no);
#else
	// xtos_interrupt_disable(irq_no);
#endif
}

static inline uint8_t IRQ_enabled(uint32_t irq_no)
{
    //FIXME: call older function, is it OK on HiFi4?
#ifdef XOS_USED
    return (xos_interrupt_enabled(irq_no) ? 1 : 0);
#else
    return (xtos_interrupt_enabled(irq_no) > 0 ? 1 : 0);
#endif
}

#ifndef CSK6001_MPW
// Subordinate interrupts are reported to the processor via IRQ_AP2CP_VECTOR (a.k.a. AP2CP combined Interrupt)
void register_sub_ISR(uint32_t sub_irq_no, ISR isr, ISR* isr_old); // Register sub ISR
void enable_sub_IRQ(uint32_t sub_irq_no); // Enable sub interrupt
void disable_sub_IRQ(uint32_t sub_irq_no); // Disable sub interrupt
uint8_t sub_IRQ_enabled(uint32_t sub_irq_no); // Is sub interrupt enabled?
//void clear_sub_IRQ(uint32_t sub_irq_no); // Clear sub interrupt
#endif // CSK6001_MPW

/************************************************************************************
 * DMA definitions
 ************************************************************************************/

// Hardware handshaking ID of each peripheral with DMA Controller
// please refer to Chap 10.2.3.3. "CP DMAC Peripherals" in Venus_Chip_Specification.pdf
#define DMA_HSID_APC_OUT_CH0        0
#define DMA_HSID_APC_OUT_CH1        1
#define DMA_HSID_APC_OUT_CH2        2
#define DMA_HSID_APC_OUT_CH3        3
#define DMA_HSID_APC_IN_CH0         4
#define DMA_HSID_APC_IN_CH1         5
#define DMA_HSID_APC_IN_CH2         6
#define DMA_HSID_APC_IN_CH3         7
#define DMA_HSID_APC_IN_CH4         8
#define DMA_HSID_APC_IN_CH5         9
//#define DMA_HSID_PSRAM              10 //FIXME:

#ifndef CSK6001_MPW
#define DMA_HSID_UART2_TX           11  // UART2 TX (on AP by default)
#define DMA_HSID_UART2_RX           12  // UART2 RX (on AP by default)
#define DMA_HSID_SPI1_TX            13  // SPI1 TX on (on AP by default)
#define DMA_HSID_SPI1_RX            14  // SPI1 RX on (on AP by default)
#define DMA_HSID_I2C1               15  // I2C1 on AP (on AP by default)
#endif // CSK6001_MPW

#define DMA_HSID_ADC_CH0    DMA_HSID_APC_IN_CH0
#define DMA_HSID_ADC_CH1    DMA_HSID_APC_IN_CH1
#define DMA_HSID_ADC_CH2    DMA_HSID_APC_IN_CH2
#define DMA_HSID_ADC_CH3    DMA_HSID_APC_IN_CH3

#define DMA_HSID_ECHO_CH0   DMA_HSID_APC_IN_CH4
#define DMA_HSID_ECHO_CH1   DMA_HSID_APC_IN_CH5
#define DMA_HSID_DAC_CH0    DMA_HSID_APC_OUT_CH0
#define DMA_HSID_DAC_CH1    DMA_HSID_APC_OUT_CH1


// Number of DMA channels
#define DMA_NUMBER_OF_CHANNELS           ((uint8_t) 8)

//default or preferred DMA channel definition
#define DMA_CH_MEM_CPY          ((uint8_t) 0)   // memory copy operation
#define DMA_CH_ADC01L_DEF       ((uint8_t) 1)   // ADC: audio recording
#define DMA_CH_ADC01R_DEF       ((uint8_t) 4)   // ADC: audio recording
#define DMA_CH_ADC23L_DEF       ((uint8_t) 2)   // ADC: audio recording
#define DMA_CH_ADC23R_DEF       ((uint8_t) 5)   // ADC: audio recording
#define DMA_CH_DAC01L_DEF       ((uint8_t) 3)   // DAC: audio playback
#define DMA_CH_DAC01R_DEF       ((uint8_t) 6)   // DAC: audio playback
#define DMA_CH_I2S_RXL_DEF      ((uint8_t) 2)   // I2S recording
#define DMA_CH_I2S_RXR_DEF      ((uint8_t) 5)   // I2S recording
#define DMA_CH_I2S_TXL_DEF      ((uint8_t) 3)   // I2S playback
#define DMA_CH_I2S_TXR_DEF      ((uint8_t) 6)   // I2S playback
#define DMA_CH_ECHO_RXL_DEF     ((uint8_t) 3)   // Echo (digital)
#define DMA_CH_ECHO_RXR_DEF     ((uint8_t) 6)   // Echo (digital)

#ifndef CSK6001_MPW
#define DMA_CH_UART_TX_DEF          ((uint8_t) 5)   // UART
#define DMA_CH_UART_RX_DEF          ((uint8_t) 4)   // UART
#define DMA_CH_SPI_TX_DEF           ((uint8_t) 3)   // SPI
#define DMA_CH_SPI_RX_DEF           ((uint8_t) 2)   // SPI
#define DMA_CH_I2C_DEF              ((uint8_t) 6)   // I2C
#endif // CSK6001_MPW

// AHB master interface of memory that stores LLI (Linked List Item) for channel
#define DMAH_CH_LMS     0

#ifdef CSK6001_MPW
static const uint8_t DMA_CHANNELS_FIFO_DEPTH[DMA_NUMBER_OF_CHANNELS] = { 64, 64, 64, 64,
                                                                        16, 16, 16, 16 }; //in bytes
#else
static const uint8_t DMA_CHANNELS_FIFO_DEPTH[DMA_NUMBER_OF_CHANNELS] = { 128, 64, 64, 64, //ch0:64=>128
                                                                        16, 16, 16, 16 }; //in bytes
#endif // !CSK6001_MPW

#endif /* INCLUDE_VENUS_CP_H_ */
