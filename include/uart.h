/*****************************************************************************
 * UARTx - CSK
 ****************************************************************************/
#ifndef __UART_CSK6001_H__
#define __UART_CSK6001_H__

#include "dma.h"
#include "Driver_UART.h"
#include "venus_ap.h"

/* IER Register (+0x4) */
#define UARTC_IER_DISABLE               0x00
#define UARTC_IER_RDR                   0x01 /* Data Ready Enable */
#define UARTC_IER_THRE                  0x02 /* THR Empty Enable */
#define UARTC_IER_RLS                   0x04 /* Receive Line Status Enable */
#define UARTC_IER_MS                    0x08 /* Modem Staus Enable */

/* IIR Register (+0x8) */
#define UARTC_IIR_NONE                  0x01 /* No interrupt pending */
#define UARTC_IIR_RLS                   0x06 /* Receive Line Status */
#define UARTC_IIR_RDA                   0x04 /* Receive Data Available */
#define UARTC_IIR_RTO                   0x0c /* Receive Time Out */
#define UARTC_IIR_THRE                  0x02 /* THR Empty */
#define UARTC_IIR_MODEM                 0x00 /* Modem Status */
#define UARTC_IIR_INT_MASK              0x0f /* Initerrupt Status Bits Mask */
#define UARTC_IIR_FIFO_EN               0xc0 /* FIFO mode is enabled, set when FCR[0] is 1 */

/* FCR Register (+0x8) */
#define UARTC_FCR_FIFO_EN               0x01 /* FIFO Enable */
#define UARTC_FCR_RFIFO_RESET           0x02 /* Rx FIFO Reset */
#define UARTC_FCR_TFIFO_RESET           0x04 /* Tx FIFO Reset */
#define UARTC_FCR_DMA_EN                0x08 /* Select UART DMA mode */

#define UARTC_FCR_TFIFO_TRGL0           0x00 /* TX FIFO int trigger level - Empty */
#define UARTC_FCR_TFIFO_TRGL1           0x10 /* TX FIFO int trigger level - 2 char */
#define UARTC_FCR_TFIFO_TRGL2           0x20 /* TX FIFO int trigger level - 1/4 full */
#define UARTC_FCR_TFIFO_TRGL3           0x30 /* TX FIFO int trigger level - 1/2 full */

#define UARTC_FCR_RFIFO_TRGL0           0x00 /* RX FIFO int trigger level - 1 char */
#define UARTC_FCR_RFIFO_TRGL1           0x40 /* RX FIFO int trigger level - 1/4 full */
#define UARTC_FCR_RFIFO_TRGL2           0x80 /* RX FIFO int trigger level - 1/2 full */
#define UARTC_FCR_RFIFO_TRGL3           0xc0 /* RX FIFO int trigger level - 2 less than full */

/* LCR Register (+0xc) */
#define UARTC_LCR_BITS5                 0x00
#define UARTC_LCR_BITS6                 0x01
#define UARTC_LCR_BITS7                 0x02
#define UARTC_LCR_BITS8                 0x03
#define UARTC_LCR_STOP1                 0x00
#define UARTC_LCR_STOP1_5OR2            0x04 // When LCR[1:0] is zero 1.5 stop bits
                                             // else 2 stop bits

#define UARTC_LCR_PARITY_NONE           0x00 /* No Parity Check */
#define UARTC_LCR_PARITY_EVEN           0x18 /* Even Parity */
#define UARTC_LCR_PARITY_ODD            0x08 /* Odd Parity */
#define UARTC_LCR_SETBREAK              0x40 /* Set Break condition */
#define UARTC_LCR_DLAB                  0x80 /* Divisor Latch Access Bit */

#define UARTC_LCR_DLAB                  0x80 /* Divisor Latch Access Bit */

/* MCR Register (+0x10) */
#define UARTC_MCR_DTR                   0x01 /* Data Terminal Ready */
#define UARTC_MCR_RTS                   0x02 /* Request to Send */
#define UARTC_MCR_OUT1                  0x04 /* output1 */
#define UARTC_MCR_OUT2                  0x08 /* output2 or global interrupt enable */
#define UARTC_MCR_LPBK                  0x10 /* loopback mode */
#define UARTC_MCR_AFE                   0x20 /* Auto flow control */

/* LSR Register (+0x14) */
#define UARTC_LSR_RDR                   0x1 /* Data Ready */
#define UARTC_LSR_OE                    0x2 /* Overrun Error */
#define UARTC_LSR_PE                    0x4 /* Parity Error */
#define UARTC_LSR_FE                    0x8 /* Framing Error */
#define UARTC_LSR_BI                    0x10 /* Break Interrupt */
#define UARTC_LSR_THRE                  0x20 /* THR/FIFO Empty */
#define UARTC_LSR_TEMT                  0x40 /* THR/FIFO and TFR Empty */
#define UARTC_LSR_ERRF                  0x80 /* FIFO Data Error */

