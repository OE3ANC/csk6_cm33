/*
 * spi_venus.h
 *
 *  Created on: Jul 29, 2020
 *
 */

#ifndef __SPI_VENUS_H
#define __SPI_VENUS_H

#include "dma.h"
#include "venus_cp.h"
#include "Driver_SPI.h"

#define IRQ_SPI1_VECTOR SUB_IRQ_SPI1_VECTOR

/* SPI transfer format register */
#define SPI_CPHA                      (1UL << 0)
#define SPI_CPOL                      (1UL << 1)
#define SPI_SLAVE                     (1UL << 2)
#define SPI_LSB                       (1UL << 3)
#define SPI_MERGE                     (1UL << 7)
#define DATA_BITS_MASK                (0x1F << 8)
#define DATA_BITS(data_bits)          (((data_bits) - 1) << 8)
#define CPHA0_FAST_MODE               (1UL << 31) // Added in 6001 MP version

/* Max RD/WR transfer count each time, limited by SPI transfer control register */
#define MAX_TRANCNT                   512

/* SPI transfer control register */
// RD/WR transfer count
#define RD_TRANCNT(num)               (((num) - 1) << 0)
#define WR_TRANCNT(num)               (((num) - 1) << 12)

/* SPI Slave Data Count Register (0x64) */
// RD/WR transferred count, 10 bits in new version (originally 9 bits)
//#define SLV_RDCNT(spi)               ((spi)->reg->SLVDATACNT & 0x1FF)
//#define SLV_WRCNT(spi)               (((spi)->reg->SLVDATACNT >> 16) & 0x1FF)
#define SLV_RDCNT(spi)               ((spi)->reg->SLVDATACNT & 0x3FF)
#define SLV_WRCNT(spi)               (((spi)->reg->SLVDATACNT >> 16) & 0x3FF)
#define CLR_SLV_RDCNT(spi)           (spi)->reg->SLVDATACNT |= (1 << 15) // bit[15] = 1 to clear RCnt
#define CLR_SLV_WRCNT(spi)           (spi)->reg->SLVDATACNT |= (1 << 31) // bit[31] = 1 to clear WCnt

// SPI transfer mode
#define SPI_TRANSMODE_WRnRD           (0x0 << 24)
#define SPI_TRANSMODE_WRONLY          (0x1 << 24)
#define SPI_TRANSMODE_RDONLY          (0x2 << 24)
#define SPI_TRANSMODE_WR_RD           (0x3 << 24)
#define SPI_TRANSMODE_RD_WR           (0x4 << 24)
#define SPI_TRANSMODE_WR_DMY_RD       (0x5 << 24)
#define SPI_TRANSMODE_RD_DMY_WR       (0x6 << 24)
#define SPI_TRANSMODE_NONEDATA        (0x7 << 24)
#define SPI_TRANSMODE_DMY_WR          (0x8 << 24)
#define SPI_TRANSMODE_DMY_RD          (0x9 << 24)

#define SPI_TRANSMODE_CMD_EN          (0x1 << 30)
#define SPI_TRANSMODE_ADDR_EN         (0x1 << 29)

/* SPI control register */
#define SPIRST                        (1UL << 0)
#define RXFIFORST                     (1UL << 1)
#define TXFIFORST                     (1UL << 2)
#define RXDMAEN                       (1UL << 3)
#define TXDMAEN                       (1UL << 4)
#define RXTHRES(num)                  ((num) << 8)
#define TXTHRES(num)                  ((num) << 16)

#define THRES_MASK                    (0x1f)
#define RXTHRES_OFFSET                (8)
#define TXTHRES_OFFSET                (16)

/* SPI interrupt status register */
/* SPI interrupt enable register */
#define SPI_RXFIFOORINT               (1UL << 0)
#define SPI_TXFIFOURINT               (1UL << 1)
#define SPI_RXFIFOINT                 (1UL << 2)
#define SPI_TXFIFOINT                 (1UL << 3)
#define SPI_ENDINT                    (1UL << 4)
#define SPI_SLVCMD                    (1UL << 5)

/* SPI Slave Status Register */
#define SLVST_READY         (1UL << 16) //Ready for data transaction
#define SLVST_OVER_RUN      (1UL << 17) //Data overrun, W1C
#define SLVST_UNDER_RUN     (1UL << 18) //Data underrun, W1C

// SPI flags
#define SPI_FLAG_INITIALIZED          (1UL << 0)     // SPI initialized
#define SPI_FLAG_POWERED              (1UL << 1)     // SPI powered on
#define SPI_FLAG_CONFIGURED           (1UL << 2)     // SPI configured
//#define SPI_FLAG_DATA_LOST            (1UL << 3)     // SPI data lost occurred
//#define SPI_FLAG_MODE_FAULT           (1UL << 4)     // SPI mode fault occurred

/*****************************************************************************
 * SPI Registers Map on VENUS
 ****************************************************************************/
