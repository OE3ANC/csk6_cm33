#ifndef __I2C_AE210P_H
#define __I2C_AE210P_H

#include "dma.h"
#include "venus_ap.h"
#include "Driver_I2C.h"
#include "ClockManager.h"

#include "assert.h"

#define MAX_XFER_SZ                 (256)           // 256 bytes

// Standard-mode
#define STANDARD_MODE_SP_MAX         50             // ns
#define STANDARD_MODE_SETUP_MIN      250
#define STANDARD_MODE_HOLD_MIN       300
#define STANDARD_MODE_HIGH_MIN       4700
#define STANDARD_MODE_RADIO_FIX      1

// Fast-mode
#define FAST_MODE_SP_MAX             50             // ns
#define FAST_MODE_SETUP_MIN          100
#define FAST_MODE_HOLD_MIN           300
#define FAST_MODE_HIGH_MIN           830
#define FAST_MODE_RADIO_FIX          2

// Standard-mode
#define FAST_MODE_PLUS_SP_MAX        50             // ns
#define FAST_MODE_PLUS_SETUP_MIN     50
#define FAST_MODE_PLUS_HOLD_MIN      150
#define FAST_MODE_PLUS_HIGH_MIN      300
#define FAST_MODE_PLUS_RADIO_FIX     2

#define I2C0_DMA_TX_CH        (2)
#define I2C0_DMA_RX_CH        (3)

#define I2C1_DMA_TX_CH        (4)
#define I2C1_DMA_RX_CH        (5)

#define _CMD_NO_ACT                    (0x0)           // no action
#define _CMD_ISSUE_TRANSACTION         (0x1)           // issue a data transaction (Master only)
#define _CMD_ACK                       (0x2)           // respond with an ACK to the received byte
#define _CMD_NACK                      (0x3)           // respond with a NACK to the received byte
#define _CMD_CLEAR_FIFO                (0x4)           // clear the FIFO
#define _CMD_RESET_I2C                 (0x5)           /* reset the I2C controller(abort current
                                                        ransaction, set the SDA and SCL line to the
                                                        open-drain mode, reset the Status Register and
                                                        the Interrupt Enable Register, and empty the
                                                        FIFO) */

#define _FIFOSIZE_2_BYTES               0x0
#define _FIFOSIZE_4_BYTES               0x1
#define _FIFOSIZE_8_BYTES               0x2
#define _FIFOSIZE_16_BYTES              0x3

typedef enum _I2C_CTRL_REG_ITEM_DIR
{
    I2C_MASTER_TX = 0x0,
    I2C_MASTER_RX = 0x1,
    I2C_SLAVE_TX = 0x1,
    I2C_SLAVE_RX = 0x0,
} I2C_CTRL_REG_ITEM_DIR;

// I2C driver running state
typedef enum _I2C_DRIVER_STATE
{
    I2C_DRV_NONE = 0x0,
    I2C_DRV_INIT = 0x1,
    I2C_DRV_POWER = 0x2,
    I2C_DRV_CFG_PARAM = 0x4,
    I2C_DRV_MASTER_TX = 0x8,
    I2C_DRV_MASTER_RX = 0x10,
    I2C_DRV_SLAVE_TX = 0x20,
    I2C_DRV_SLAVE_RX = 0x40,
    I2C_DRV_MASTER_TX_CMPL = 0x80,
    I2C_DRV_MASTER_RX_CMPL = 0x100,
    I2C_DRV_SLAVE_TX_CMPL = 0x200,
    I2C_DRV_SLAVE_RX_CMPL = 0x400,
} I2C_DRIVER_STATE;

typedef struct _CSK_I2C_STATUS
{
    uint32_t busy :1;                   ///< Busy flag
    uint32_t mode :1;                   ///< Mode: 0=Slave, 1=Master
    uint32_t direction :1;              ///< Direction: 0=Transmitter, 1=Receiver
    uint32_t general_call :1;           ///< General Call indication (cleared on start of next Slave operation)
    uint32_t arbitration_lost :1;       ///< Master lost arbitration (cleared on start of next Master operation)
    uint32_t bus_error :1;              ///< Bus error detected (cleared on start of next Master/Slave operation)
    uint32_t slave_rx_over_flow :1;
} CSK_I2C_STATUS;

typedef struct _I2C_TRANSFER_INFO
{
    uint8_t* tx_buf;
    uint32_t tx_num;                       /// total transmit number
    uint32_t tx_buf_pointer;               /// current send data pointer

    uint8_t* rx_buf;
    uint32_t rx_num;                       /// total transmit number
    uint32_t rx_buf_pointer;               /// current receive data pointer

    uint32_t cmpl_count;                   /// complete transmit count

    uint8_t  slave_read_mid_buf[MAX_XFER_SZ];     /// only for slave read
    uint32_t slave_read_mid_buf_pointer;          /// slave read buffer pointer
    uint32_t slave_read_last_rx_data_count;
} I2C_TRANSFER_INFO;

typedef struct _I2C_INFO
{
    CSK_I2C_SignalEvent_t cb_event;

    volatile I2C_DRIVER_STATE state;
    CSK_POWER_STATE pwr_state;

    // Transfer information for TX/RX
    I2C_TRANSFER_INFO trans_info;

    uint32_t slave_address;

    volatile CSK_I2C_STATUS status;

    uint8_t fifo_depth;
    uint8_t inter_en;   // 0: enable dma
                        // 1: enable interrupt
    void* workspace;
} I2C_INFO;

// I2C DMA
#define _I2C_TX_DMA_WIDTH      DMA_WIDTH_BYTE
#define _I2C_TX_DMA_BSIZE      DMA_BSIZE_1

#define _I2C_RX_DMA_WIDTH      DMA_WIDTH_BYTE
#define _I2C_RX_DMA_BSIZE      DMA_BSIZE_1

typedef struct _I2C_DMA
{
    uint8_t channel;       // DMA Channel
    uint8_t reqsel;        // DMA request selection
    DMA_SignalEvent_t cb_event;      // DMA Event callback
} I2C_DMA;

// I2C Resource Configuration
typedef const struct
{
    I2C_RegDef* reg;                // I2C register interface
    uint32_t irq_num;
    void (*irq_handler)(void);
    I2C_DMA* dma_tx;
    I2C_DMA* dma_rx;
    I2C_INFO* info;               // Run-Time control information
}I2C_RESOURCES;

#endif /* __I2C_AE210P_H */