/* MSR Register (+0x18) */
#define UARTC_MSR_DCTS                  0x1 /* Delta CTS */
#define UARTC_MSR_DDSR                  0x2 /* Delta DSR */
#define UARTC_MSR_TERI                  0x4 /* Trailing Edge RI */
#define UARTC_MSR_DDCD                  0x8 /* Delta CD */
#define UARTC_MSR_CTS                   0x10 /* Clear To Send */
#define UARTC_MSR_DSR                   0x20 /* Data Set Ready */
#define UARTC_MSR_RI                    0x40 /* Ring Indicator */
#define UARTC_MSR_DCD                   0x80 /* Data Carrier Detect */


// CONSTANT MACRO
// ******************************************UART0
#define CSK_UART0_FIFO            (32)// Depth of FIFO (Bytes)

#define CSK_UART0_TX_DMA_CH       (0)
#define CSK_UART0_TX_DMA_REQSEL   (DMA_HSID_UART0_TX)
#define CSK_UART0_RX_DMA_CH       (1)
#define CSK_UART0_RX_DMA_REQSEL   (DMA_HSID_UART0_RX)
// ******************************************


// ******************************************UART1
#define CSK_UART1_FIFO            (32)// Depth of FIFO (Bytes)

#define CSK_UART1_TX_DMA_CH       (2)
#define CSK_UART1_TX_DMA_REQSEL   (DMA_HSID_UART1_TX)
#define CSK_UART1_RX_DMA_CH       (3)
#define CSK_UART1_RX_DMA_REQSEL   (DMA_HSID_UART1_RX)
// ******************************************


// ******************************************UART2
#define CSK_UART2_FIFO            (32)// Depth of FIFO (Bytes)

#define CSK_UART2_TX_DMA_CH       (4)
#define CSK_UART2_TX_DMA_REQSEL   (DMA_HSID_UART2_TX)
#define CSK_UART2_RX_DMA_CH       (5)
#define CSK_UART2_RX_DMA_REQSEL   (DMA_HSID_UART2_RX)
// ******************************************

<<<<<<< HEAD
=======

// ******************************************UART3
#define CSK_UART3_FIFO            (32)// Depth of FIFO (Bytes)
// ******************************************

>>>>>>> feature/zephyr
#define CSK_UART_RX_TRIG_LVL_0   (1)           // 1 character in FIFO
#define CSK_UART_RX_TRIG_LVL_1   (8)           // FIFO 1/4 full
#define CSK_UART_RX_TRIG_LVL_2   (16)          // FIFO 1/2 full
#define CSK_UART_RX_TRIG_LVL_3   (30)          // FIFO 2 less than full

<<<<<<< HEAD
#define CSK_UART_RX_TRIG_LVL                   CSK_UART_RX_TRIG_LVL_0
=======
#define CSK_UART_RX_TRIG_LVL                   CSK_UART_RX_TRIG_LVL_1
>>>>>>> feature/zephyr


#if CSK_UART_RX_TRIG_LVL == CSK_UART_RX_TRIG_LVL_0     // Greater than or equal 1 character trigger DMA transmit
#define CSK_UART_RX_TRIG_LVL_PARA              (UARTC_FCR_RFIFO_TRGL0)
#define CSK_UART_RX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_RX_DMA_BSIZE_PARA             (DMA_BSIZE_1)
#elif CSK_UART_RX_TRIG_LVL == CSK_UART_RX_TRIG_LVL_1   // Greater than or equal 8 character trigger DMA transmit
#define CSK_UART_RX_TRIG_LVL_PARA              (UARTC_FCR_RFIFO_TRGL1)
#define CSK_UART_RX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_RX_DMA_BSIZE_PARA             (DMA_BSIZE_4)
#elif CSK_UART_RX_TRIG_LVL == CSK_UART_RX_TRIG_LVL_2   // Greater than or equal 16 character trigger DMA transmit
#define CSK_UART_RX_TRIG_LVL_PARA              (UARTC_FCR_RFIFO_TRGL2)
#define CSK_UART_RX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_RX_DMA_BSIZE_PARA             (DMA_BSIZE_8)
#elif CSK_UART_RX_TRIG_LVL == CSK_UART_RX_TRIG_LVL_3   // Greater than or equal 30 character trigger DMA transmit
#define CSK_UART_RX_TRIG_LVL_PARA              (UARTC_FCR_RFIFO_TRGL3)
#define CSK_UART_RX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_RX_DMA_BSIZE_PARA             (DMA_BSIZE_16)
#else
#error "Parameter configure error in 'CSK_UART_RX_TRIG_LVL'"
#endif

#define CSK_UART_TX_TRIG_LVL_0   (0)           // Empty in FIFO
#define CSK_UART_TX_TRIG_LVL_1   (2)           // 2 character in FIFO
#define CSK_UART_TX_TRIG_LVL_2   (8)           // FIFO 1/4 full
#define CSK_UART_TX_TRIG_LVL_3   (16)          // FIFO 1/2 full

#define CSK_UART_TX_TRIG_LVL                   CSK_UART_TX_TRIG_LVL_2