typedef struct {
    __I  unsigned int IDREV;                /* 0x00 ID and revision register */
         unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
    __IO unsigned int TRANSFMT;             /* 0x10 SPI transfer format register */
    __IO unsigned int DIRECTIO;             /* 0x14 SPI direct IO control register */
         unsigned int RESERVED1[2];         /* 0x18 ~ 0x1c Reserved */
    __IO unsigned int TRANSCTRL;            /* 0x20 SPI transfer control register */
    __IO unsigned int CMD;                  /* 0x24 SPI command register */
    __IO unsigned int ADDR;                 /* 0x28 SPI address register */
    __IO unsigned int DATA;                 /* 0x2c SPI data register */
    __IO unsigned int CTRL;                 /* 0x30 SPI control register */
    __I  unsigned int STATUS;               /* 0x34 SPI status register */
    __IO unsigned int INTREN;               /* 0x38 SPI interrupt enable register */
    __O  unsigned int INTRST;               /* 0x3c SPI interrupt status register */
    __IO unsigned int TIMING;               /* 0x40 SPI interface timing register */
         unsigned int RESERVED2[3];         /* 0x44 ~ 0x4c Reserved */
    __IO unsigned int MEMCTRL;              /* 0x50 SPI memory access control register */
         unsigned int RESERVED3[3];         /* 0x54 ~ 0x5c Reserved */
    __IO unsigned int SLVST;                /* 0x60 SPI slave status register */
    __IO unsigned int SLVDATACNT;           /* 0x64 SPI slave data count register */
         unsigned int RESERVED4[5];         /* 0x68 ~ 0x78 Reserved */
    __I  unsigned int CONFIG;               /* 0x7c Configuration register */
} CSK_SPI_RegDef;

#define CSK_SPI0             ((CSK_SPI_RegDef *)  SPI0_BASE)
#define CSK_SPI1             ((CSK_SPI_RegDef *)  SPI1_BASE)
//#define CSK_SPI2             ((CSK_SPI_RegDef *)  SPI2_BASE)

//SPI TX/RX FIFO depth on VENUS
#define CSK_SPI_TXFIFO_DEPTH    8 // words
#define CSK_SPI_RXFIFO_DEPTH    8 // words

//current entries in SPI TX/RX FIFO
#define SPI_TXFIFO_ENTRIES(spi)     (((spi)->reg->STATUS >> 16) & 0x1f)
#define SPI_RXFIFO_ENTRIES(spi)     (((spi)->reg->STATUS >> 8) & 0x1f)

#define SPI_TXFIFO_FULL(spi)     (((spi)->reg->STATUS >> 23) & 0x1)
#define SPI_TXFIFO_EMPTY(spi)    (((spi)->reg->STATUS >> 22) & 0x1)
#define SPI_RXFIFO_FULL(spi)     (((spi)->reg->STATUS >> 15) & 0x1)
#define SPI_RXFIFO_EMPTY(spi)    (((spi)->reg->STATUS >> 14) & 0x1)


/*****************************************************************************
 * SPI clock in cmn_sysctrl_reg_venus.h
 ****************************************************************************/
struct SYSCFG_REG_SPI_CLK_CFG_16BITS
{
    volatile uint16_t SEL_SPI_CLK                  : 1; // bit 0~0
    volatile uint16_t RESV_1_2                     : 2; // bit 1~2
    volatile uint16_t ENA_SPI_CLK                  : 1; // bit 3~3
    volatile uint16_t DIV_SPI_CLK_M                : 3; // bit 4~6
    volatile uint16_t DIV_SPI_CLK_N                : 3; // bit 7~9
    volatile uint16_t DIV_SPI_CLK_LD               : 1; // bit 10~10
    volatile uint16_t RESV_11_15                   : 5; // bit 11~15
};

typedef union {
    volatile uint16_t                               all;
    struct SYSCFG_REG_SPI_CLK_CFG_16BITS            bit;
} SYSCFG_SPI_CLK_REG16;

#define SPI_CLK_SEL_MASK          (0x1 << 0) // bit[0]
#define SPI_CLK_SEL_XTAL          (0x0 << 0) // default
#define SPI_CLK_SEL_USBPLL        (0x1 << 0)
#define SPI_CLK_ENA_MASK          (0x1 << 3) // bit[3]
#define SPI_CLK_DISABLE           (0x0 << 3)
#define SPI_CLK_ENABLE            (0x1 << 3)
#define SPI_CLK_M_MASK            (0x7 << 4) // bit[6:4], 0x70
#define SPI_CLK_M(val)            (((val) & 0x7) << 4)
#define SPI_CLK_M_MAX             7
#define SPI_CLK_N_MASK            (0x7 << 7) // bit[9:7], 0x380
#define SPI_CLK_N(val)            (((val) & 0x7) << 7)
#define SPI_CLK_N_MAX             7
#define SPI_CLK_LD_MASK           (0x1 << 10) // bit[10]
#define SPI_CLK_LD                (0x1 << 10)

