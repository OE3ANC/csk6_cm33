/*
 * venus_ap.h
 *
 *  Created on: Jul 7, 2020
 *
 */

#ifndef INCLUDE_VENUS_AP_H_
#define INCLUDE_VENUS_AP_H_

#include <stddef.h> // for NULL etc.
#include <stdint.h>
//#include <core_cm33.h> // for IRQ operations
#include <ARMCM33_DSP_FP.h> // for IRQn_Type definition

/*****************************************************************************
 * Header for All peripherals
 ****************************************************************************/
#include <ana_aon_regfile_reg_venus.h>
#include <ana_core_regfile_reg_venus.h>
// #include <aon_efuse_ctrl_reg_venus.h>
#include <aon_iomux_reg_venus.h>
#include <aon_pmuctrl_reg_venus.h>
#include <audpll_ctrl_reg_venus.h>
// #include <cbutton_reg_venus.h>
#include <cmn_iomux_reg_venus.h>
#include <cmn_mailbox_reg_venus.h>
#include <cmn_sysctrl_reg_venus.h>
// #include <crypto_reg_venus.h>
// #include <dual_timer_reg_venus.h>
//#include <ftsdc021.h>
#include <gpio_reg_venus.h>
#include <gpt_reg_venus.h>
#include <i2c_reg_venus.h>
// #include <ir_reg_venus.h>
#include <keysense_reg_venus.h>
#include <psram_mc_reg_venus.h>
#include <syspll_ctrl_reg_venus.h>
// #include <trng_reg_venus.h>
#include <dw_uart_reg_venus.h>
// #include <wdt_reg_venus.h>
//#include <usb/usb_venus.h>

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
#define _FAST_FUNC_RO           __attribute__ ((section (".itcm")))
//fast data value initialize
#define _FAST_DATA_VI
//fast data zero initialize
#define _FAST_DATA_ZI

#define _DMA
#define _DMA_PRAM               __attribute__((aligned(32)))


/************************************************************************************
 * Cache definitions
 ************************************************************************************/
enum cache_t { ICACHE, DCACHE };

static inline unsigned long ICACHE_WAY(enum cache_t cache)
{
    return 2;
}

static inline unsigned long DCACHE_WAY(enum cache_t cache)
{
    return 4;
}

static inline unsigned long CACHE_LINE_SIZE(enum cache_t cache)
{
    return 32;
}


/************************************************************************************
 * Memory map
 ************************************************************************************/

#define ROM_BASE                0x00000000 // size=64KB
#define ITCM_BASE               0x00010000 // size=12KB
#define CODE_RAM_BASE           0x00080000 // size=384KB (CODE_RAM + SYS_RAM)
#define IFLASH_BASE             0x18000000 // size=8MB, max_size=128MB (same as FLASH, for code)

#define DTCM_BASE               0x20000000 // size=12KB
#define SYS_RAM_BASE            0x20080000 // size=384KB (CODE_RAM + SYS_RAM)
#define SHMEM_BASE              0x28000000 // size=640KB (share with CP)

#define PSRAM_BASE              0x30000000 // size=8MB, max_size=128MB
#define FLASH_BASE              0x38000000 // size=8MB, max_size=128MB

#define PSRAM_BASE              0x30000000 // size=8MB, max_size=128MB
#define FLASH_BASE              0x38000000 // size=8MB, max_size=128MB

#define DMAC_BASE               0x40000000 // size=16MB
#define USBC_BASE               0x41000000 // size=16MB
#define SDIO_BASE               0x42000000 // size=16MB

#define EFUSE_CTRL_BASE         0x44000000 // size=1MB
#define TIMER_BASE              0x44100000 // size=1MB
#define WDT_BASE                0x44200000 // size=1MB
#define CRYPTO_BASE             0x44300000

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
#define GPADC_BASE              0x45B00000 // size=1MB
#define TRNG_BASE               0x45C00000 // size=1MB
#define ABB_BASE                0x45D00000 // size=1MB