#if CSK_UART_TX_TRIG_LVL == CSK_UART_TX_TRIG_LVL_0     // Less than or equal 0 character trigger DMA transmit
#define CSK_UART_TX_TRIG_LVL_PARA              (UARTC_FCR_TFIFO_TRGL0)
#define CSK_UART_TX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_TX_DMA_BSIZE_PARA             (DMA_BSIZE_32)
#elif CSK_UART_TX_TRIG_LVL == CSK_UART_TX_TRIG_LVL_1     // Less than or equal 2 character trigger DMA transmit
#define CSK_UART_TX_TRIG_LVL_PARA              (UARTC_FCR_TFIFO_TRGL1)
#define CSK_UART_TX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_TX_DMA_BSIZE_PARA             (DMA_BSIZE_16)
#elif CSK_UART_TX_TRIG_LVL == CSK_UART_TX_TRIG_LVL_2     // Less than or equal 8 character trigger DMA transmit
#define CSK_UART_TX_TRIG_LVL_PARA              (UARTC_FCR_TFIFO_TRGL2)
#define CSK_UART_TX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_TX_DMA_BSIZE_PARA             (DMA_BSIZE_16)
#elif CSK_UART_TX_TRIG_LVL == CSK_UART_TX_TRIG_LVL_3     // Less than or equal 16 character trigger DMA transmit
#define CSK_UART_TX_TRIG_LVL_PARA              (UARTC_FCR_TFIFO_TRGL3)
#define CSK_UART_TX_DMA_WIDTH_PARA             (DMA_WIDTH_BYTE)
#define CSK_UART_TX_DMA_BSIZE_PARA             (DMA_BSIZE_16)
#endif


// Modem configure marco
#define CSK_UART_MODEM_NONE            (0x00)
#define CSK_UART_AUTO_FLOW_EN_RTS      (0x01 << 0U)
#define CSK_UART_AUTO_FLOW_EN_CTS      (0x01 << 1U)


// UART flags
#define UART_FLAG_INITIALIZED          (1U << 0)
#define UART_FLAG_POWERED              (1U << 1)
#define UART_FLAG_CONFIGURED           (1U << 2)
#define UART_FLAG_TX_ENABLED           (1U << 3)
#define UART_FLAG_RX_ENABLED           (1U << 4)
#define UART_FLAG_SEND_ACTIVE          (1U << 5)

// UART Transfer Information (Run-Time)
typedef struct _UART_TRANSFER_INFO
{
    uint32_t rx_num;        // Total number of data to be received
    uint32_t tx_num;        // Total number of data to be send
    uint8_t *rx_buf;        // Pointer to in data buffer
    uint8_t *tx_buf;        // Pointer to out data buffer
    uint32_t rx_cnt;        // Number of data received
    uint32_t tx_cnt;        // Number of data sent
    uint32_t tx_def_val;    // Default value
    uint8_t  send_active;   // Send active flag
} UART_TRANSFER_INFO;

typedef struct _UART_RX_STATUS
{
    uint8_t rx_busy;            // Receiver busy flag
    uint8_t rx_overflow;        // Receive data overflow detected (cleared on start of next receive operation)
    uint8_t rx_break;           // Break detected on receive (cleared on start of next receive operation)
    uint8_t rx_framing_error;   // Framing error detected on receive (cleared on start of next receive operation)
    uint8_t rx_parity_error;    // Parity error detected on receive (cleared on start of next receive operation)
} UART_RX_STATUS;

// UART Information (Run-Time)
typedef struct _UART_INFO
{
    CSK_UART_SignalEvent_t cb_event;       // Event callback
    UART_RX_STATUS rx_status;              // Receive status flags
    UART_TRANSFER_INFO xfer;               // Transfer information
    uint32_t tx_trig_lvl;                  // FIFO Trigger level
    uint32_t rx_trig_lvl;                  // FIFO Trigger level
    uint32_t baudrate;                     // Baudrate
    uint8_t mode;                          // UART mode
    uint8_t flags;                         // UART driver flags

    uint8_t inter_en;                      // 0 for dma
                                           // 1 for interrupt
    void* workspace;

    uint8_t timeout;                       // 0 disable timeout
                                           // 1 enable timeout
} UART_INFO;

// UART DMA
typedef struct _UART_DMA
{
    uint8_t bsize_para;                    // DMA Burst size
    uint8_t en;                            // DMA Enable
    uint8_t channel;                       // DMA Channel
    uint8_t reqsel;                        // DMA request selection
    DMA_SignalEvent_t cb_event;            // DMA Event callback
} UART_DMA;

// UART Resources definitions
typedef const struct _UART_RESOURCES
{
    DW_UART_RegDef *reg;                  // Pointer to UART peripheral
    uint32_t irq_num;                      // UART IRQ Number
    void (*irq_handler)(void);
    uint32_t fifo_d;                       // Depth of FIFO
    UART_DMA *dma_tx;
    UART_DMA *dma_rx;
    UART_INFO *info;                       // Run-Time Information
    uint32_t modem;
}UART_RESOURCES;

#endif