#define CSK_SPI0_CLK             ((SYSCFG_SPI_CLK_REG16 *)(CMN_SYSCTRL_BASE + 0x44))
#define CSK_SPI1_CLK             ((SYSCFG_SPI_CLK_REG16 *)(CMN_SYSCTRL_BASE + 0x46))


// SPI transfer operation
#define SPI_SEND                      0x0
#define SPI_RECEIVE                   0x1
#define SPI_TRANSFER                  0x2

// SPI transfer information (Run-time)
typedef struct _SPI_TRANSFER_INFO
{
    void *rx_buf;         // pointer to in data buffer
    const void *tx_buf;   // pointer to out data buffer
    uint32_t rx_cnt;      // number of data received (see TRANSFMT register bit[12:8])
    uint32_t tx_cnt;      // number of data sent (see TRANSFMT register bit[12:8])
    uint32_t req_rx_cnt;  // number of data requested to receive (see TRANSFMT register bit[12:8])
    uint32_t req_tx_cnt;  // number of data requested to send (see TRANSFMT register bit[12:8])
    uint8_t  dma_tx_done; // whether dma send is done, 1: done, 0: not done
    uint8_t  dma_rx_done; // whether dma receive is done, 1: done, 0: not done
    uint8_t  cur_op;      // current operation, send, receive or both(transfer)?
} SPI_TRANSFER_INFO;

// SPI TX/RX IO & SPI Mode
typedef struct _SPI_TXRX_MODE {
    uint8_t mode :4;    // SPI Mode, Master or Slave?
    uint8_t tx_dma:1;   // support TX DMA?
    uint8_t tx_pio:1;   // support TX PIO?
    uint8_t rx_dma:1;   // support RX DMA?
    uint8_t rx_pio:1;   // support RX PIO?
} SPI_TXRX_MODE;

#define TX_DMA(txrx_mode)         (((txrx_mode) & CSK_SPI_TXIO_DMA) == CSK_SPI_TXIO_DMA)
#define TX_PIO(txrx_mode)         (((txrx_mode) & CSK_SPI_TXIO_PIO) == CSK_SPI_TXIO_PIO)
#define TX_DMA_ONLY(txrx_mode)    (((txrx_mode) & CSK_SPI_TXIO_BOTH) == CSK_SPI_TXIO_DMA)

#define RX_DMA(txrx_mode)         (((txrx_mode) & CSK_SPI_RXIO_DMA) == CSK_SPI_RXIO_DMA)
#define RX_PIO(txrx_mode)         (((txrx_mode) & CSK_SPI_RXIO_PIO) == CSK_SPI_RXIO_PIO)
#define RX_DMA_ONLY(txrx_mode)    (((txrx_mode) & CSK_SPI_RXIO_BOTH) == CSK_SPI_RXIO_DMA)


// SPI information (Run-time)
typedef struct _SPI_INFO
{
    CSK_SPI_SignalEvent_t cb_event;    // event callback
    uint32_t usr_param; // user parameter of event callback

    uint8_t flags;            // SPI driver flags
    uint8_t txrx_mode;        // SPI TX/RX & mode
    uint8_t tx_dyn_dma_ch;    // dynamically allocated DMA channel for transmit
    uint8_t rx_dyn_dma_ch;    // dynamically allocated DMA channel for receive
    uint8_t data_bits;        // length of each data unit in bits (see DataLen of TRANSFMT)
    uint8_t dma_width_shift;  // shift of dma width, 0: byte, 1: halfword, 2: word
    uint8_t tx_bsize_shift;   // burst size shift for TX DMA
    uint8_t rx_bsize_shift;   // burst size shift for RX DMA

    CSK_SPI_STATUS status;        // SPI status flags
    SPI_TRANSFER_INFO xfer;   // SPI transfer information

} SPI_INFO;

// SPI DMA
typedef const struct _SPI_DMA
{
    uint8_t channel;    // DMA channel
    uint8_t reqsel;     // DMA request selection
    DMA_SignalEvent_t cb_event; // DMA event callback
    uint32_t usr_param; // user parameter of DMA callback
} SPI_DMA;

// SPI device structure definitions
typedef struct
{
    CSK_SPI_RegDef *reg;  // pointer to SPI peripheral
    SYSCFG_SPI_CLK_REG16 *ext_clk; // pointer to external SPI clock config.
    uint32_t txfifo_depth;  // TX FIFO depth, in words
    uint32_t rxfifo_depth;  // RX FIFO depth, in words
    uint32_t irq_num;      // SPI IRQ number
    //void (*irq_handler)(void);
    ISR irq_handler;

    SPI_DMA dma_tx;      // SPI TX DMA
    SPI_DMA dma_rx;      // SPI RX DMA

    SPI_INFO *info;        // SPI run-time information
} SPI_DEV;

#endif /* __SPI_VENUS_H */
