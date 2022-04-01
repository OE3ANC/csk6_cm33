#ifndef __I2C_AE210P_H
#define __I2C_AE210P_H

#include "cmn_sysctrl_reg_venus.h"
#include "dma.h"
#include "i2c_reg_venus.h"

#include "venus_ap.h"
#include "Driver_I2C.h"

// n-byte FIFO, where n is the number 2, 4, 8 or 16.
#define VENUS_IIC_FIFO_DEPTH_N      (16)

#define MAX_XFER_SZ                 (256)           // 256 bytes

// I2C mode: standard-mode, fast-mode, fast-mode-plus, all based on APB-clk(40MHZ)
#define SETUP_T_SUDAT_STD           (0x9)           // setup time = (4 + 2 + 4) * 25ns = 250ns
#define SETUP_T_SP_STD              (0x2)           // spikes time = 2 * 25ns = 50ns
#define SETUP_T_HDDAT_STD           (0x9)           // hold time = (4 + 2 + 6) * 25ns = 300ns
#define SETUP_T_SCL_RATIO_STD       (0x0)           // ratio=1, (4 + 2 + 182) * 25ns >= 4000ns, for I2C SCL clock
// The T_SCLHi value must be greater than T_SP and T_HDDAT values.
#define SETUP_T_SCLHI_STD           (229)           // (4 + 2 + 182 * 1) * 25ns >= 4700ns

#define SETUP_T_SUDAT_FAST          (0x4)           // setup time = (4 + 2 + 0) * 25ns = 150ns
#define SETUP_T_SP_FAST             (0x2)           // spikes time = 2 * 25ns = 50ns
#define SETUP_T_HDDAT_FAST          (0x9)           // hold time = (4 + 2 + 6) * 25ns = 300ns
#define SETUP_T_SCL_RATIO_FAST      (0x1)           // ratio=2, (4 + 2 + 23) * 25ns >= 600ns, for I2C SCL clock
// The T_SCLHi value must be greater than T_SP and T_HDDAT values.
#define SETUP_T_SCLHI_FAST          (32)            // (4 + 2 + 23 * 2) * 25ns >= 1300ns

#define SETUP_T_SUDAT_FAST_P        (0x4)           // setup time = (4 + 2 + 0) * 25ns = 150ns
#define SETUP_T_SP_FAST_P           (0x2)           // spikes time = 2 * 25ns = 50ns
#define SETUP_T_HDDAT_FAST_P        (0x9)           // hold time = (4 + 2 + 0) * 25ns = 150ns
#define SETUP_T_SCL_RATIO_FAST_P    (0x1)           // ratio=2, (4 + 2 + 7) * 25ns >= 260ns, for I2C SCL clock
// The T_SCLHi value must be greater than T_SP and T_HDDAT values.
#define SETUP_T_SCLHI_FAST_P        (10)             // (4 + 2 + 7 * 2) * 25ns >= 500ns

#define VENUS_I2C0              ((I2C_RegDef *)  I2C0_BASE)
#define VENUS_I2C1              ((I2C_RegDef *)  I2C1_BASE)

#define VENUS_I2C0_DMA_TX_CH        (2)
#define VENUS_I2C0_DMA_RX_CH        (3)

#define VENUS_I2C1_DMA_TX_CH        (4)
#define VENUS_I2C1_DMA_RX_CH        (5)

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
    uint32_t busy :1;        ///< Busy flag
    uint32_t mode :1;        ///< Mode: 0=Slave, 1=Master
    uint32_t direction :1;        ///< Direction: 0=Transmitter, 1=Receiver
    uint32_t general_call :1; ///< General Call indication (cleared on start of next Slave operation)
    uint32_t arbitration_lost :1; ///< Master lost arbitration (cleared on start of next Master operation)
    uint32_t bus_error :1; ///< Bus error detected (cleared on start of next Master/Slave operation)
} CSK_I2C_STATUS;

typedef struct _I2C_INFO
{
    CSK_I2C_SignalEvent_t Sig_Evt_CB;
    volatile I2C_DRIVER_STATE Driver_State;
    CSK_POWER_STATE Pwr_State;
    uint8_t* middleware_rx_buf;
    uint8_t* middleware_tx_buf;
    /* flags for flow control */
    // salve mode only
    uint32_t last_rx_data_count;
    // Xfer_Data_Rd_Buf[] for middleware read out, salve mode only
    uint32_t middleware_rx_ptr;
    // salve mode only
    uint8_t nack_assert;
    uint32_t FIFO_Depth;
    uint32_t Slave_Addr;
    uint32_t Xfer_Wt_Num;
    uint32_t Xfer_Rd_Num;
    // write pointer
    uint32_t Xfered_Data_Wt_Ptr;
    // read pointer
    uint32_t Xfered_Data_Rd_Ptr;
    // read pointer overwrite
    uint32_t Xfered_Data_Rd_Ptr_Ow;
    uint8_t Xfer_Data_Rd_Buf[MAX_XFER_SZ];
    uint32_t Slave_Rx_Cmpl_Ctrl_Reg_Val;
    volatile CSK_I2C_STATUS Status;
    uint32_t Xfer_Cmpl_Count;

    // 1: enable dma
    // 0: enable interrupt
    uint8_t DMA_EN;
    void* workspace;
} I2C_INFO;

// I2C DMA
#define CSK_I2C_TX_DMA_WIDTH      DMA_WIDTH_BYTE
#define CSK_I2C_TX_DMA_BSIZE      DMA_BSIZE_1

#define CSK_I2C_RX_DMA_WIDTH      DMA_WIDTH_BYTE
#define CSK_I2C_RX_DMA_BSIZE      DMA_BSIZE_1

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