#define CMN_SYSCTRL_BASE        0x46000000 // size=64KB
#define CMN_ANACORE_BASE        0x46010000 // size=64KB
#define CMN_SYSPLL_CTRL_BASE    0x46020000 // size=64KB
#define CMN_AUDPLL_CTRL_BASE    0x46030000 // size=(1M-3*64KB)
#define CMN_MAILBOX_BASE        0x46100000 // size=1MB
#define CMN_IOMUX_BASE          0x46200000 // size=1MB
#define CMN_UART_BASE           0x46300000 // size=1MB
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
#define IP_USBC               		((CSK_USB_RegDef *) USBC_BASE)
//#define IP_SDIO                	((ftsdc021_reg *) SDIO_BASE)
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
//#define VAD                		((XXX_TypeDef *) AON_VAD_BASE)
#define IP_AON_IOMUX              	((AON_IOMUX_RegDef *) AON_IOMUX_BASE)
#define IP_PSRAMC                	((PSRAM_MC_RegDef *) CMN_PSRAMC_BASE)

/**
  * @}
  */


/************************************************************************************
 * IRQ Vector
 ************************************************************************************/

#define IRQ_DMAC_VECTOR                 0
#define IRQ_USBC_VECTOR                 1
#define IRQ_SDIO_VECTOR                 2
#define IRQ_CRYPTO_VECTOR               3
#define IRQ_QSPI_FLASHC_VECTOR          4
#define IRQ_EFUSE_CTRL_VECTOR           5
#define IRQ_TIMER_VECTOR                6
#define IRQ_WDT_VECTOR                  7
#define IRQ_UART0_VECTOR                8
#define IRQ_UART1_VECTOR                9
#define IRQ_UART2_VECTOR                10
#define IRQ_IR_VECTOR                   11
#define IRQ_SPI0_VECTOR                 12
#define IRQ_SPI1_VECTOR                 13
#define IRQ_I2C0_VECTOR                 14
#define IRQ_I2C1_VECTOR                 15
#define IRQ_GPT_VECTOR                  16
#define IRQ_GPIO0_VECTOR                17
#define IRQ_GPIO1_VECTOR                18
#define IRQ_GPADC_VECTOR                19
#define IRQ_TRNG_VECTOR                 20
#define IRQ_CMN_MAILBOX_VECTOR          21
#define IRQ_CMN_UART_VECTOR             22
#define IRQ_AON_KEYSENSE_VECTOR         23
#define IRQ_AON_CBUTTON_VECTOR          24
#define IRQ_AON_RTC_VECTOR              25
#define IRQ_AON_IWDT_VECTOR             26
#define IRQ_AON_WAKEUP_VECTOR           27
#define IRQ_AON_PMUC_VECTOR             28 // Reserved
#define IRQ_CP0_VECTOR                  29 // Reserved
#define IRQ_CP1_VECTOR                  30 // Reserved
#define IRQ_CP2_VECTOR                  31 // Reserved
#define IRQ_MAX                         31

/*****************************************************************************
 * System clock
 ****************************************************************************/

// IC_BOARD == 1 ---> ASIC
// IC_BOARD == 0 ---> FPGA

#if IC_BOARD // ASIC

#define DEF_MAIN_FREQUENCE    (300000000) // 300 MHz
//#define DEF_MAIN_FREQUENCE    (24000000) // 24 MHz

static inline uint32_t CPUFREQ() {
    extern uint32_t CRM_GetApFreq();
    return CRM_GetApFreq();
}

static inline uint32_t HCLKFREQ() {
    extern uint32_t CRM_GetApahbFreq();
    return CRM_GetApahbFreq();
}

static inline uint32_t PCLKFREQ() {
    extern uint32_t CRM_GetApperiapbFreq();
    return CRM_GetApperiapbFreq();
}

#else // FPGA

#define DEF_MAIN_FREQUENCE    (48000000) // 48MHZ

static inline uint32_t CPUFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

static inline uint32_t HCLKFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

static inline uint32_t PCLKFREQ() {
    return (DEF_MAIN_FREQUENCE);
}

#endif // IC_BOARD


// evaluate frequency division coefficient: N / M (N <= M),
// the divided frequency CAN be more than, less than or equal to freq_out_req.
// return value: N @ high 16bits, M @ low 16bits. 0 indicates failure.
// *freq_out_p = freq_out_divided
uint32_t eval_freq_divNM(uint16_t N_max, uint16_t M_max, uint32_t freq_in,
                        uint32_t freq_out_req, int32_t *freq_out_p);

// evaluate frequency division coefficient: N / M (N <= M),
// the divided frequency CANNOT be more than freq_out_max!
// return value: N @ high 16bits, M @ low 16bits. 0 indicates failure.
// *freq_out_p = freq_out_divided
uint32_t eval_freq_divNM2(uint16_t N_max, uint16_t M_max, uint32_t freq_in,
                        uint32_t freq_out_max, int32_t *freq_out_p);

/************************************************************************************
 * GI(Global Interrupt) and IRQ vector inline functions
 ************************************************************************************/
#define MAX_INTERRUPT_PRIORITY_RVAL     (0x5UL << (8 - __NVIC_PRIO_BITS)) // __NVIC_PRIO_BITS = 5
#define MID_INTERRUPT_PRIORITY          ((1 << (__NVIC_PRIO_BITS - 1)) - 1) // 15
#define DEF_INTERRUPT_PRIORITY          (MID_INTERRUPT_PRIORITY > 5 ? MID_INTERRUPT_PRIORITY : 5) // 15

// Is Global Interrupt enabled? 0 = disabled, 1 = enabled
static inline uint8_t GINT_enabled()
{
    //TODO: is it OK for Arm Star?
    //uint32_t ret = __get_PRIMASK(); // based on PRIMASK
    //return (ret ? 0 : 1);
    uint32_t ret = __get_BASEPRI(); // based on BASEPRI
    return (ret == MAX_INTERRUPT_PRIORITY_RVAL ? 0 : 1);
}

// Enable GINT
static inline void enable_GINT()
{
    //TODO: is it OK for Arm Star?
    //__set_PRIMASK(0); // based on PRIMASK
    __set_BASEPRI(0); // based on BASEPRI
}

// Disable GINT
static inline void disable_GINT()
{
    //TODO: is it OK for Arm Star?
    //__set_PRIMASK(1); // based on PRIMASK
    __set_BASEPRI(MAX_INTERRUPT_PRIORITY_RVAL); // based on BASEPRI
}

// ISR function prototype
typedef void (*ISR)(void);

// Register ISR into Interrupt Vector Table
void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old);
//static inline void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old)
//{
//}

static inline uint8_t IRQ_enabled(uint32_t irq_no)
{
    //TODO: is it OK for Arm Star?
    return (uint8_t)(NVIC_GetEnableIRQ(irq_no));
}

static inline void enable_IRQ(uint32_t irq_no)
{
   //TODO: is it OK for Arm Star?
   NVIC_EnableIRQ(irq_no);
}

static inline void disable_IRQ(uint32_t irq_no)
{
    //TODO: is it OK for Arm Star?
    NVIC_DisableIRQ(irq_no);
}

static inline void clear_IRQ(uint32_t irq_no)
{
    //TODO: is it OK for Arm Star?
    NVIC_ClearPendingIRQ(irq_no);
}

/************************************************************************************
 * DMA definitions
 ************************************************************************************/

// Hardware handshaking ID of each peripheral with DMA Controller
// default configuration (CFG0)
#define DMA_HSID_UART0_TX       0
#define DMA_HSID_UART0_RX       1
#define DMA_HSID_UART1_TX       2
#define DMA_HSID_UART1_RX       3
#define DMA_HSID_UART2_TX       4
#define DMA_HSID_UART2_RX       5
#define DMA_HSID_IR_TX          6
#define DMA_HSID_IR_RX          7

#define DMA_HSID_SPI0_TX        8
#define DMA_HSID_SPI0_RX        9
#define DMA_HSID_SPI1_TX        10
#define DMA_HSID_SPI1_RX        11

#define DMA_HSID_I2C0           12
#define DMA_HSID_I2C1           13
#define DMA_HSID_GPT_TX         14
#define DMA_HSID_GPT_RX         15

//TODO: Add other HSID...

#define DMA_HSID_I2C0        12
#define DMA_HSID_I2C1        13

// Number of DMA channels & FIFO depth of each channel
#ifdef CSK6001_MPW
#define DMA_NUMBER_OF_CHANNELS           ((uint8_t) 6)
static const uint8_t DMA_CHANNELS_FIFO_DEPTH[DMA_NUMBER_OF_CHANNELS] = { 64, 64, 16, 16, 16, 16}; //in bytes
#else
#define DMA_NUMBER_OF_CHANNELS           ((uint8_t) 8)
static const uint8_t DMA_CHANNELS_FIFO_DEPTH[DMA_NUMBER_OF_CHANNELS] = { 64, 64, 16, 16, 16, 16, 16, 16}; //in bytes
#endif

//default or preferred DMA channel definition
#define DMA_CH_UART_TX_DEF          ((uint8_t) 5)   // UART
#define DMA_CH_UART_RX_DEF          ((uint8_t) 4)   // UART
#define DMA_CH_SPI_TX_DEF           ((uint8_t) 3)   // SPI
#define DMA_CH_SPI_RX_DEF           ((uint8_t) 2)   // SPI

// AHB master interface of memory that stores LLI (Linked List Item) for channel
#define DMAH_CH_LMS     0

extern void mpu_init( void );

#if defined(CSK6001_MPW)
#define IRQ_CP_MAILBOX_VECTOR          0 // AP-to-CP mailbox
#define IRQ_CP_WDT_VECTOR              1 // CP WatchDog
#define IRQ_CP_DUAL_TIMER_VECTOR       2 // CP Dual-Timer
#define IRQ_CP_DMAC_VECTOR             3 // CP Generic DMA
#define IRQ_CP_APC_VECTOR              4 // APC
#define IRQ_CP_NPU_VECTOR              5 // aka MVA, LUNA, or Accelerator
#define IRQ_CP_INNER_TIMER0_VECTOR     6 // Timer0 inside CP
#define IRQ_CP_SWI_VECTOR              7 // Software
#define IRQ_CP_UART3_VECTOR            8 // UART3 in common sub wrapper (level=2)
#define IRQ_CP_INNER_TIMER1_VECTOR     10 // Timer1 inside CP (level=3)
#define IRQ_CP_SWI2_VECTOR             11 // Software 2 (level=3)
#define IRQ_CP_INNER_TIMER2_VECTOR     13 // Timer2 inside CP (level=5)
#define IRQ_CP_NMI_VECTOR              14 // NMI
#elif defined(CSK6001)
#define IRQ_CP_MAILBOX_VECTOR          0  // AP-to-CP mailbox
#define IRQ_CP_WDT_VECTOR              1  // CP WatchDog
#define IRQ_CP_DUAL_TIMER_VECTOR       2  // CP Dual-Timer
#define IRQ_CP_DMAC_VECTOR             3  // CP Generic DMA
#define IRQ_CP_APC_VECTOR              4  // APC
#define IRQ_CP_NPU_VECTOR              5  // LUNA
#define IRQ_CP_CMN_UART3_VECTOR        6  // CMN Uart
#define IRQ_CP_AON_VAD_VECTOR          7  // VAD
#define IRQ_CP_CP_UART_VECTOR          8  // CP Uart
#define IRQ_CP_VIC_VECTOR              10 // VIC
#define IRQ_CP_PSRAM_VECTOR            11 // PSRAM
#define IRQ_CP_AP2CP_VECTOR            12 // AP to CP
#endif

void HAL_CP_Interrupt_Init();
void HAL_CP_Interrupt_Deinit();
void HAL_CP_Interrupt_Enable(uint32_t irq_no);
void HAL_CP_Interrupt_Disable(uint32_t irq_no);
void HAL_CP_Interrupt_RegisterISR(uint32_t irq_no, ISR isr, ISR* isr_old);

#endif /* INCLUDE_VENUS_AP_H_ */

