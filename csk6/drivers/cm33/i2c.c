#include "i2c.h"

#define CSK_I2C_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

// Driver Version
static const CSK_DRIVER_VERSION DriverVersion =
{
        CSK_I2C_API_VERSION,
        CSK_I2C_DRV_VERSION
};

/********************************
 *          IIC0
 * *****************************/
// I2C0 Control Information
static I2C_INFO I2C0_Info = { 0 };

static void i2c0_dma_tx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static I2C_DMA i2c0_dma_tx =
{
    I2C0_DMA_TX_CH,
    DMA_HSID_I2C0,
    i2c0_dma_tx_event
};

static void i2c0_dma_rx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static I2C_DMA i2c0_dma_rx =
{
    I2C0_DMA_RX_CH,
    DMA_HSID_I2C0,
    i2c0_dma_rx_event
};

static void i2c0_irq_handler(void);
// I2C0 Resources
static I2C_RESOURCES i2c0_resources =
    {
    IP_I2C0,
    IRQ_I2C0_VECTOR,
    i2c0_irq_handler,
    &i2c0_dma_tx,
    &i2c0_dma_rx,
    &I2C0_Info
};

void* I2C0(void){
    return (void*)&i2c0_resources;
}


/********************************
 *          IIC1
 * *****************************/
// I2C0 Control Information
static I2C_INFO I2C1_Info = { 0 };

static void i2c1_dma_tx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static I2C_DMA i2c1_dma_tx =
{
    I2C1_DMA_TX_CH,
    DMA_HSID_I2C1,
    i2c1_dma_tx_event
};

static void i2c1_dma_rx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param);
static I2C_DMA i2c1_dma_rx =
{
    I2C1_DMA_RX_CH,
    DMA_HSID_I2C1,
    i2c1_dma_rx_event
};

static void i2c1_irq_handler(void);
// I2C0 Resources
static I2C_RESOURCES i2c1_resources =
    {
    IP_I2C1,
    IRQ_I2C1_VECTOR,
    i2c1_irq_handler,
    &i2c1_dma_tx,
    &i2c1_dma_rx,
    &I2C1_Info
};

void* I2C1(void){
    return (void*)&i2c1_resources;
}

#define CHECK_RESOURCES(res)  do{\
        if(res != &i2c0_resources && res != &i2c1_resources){\
            return CSK_DRIVER_ERROR_PARAMETER;\
        }\
}while(0)

static void
i2cx_master_fifo_write(I2C_RESOURCES* i2c);
static void
i2cx_slave_fifo_write(I2C_RESOURCES* i2c);
static void
i2cx_master_fifo_read(I2C_RESOURCES* i2c);
static void
i2cx_slave_fifo_read(I2C_RESOURCES* i2c, uint8_t is_fifo_full);

CSK_DRIVER_VERSION
I2C_GetVersion(void)
{
    return DriverVersion;
}

int32_t
I2C_Initialize(void* res, CSK_I2C_SignalEvent_t cb_event, void* workspace)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    if (i2c->info->state & I2C_DRV_INIT)
    {
        return CSK_DRIVER_OK;
    }

    i2c->info->cb_event = cb_event;
    i2c->info->workspace = workspace;

    i2c->info->state |= I2C_DRV_INIT;
    return CSK_DRIVER_OK;
}

int32_t
I2C_Uninitialize(void* res)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    i2c->info->cb_event = NULL;

    if(!i2c->info->inter_en){
        dma_uninitialize();
    }

    // clear & set driver state to none
    i2c->info->state = I2C_DRV_NONE;

    return CSK_DRIVER_OK;
}

int32_t
I2C_PowerControl(void* res, CSK_POWER_STATE state)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    i2c->info->pwr_state = state;

    switch (state)
    {
    case CSK_POWER_OFF:
        disable_IRQ(i2c->irq_num);

        // I2C reset controller
        i2c->reg->REG_CMD.bit.CMD = _CMD_RESET_I2C;

        // I2C disable
        i2c->reg->REG_SETUP.bit.IICEN = 0x0;

        // reset i2c & enable i2c clock
        if (i2c == &i2c0_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.I2C0_RESET = 0x1;
            IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C0_CLK = 0x0;
        }else if (i2c == &i2c1_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.I2C1_RESET = 0x1;
            IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C1_CLK = 0x0;
        }else {
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        i2c->info->state &= (~I2C_DRV_POWER);

        register_ISR(i2c->irq_num , NULL, NULL);

        break;

    case CSK_POWER_LOW:
        break;

    case CSK_POWER_FULL:

        // reset i2c & enable i2c clock
        if (i2c == &i2c0_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.I2C0_RESET = 0x1;
            IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C0_CLK = 0x1;
        }else if (i2c == &i2c1_resources){
            IP_SYSCTRL->REG_SW_RESET2.bit.I2C1_RESET = 0x1;
            IP_SYSCTRL->REG_MISC_CLK_CFG2.bit.ENA_I2C1_CLK = 0x1;
        }else {
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        // I2C query FIFO depth
        // read only FIFO size config
        switch (i2c->reg->REG_CFG.bit.FIFOSIZE)
        {
        case _FIFOSIZE_2_BYTES:
            i2c->info->fifo_depth = 2;
            break;
        case _FIFOSIZE_4_BYTES:
            i2c->info->fifo_depth = 4;
            break;
        case _FIFOSIZE_8_BYTES:
            i2c->info->fifo_depth = 8;
            break;
        case _FIFOSIZE_16_BYTES:
            i2c->info->fifo_depth = 16;
            break;
        }

        // I2C reset controller
        i2c->reg->REG_CMD.all = _CMD_RESET_I2C;

        // I2C setting: slave mode(default), FIFO(CPU) mode, 7-bit slave address, Ctrl enable
        i2c->reg->REG_SETUP.all = 0x0;

        // Configure slave setup parameter
        {
            uint32_t pclk = 1000000000 / CRM_GetApapbFreq();
            uint16_t sudat, sp, hddat;

            // I2C speed standard
            sp = STANDARD_MODE_SP_MAX / pclk;
            sudat = (STANDARD_MODE_SETUP_MIN / pclk) - (4 + sp);
            hddat = (STANDARD_MODE_HOLD_MIN / pclk) - (4 + sp);

            // apply
            i2c->reg->REG_SETUP.bit.T_HDDAT = hddat;
            i2c->reg->REG_SETUP.bit.T_SUDAT = sudat;
            i2c->reg->REG_SETUP.bit.T_SP = sp;
        }

        i2c->reg->REG_SETUP.bit.IICEN = 0x1;

        // I2C setting: enable completion interrupt & address hit interrupt
        // For slave device
        i2c->reg->REG_INTEN.all = (BITSET_0X014_CMPL | BITSET_0X014_ADDRHIT);

        // clear status
        i2c->info->status.busy = 0;
        // define mode => 0:slave / 1:master
        i2c->info->status.mode = 0;
        // define direction => 0:tx / 1:rx
        i2c->info->status.direction = 0;
        i2c->info->status.arbitration_lost = 0;
        i2c->info->status.bus_error = 0;

        i2c->info->state = I2C_DRV_POWER;

        register_ISR(i2c->irq_num, i2c->irq_handler, NULL);

        enable_IRQ(i2c->irq_num);
        break;

    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    return CSK_DRIVER_OK;
}

int32_t
I2C_MasterTransmit(void* res, uint32_t addr, const uint8_t* data, uint32_t num,
        bool xfer_pending)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    // max 10-bit address(0x3FF), null data or num is no payload for acknowledge polling
    // If no I2C payload, set Phase_data=0x0
    if (addr > 0x3FF)
    {
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // Transfer operation in progress, or slave stalled
    if (i2c->info->status.busy)
    {
        return CSK_DRIVER_ERROR_BUSY;
    }

    i2c->info->status.busy = 1;
    // define mode => 0:slave / 1:master
    i2c->info->status.mode = 1;
    // define direction => 0:tx / 1:rx
    i2c->info->status.direction = 0;
    i2c->info->status.arbitration_lost = 0;
    i2c->info->status.bus_error = 0;

    // clear & set driver state to master tx before issue transaction
    i2c->info->state = I2C_DRV_MASTER_TX;

    // I2C reset controller, including disable all I2C interrupts & clear fifo
    i2c->reg->REG_CMD.bit.CMD = _CMD_RESET_I2C;

    // I2C master, FIFO(CPU) mode, Ctrl enable
    if (i2c->info->inter_en)
    {
        // Interrupt mode
        i2c->reg->REG_SETUP.bit.DMAEN = 0x0;
    }
    else
    {
        i2c->reg->REG_SETUP.bit.DMAEN = 0x1;
    }

    i2c->reg->REG_SETUP.bit.MASTER = 0x1;
    i2c->reg->REG_SETUP.bit.IICEN = 0x1;

    // I2C phase start enable, phase addr enable, phase data enable, phase stop enable.
    // If I2C data transaction w/o I2C payload, remember to clear data bit.
    // xfer_pending: Transfer operation is pending - Stop condition will not be generated.
    // The bus is busy when a START condition is on bus and it ends when a STOP condition is seen.
    // 10-bit slave address must set STOP bit.
    // I2C direction : master tx, set xfer data count.
    {
        uint32_t temp_para = 0;
        temp_para = (BITSET_0X024_PHASE_START | BITSET_0X024_PHASE_ADDR | (!xfer_pending << 9)
                    | (num & BITSET_0X024_DATACNT));

        if(num){
            temp_para |= BITSET_0X024_PHASE_DATA;
        }

        i2c->reg->REG_CTRL.all = temp_para;
    }

    i2c->info->slave_address = addr;

    i2c->info->trans_info.tx_num = num;
    i2c->info->trans_info.cmpl_count = 0;
    i2c->info->trans_info.tx_buf_pointer = 0;
    i2c->info->trans_info.tx_buf = (uint8_t*)data;

    // I2C slave address, general call address = 0x0(7-bit or 10-bit)
    i2c->reg->REG_ADDR.all = i2c->info->slave_address & BITSET_0X01C_ADDR_MASK;

    {
        uint32_t temp_para = (BITSET_0X014_CMPL | BITSET_0X014_ARBLOS);

        if (i2c->info->inter_en)
        {
            // I2C write a patch of data(FIFO_Depth) to FIFO,
            // it will be consumed empty if data is actually issued on I2C bus,
            // currently FIFO is not empty, will not trigger FIFO_EMPTY interrupt
            i2cx_master_fifo_write(i2c);

            if (num){
                // enable
                temp_para |= BITSET_0X014_FIFOEMPTY;
            } else {
                // disable
                temp_para &= (~ BITSET_0X014_FIFOEMPTY);
            }
        } else {
            int32_t stat;

            dma_channel_select(
                &i2c->dma_tx->channel,
                i2c->dma_tx->cb_event,
                0,
                DMA_CACHE_SYNC_SRC);
            if (i2c->dma_tx->channel == DMA_CHANNEL_ANY) {
                return CSK_DRIVER_ERROR;
            }

            stat = dma_channel_configure (i2c->dma_tx->channel,
                        (uint32_t) i2c->info->trans_info.tx_buf,
                        (uint32_t) (&(i2c->reg->REG_DATA.all)),
                        i2c->info->trans_info.tx_num,
                        DMA_CH_CTLL_DST_WIDTH(_I2C_TX_DMA_WIDTH) | DMA_CH_CTLL_SRC_WIDTH(_I2C_TX_DMA_WIDTH) |\
                      DMA_CH_CTLL_DST_BSIZE(_I2C_TX_DMA_BSIZE) | DMA_CH_CTLL_SRC_BSIZE(_I2C_TX_DMA_BSIZE) |\
                      DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |\
                      DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                      DMA_CH_CFGL_CH_PRIOR(1), // config_low
                      DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(i2c->dma_tx->reqsel), // config_high
                      0, 0);

            if(stat == -1){
                return CSK_DRIVER_ERROR;
            }
        }

        i2c->reg->REG_INTEN.all = temp_para;
    }

    // I2C Write 0x1 to the Command register to issue the transaction
    i2c->reg->REG_CMD.bit.CMD = _CMD_ISSUE_TRANSACTION;

    return CSK_DRIVER_OK;
}

int32_t
I2C_MasterReceive(void* res, uint32_t addr, uint8_t* data, uint32_t num,
        bool xfer_pending)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    // max 10-bit address(0x3FF), null data or num is no payload for acknowledge polling
    // If no I2C payload, set Phase_data=0x0
    if (addr > 0x3FF)
    {
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // Transfer operation in progress, or slave stalled
    if (i2c->info->status.busy)
    {
        return CSK_DRIVER_ERROR_BUSY;
    }

    i2c->info->status.busy = 1;
    // define mode => 0:slave / 1:master
    i2c->info->status.mode = 1;
    // define direction => 0:tx / 1:rx
    i2c->info->status.direction = 1;
    i2c->info->status.arbitration_lost = 0;
    i2c->info->status.bus_error = 0;

    //clear & set driver state to master rx before issue transaction
    i2c->info->state = I2C_DRV_MASTER_RX;

    // I2C reset controller, including disable all I2C interrupts & clear fifo
    i2c->reg->REG_CMD.bit.CMD = _CMD_RESET_I2C;

    // I2C master, FIFO(CPU) mode, Ctrl enable
    if (i2c->info->inter_en)
    {
        // Interrupt mode
        i2c->reg->REG_SETUP.bit.DMAEN = 0x0;
    }
    else
    {
        i2c->reg->REG_SETUP.bit.DMAEN = 0x1;
    }

    i2c->reg->REG_SETUP.bit.MASTER = 0x1;
    i2c->reg->REG_SETUP.bit.IICEN = 0x1;

    // I2C phase start enable, phase addr enable, phase data enable, phase stop enable.
    // If I2C data transaction w/o I2C payload, remember to clear data bit.
    // xfer_pending: Transfer operation is pending - Stop condition will not be generated.
    // The bus is busy when a START condition is on bus and it ends when a STOP condition is seen.
    // 10-bit slave address must set STOP bit.
    // I2C direction : master rx, set xfer data count.
    {
        uint32_t temp_para = 0;
        temp_para = (BITSET_0X024_PHASE_START | BITSET_0X024_PHASE_ADDR | (!xfer_pending << 9)
                    | (num & BITSET_0X024_DATACNT) | BITSET_0X024_DIR);

        if(num){
            temp_para |= BITSET_0X024_PHASE_DATA;
        }

        i2c->reg->REG_CTRL.all = temp_para;
    }

    i2c->info->slave_address = addr;

    i2c->info->trans_info.rx_num = num;
    i2c->info->trans_info.cmpl_count = 0;
    i2c->info->trans_info.rx_buf_pointer = 0;
    i2c->info->trans_info.rx_buf = (uint8_t*)data;

    // I2C slave address, general call address = 0x0(7-bit or 10-bit)
    i2c->reg->REG_ADDR.all = i2c->info->slave_address & BITSET_0X01C_ADDR_MASK;

    // I2C Enable the Completion Interrupt, Enable the FIFO Full Interrupt
    // I2C Enable the Arbitration Lose Interrupt, master mode only
    {
        uint32_t temp_para = (BITSET_0X014_CMPL | BITSET_0X014_ARBLOS);

        if (i2c->info->inter_en)
        {
            temp_para |= BITSET_0X014_FIFOFULL;
        } else {
            int32_t stat;

            dma_channel_select(
                &i2c->dma_rx->channel,
                i2c->dma_rx->cb_event,
                0,
                DMA_CACHE_SYNC_DST);
            if (i2c->dma_rx->channel == DMA_CHANNEL_ANY) {
                return CSK_DRIVER_ERROR;
            }

            stat = dma_channel_configure (i2c->dma_rx->channel,
                        (uint32_t) (&(i2c->reg->REG_DATA.all)),
                        (uint32_t) i2c->info->trans_info.rx_buf,
                        i2c->info->trans_info.rx_num,
                        DMA_CH_CTLL_DST_WIDTH(_I2C_RX_DMA_WIDTH) | DMA_CH_CTLL_SRC_WIDTH(_I2C_RX_DMA_WIDTH) |\
                      DMA_CH_CTLL_DST_BSIZE(_I2C_RX_DMA_BSIZE) | DMA_CH_CTLL_SRC_BSIZE(_I2C_RX_DMA_BSIZE) |\
                      DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |\
                      DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                      DMA_CH_CFGL_CH_PRIOR(1), // config_low
                      DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(i2c->dma_rx->reqsel), // config_high
                      0, 0);

            if(stat == -1){
                return CSK_DRIVER_ERROR;
            }
        }

        i2c->reg->REG_INTEN.all = temp_para;
    }

    // I2C Write 0x1 to the Command register to issue the transaction
    i2c->reg->REG_CMD.bit.CMD = _CMD_ISSUE_TRANSACTION;

    return CSK_DRIVER_OK;
}

// slave mode unknow how many bytes to tx, so tx 1 byte each time until complete or nack_assert
int32_t
I2C_SlaveTransmit(void* res, const uint8_t* data, uint32_t num)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // Transfer operation in progress, or slave stalled
    if (i2c->info->status.busy)
    {
        return CSK_DRIVER_ERROR_BUSY;
    }

    i2c->info->status.busy = 1;
    // define mode => 0:slave / 1:master
    i2c->info->status.mode = 0;
    // define direction => 0:tx / 1:rx
    i2c->info->status.direction = 0;
    i2c->info->status.arbitration_lost = 0;
    i2c->info->status.bus_error = 0;

    // clear & set driver state to slave tx before issue transaction
    i2c->info->state = I2C_DRV_SLAVE_TX;

    // I2C xfer data count
    // If DMA is not enabled, DataCnt is the number of
    // bytes transmitted/received from the bus master.
    // It is reset to 0 when the controller is addressed
    // and then increased by one for each byte of data
    // transmitted/received
    i2c->info->trans_info.tx_num = num;
    i2c->info->trans_info.cmpl_count = 0;
    i2c->info->trans_info.tx_buf_pointer = 0;
    i2c->info->trans_info.tx_buf = (uint8_t*)data;

    i2cx_slave_fifo_write(i2c);

    return CSK_DRIVER_OK;
}

// slave mode unknow how many bytes to rx, so rx fifo-full byte each time until complete
int32_t
I2C_SlaveReceive(void* res, uint8_t* data, uint32_t num)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    // since middleware just read data from Xfer_Data_Rd_Buf,
    // no set busy flag, driver slave rx is independent of
    // middleware slave rx, if set busy flag will affect
    // slave tx behavior
    // define mode => 0:slave / 1:master
    i2c->info->status.mode = 0;
    // define direction => 0:tx / 1:rx
    i2c->info->status.direction = 1;
    i2c->info->status.bus_error = 0;

    // I2C xfer data count
    // If DMA is not enabled, DataCnt is the number of
    // bytes transmitted/received from the bus master.
    // It is reset to 0 when the controller is addressed
    // and then increased by one for each byte of data
    // transmitted/received

    // no I2C reset controller, no I2C clear fifo since middleware just read data from Xfer_Data_Rd_Buf
    // w/ minimal change I2C HW setting

    // Xfer_Data_Rd_Buf already read the data from hw fifo and keep,
    // currently middleware able to take from the buffer */
    // error hit
    if (num > MAX_XFER_SZ)
    {
        return CSK_DRIVER_ERROR_PARAMETER;
    }
    __builtin_memcpy (data, &i2c->info->trans_info.slave_read_mid_buf[i2c->info->trans_info.slave_read_mid_buf_pointer], num);

    i2c->info->trans_info.slave_read_mid_buf_pointer += num;

    return CSK_DRIVER_OK;
}

int32_t
I2C_GetDataCount(void* res)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    return (i2c->info->trans_info.cmpl_count);
}

int32_t
I2C_Control(void* res, uint32_t control, uint32_t arg0)
{
    CHECK_RESOURCES(res);

    I2C_RESOURCES* i2c = (I2C_RESOURCES*)res;

    int32_t status = CSK_DRIVER_OK;

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    switch (control)
    {
    // middleware use control code
    case CSK_I2C_OWN_ADDRESS:
        if (arg0 & CSK_I2C_ADDRESS_10BIT)
        {
            // I2C 10-bit slave address
            i2c->reg->REG_SETUP.bit.ADDRESSING = 0x1;
        }
        else
        {
            // I2C 7-bit slave address
            i2c->reg->REG_SETUP.bit.ADDRESSING = 0x0;
        }

        // I2C slave address, general call address = 0x0(7-bit or 10-bit)
        i2c->reg->REG_ADDR.bit.ADDR = (arg0 & (BITSET_0X01C_ADDR_MASK));
        break;
    case CSK_I2C_BUS_SPEED:
    {
        uint32_t temp_para;
        temp_para = i2c->reg->REG_SETUP.all;
        // clear previous setting
        temp_para &= (~(BITSET_0X02C_T_SUDAT | BITSET_0X02C_T_SP | BITSET_0X02C_T_HDDAT
                    | BITSET_0X02C_T_SCLRATIO | BITSET_0X02C_T_SCLHI));

        uint32_t pclk = 1000000000 / CRM_GetApapbFreq();
        uint16_t sudat, sp, hddat, ratio, sclhi = 0;

        switch (arg0)
        {
        case CSK_I2C_BUS_SPEED_STANDARD:
            // I2C speed standard
            sp = STANDARD_MODE_SP_MAX / pclk;
            sudat = (STANDARD_MODE_SETUP_MIN / pclk) - (4 + sp);
            hddat = (STANDARD_MODE_HOLD_MIN / pclk) - (4 + sp);
            sclhi = (STANDARD_MODE_HIGH_MIN / pclk) - (4 + sp);
            ratio = STANDARD_MODE_RADIO_FIX - 1;
            break;
        case CSK_I2C_BUS_SPEED_FAST:
            // I2C speed fast
            sp = FAST_MODE_SP_MAX / pclk;
            sudat = (FAST_MODE_SETUP_MIN / pclk) - (4 + sp);
            hddat = (FAST_MODE_HOLD_MIN / pclk) - (4 + sp);
            sclhi = (FAST_MODE_HIGH_MIN / pclk) - (4 + sp);
            ratio = FAST_MODE_RADIO_FIX - 1;
            break;
        case CSK_I2C_BUS_SPEED_FAST_PLUS:
            // I2C speed fast plus
            sp = FAST_MODE_PLUS_SP_MAX / pclk;
            sudat = (FAST_MODE_PLUS_SETUP_MIN / pclk) - (4 + sp);
            hddat = (FAST_MODE_PLUS_HOLD_MIN / pclk) - (4 + sp);
            sclhi = (FAST_MODE_PLUS_HIGH_MIN / pclk) - (4 + sp);
            ratio = FAST_MODE_PLUS_RADIO_FIX - 1;
            break;
        default:
            return CSK_DRIVER_ERROR_UNSUPPORTED;
        }

        temp_para |= ((sudat << 24) |
                  (sp << 21) |
                  (hddat << 16) |
                  (ratio << 13) |
                  (sclhi << 4));

        // apply
        i2c->reg->REG_SETUP.all = temp_para;
        break;
    }
    case CSK_I2C_BUS_CLEAR:
        // I2C reset controller, including disable all I2C interrupts & clear fifo
        i2c->reg->REG_CMD.bit.CMD = _CMD_RESET_I2C;
        break;
    case CSK_I2C_ABORT_TRANSFER:
        // I2C reset controller ??
        // I2C reset controller, including disable all I2C interrupts & clear fifo

        // in dma mode
        // TODO get count
        if(!i2c->info->inter_en){
            if((i2c->info->state & I2C_DRV_MASTER_TX) || (i2c->info->state & I2C_DRV_SLAVE_TX)){
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_tx->channel) - i2c->reg->REG_CTRL.bit.DATACNT;
                dma_channel_disable(i2c->dma_tx->channel, 0);
            }else if((i2c->info->state & I2C_DRV_MASTER_RX) || (i2c->info->state & I2C_DRV_SLAVE_RX)){
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_rx->channel);
                dma_channel_disable(i2c->dma_rx->channel, 0);
            }
        } else {
            if(i2c->info->state & I2C_DRV_MASTER_TX){
                i2c->info->trans_info.cmpl_count = i2c->info->trans_info.tx_num - i2c->reg->REG_CTRL.bit.DATACNT;
            }
            else if (i2c->info->state & I2C_DRV_SLAVE_TX){
                i2c->info->trans_info.cmpl_count = i2c->reg->REG_CTRL.bit.DATACNT;
            }
            else if(i2c->info->state & I2C_DRV_MASTER_RX){
                i2c->info->trans_info.cmpl_count = i2c->info->trans_info.rx_num - i2c->reg->REG_CTRL.bit.DATACNT;
            }
            else if (i2c->info->state & I2C_DRV_SLAVE_RX){
                i2c->info->trans_info.cmpl_count = i2c->reg->REG_CTRL.bit.DATACNT;
            }

        }

        i2c->info->status.busy = 0;
        // reset i2c
        i2c->reg->REG_CMD.bit.CMD = _CMD_RESET_I2C;

        return status;

    case CSK_I2C_TRANSMIT_MODE:
        // arg0 = 1, means DMA mode
        if(arg0){
            i2c->info->inter_en = 0x0;
            i2c->reg->REG_SETUP.bit.DMAEN = 0x1;
            dma_initialize();
        // arg0 = 0, means Interrupt mode
        }else{
            i2c->info->inter_en = 0x1;
            i2c->reg->REG_SETUP.bit.DMAEN = 0x0;
        }
        break;
    default:
        return CSK_DRIVER_ERROR_UNSUPPORTED;
    }

    i2c->info->state = I2C_DRV_CFG_PARAM;

    return status;
}

static void
i2c_cmpl_handler(I2C_RESOURCES* i2c)
{
    // master mode
    if (i2c->reg->REG_SETUP.bit.MASTER == 1)
    {
        // I2C disable all Interrupts in the Interrupt Enable Register
        i2c->reg->REG_INTEN.all = (~ BITSET_0X014_INTEN_MASK);
    }
    else
    {
        // I2C no disable all Interrupts in the Interrupt Enable Register,
        // keep previous setting for slave tx */
    }

    // check the DataCnt field of the Control Register
    // to know if all data are successfully transmitted.
    // -> Master: The number of bytes to transmit/receive.
    // 0 means 256 bytes. DataCnt will be decreased by one
    // for each byte transmitted/received.
    if ((i2c->info->state & I2C_DRV_MASTER_TX) || (i2c->info->state & I2C_DRV_MASTER_RX)) {

        if (i2c->info->state & I2C_DRV_MASTER_TX) {
            if (i2c->info->inter_en)
            {
                i2c->info->trans_info.cmpl_count = i2c->info->trans_info.tx_num - i2c->reg->REG_CTRL.bit.DATACNT;
            }
            else
            {
                i2c->info->trans_info.tx_buf_pointer = dma_channel_get_count(i2c->dma_tx->channel);
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_tx->channel);
            }

            // clear & set driver state to master tx complete
            i2c->info->state = I2C_DRV_MASTER_TX_CMPL;

            // clear busy bit on i2c complete event as master dma/cpu tx
            i2c->info->status.busy = 0;
        }

        if (i2c->info->state & I2C_DRV_MASTER_RX) {
            if (i2c->info->inter_en) {
                i2cx_master_fifo_read(i2c);

                i2c->info->trans_info.cmpl_count = i2c->info->trans_info.rx_num - i2c->reg->REG_CTRL.bit.DATACNT;

                // clear & set driver state to master rx complete
                i2c->info->state = I2C_DRV_MASTER_RX_CMPL;

                // clear busy bit on i2c complete event as master cpu rx
                i2c->info->status.busy = 0;
            }
            else
            {
                i2c->info->trans_info.rx_buf_pointer = dma_channel_get_count(i2c->dma_rx->channel);
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_rx->channel);

                // clear & set driver state to master rx complete
                i2c->info->state = I2C_DRV_MASTER_RX_CMPL;
            }


        }
    }

    // check the DataCnt field of the Control Register
    // to know if all data are successfully transmitted.
    // -> Slave: the meaning of DataCnt depends on the
    // DMA mode:
    // If DMA is not enabled, DataCnt is the number of
    // bytes transmitted/received from the bus master.
    // It is reset to 0 when the controller is addressed
    // and then increased by one for each byte of data
    // transmitted/received.
    // If DMA is enabled, DataCnt is the number of
    // bytes to transmit/receive. It will not be reset to 0
    // when the slave is addressed and it will be
    // decreased by one for each byte of data
    // transmitted/received..
    if ((i2c->info->state & I2C_DRV_SLAVE_TX) || (i2c->info->state & I2C_DRV_SLAVE_RX)) {
        // I2C_FIFO mode
        if (i2c->info->inter_en) {
            if (i2c->info->state & I2C_DRV_SLAVE_TX) {
                // I2C Disable the Byte Transmit Interrupt in the Interrupt Enable Register
                i2c->reg->REG_INTEN.all &= (~ BITSET_0X014_BYTETRANS);

                // clear & set driver state to slave tx complete
                i2c->info->state = I2C_DRV_SLAVE_TX_CMPL;
            }

            if (i2c->info->state & I2C_DRV_SLAVE_RX) {
                i2cx_slave_fifo_read(i2c, 0);

                // I2C Disable the FIFO Full Interrupt in the Interrupt Enable Register
                i2c->reg->REG_INTEN.all &= (~ BITSET_0X014_FIFOFULL);

                // keypoint for middleware to query
                i2c->info->trans_info.cmpl_count = i2c->info->trans_info.slave_read_last_rx_data_count;

                // clear & set driver state to slave rx complete
                i2c->info->state = I2C_DRV_SLAVE_RX_CMPL;
            }

            // clear busy bit on i2c complete event as slave cpu tx/rx
            i2c->info->status.busy = 0;
        }
        // I2C_DMA mode
        else
        {
            if (i2c->info->state & I2C_DRV_SLAVE_TX)
            {
                // keypoint for middleware to query
                i2c->info->trans_info.tx_buf_pointer = dma_channel_get_count(i2c->dma_tx->channel);
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_tx->channel);

                dma_channel_disable(i2c->dma_tx->channel, 1);

                // clear & set driver state to slave tx complete
                i2c->info->state = I2C_DRV_SLAVE_TX_CMPL;

                // clear busy bit on i2c complete event as slave dma tx
                i2c->info->status.busy = 0;
            }

            if (i2c->info->state & I2C_DRV_SLAVE_RX)
            {
                // keypoint for middleware to query
                i2c->info->trans_info.cmpl_count = dma_channel_get_count(i2c->dma_rx->channel);

                // abort dma channel since MAX_XFER_SZ-read and expect complete in cmpl_handler
                dma_channel_disable(i2c->dma_rx->channel, 1);

                // clear & set driver state to slave rx complete
                i2c->info->state = I2C_DRV_SLAVE_RX_CMPL;

                // clear busy bit on i2c complete event as slave dma rx since read MAX_XFER_SZ
                i2c->info->status.busy = 0;
            }
        }

        // if the Completion Interrupt asserts, clear the FIFO and go next transaction.
        i2c->reg->REG_CMD.bit.CMD = _CMD_CLEAR_FIFO;
    }
}

static void
i2cx_master_fifo_write(I2C_RESOURCES* i2c)
{
    uint32_t write_fifo_count = 0;

    write_fifo_count = ((i2c->info->trans_info.tx_num - i2c->info->trans_info.tx_buf_pointer)\
            >= i2c->info->fifo_depth) ?
                    i2c->info->fifo_depth :
                    (i2c->info->trans_info.tx_num - i2c->info->trans_info.tx_buf_pointer);

    {
        uint32_t i;

        for (i = 0; i < write_fifo_count; i++) {

            i2c->reg->REG_DATA.all = (\
                    i2c->info->trans_info.tx_buf[i2c->info->trans_info.tx_buf_pointer]\
                    & (BITSET_0X020_DATA_MASK));

            i2c->info->trans_info.tx_buf_pointer++;

            if (i2c->info->trans_info.tx_buf_pointer == i2c->info->trans_info.tx_num) {
                // I2C disable the FIFO Empty Interrupt in the Interrupt Enable Register
                i2c->reg->REG_INTEN.bit.FIFOEMPTY = 0x0;
                break;
            }
        }
    }
}

static void
i2cx_slave_fifo_write(I2C_RESOURCES* i2c)
{
    // interrupt mode
    if (i2c->info->inter_en)
    {
        // slave TX 1 byte each time, since no information got
        // about how many bytes of master rx should be,
        // check nack_assert to complete slave tx
        uint32_t write_fifo_count = 1;

        uint32_t i;
        // I2C write a patch of data(FIFO_Depth) to FIFO,
        // it will be consumed empty if data is actually issued on I2C bus
        for (i = 0; i < write_fifo_count; i++){

            // I2C write data to FIFO through data port register
            i2c->reg->REG_DATA.bit.DATA = i2c->info->trans_info.tx_buf[i2c->info->trans_info.tx_buf_pointer] & (BITSET_0X020_DATA_MASK);

            i2c->info->trans_info.tx_buf_pointer++;
        }
    }
    else
    {
        int8_t stat;
        // If DMA is enabled, DataCnt is the number of
        // bytes to transmit/receive. It will not be reset to 0
        // when the slave is addressed and it will be
        // decreased by one for each byte of data
        // transmitted/received.*/
        // fix bug of dma-slave 10bit address test(rx 3 bytes flash addr)
        i2c->reg->REG_CTRL.bit.DATACNT = i2c->info->trans_info.tx_num & BITSET_0X024_DATACNT;

        dma_channel_select(
            &i2c->dma_tx->channel,
            i2c->dma_tx->cb_event,
            0,
            DMA_CACHE_SYNC_SRC);

        if (i2c->dma_tx->channel == DMA_CHANNEL_ANY) {
            return;
        }

        stat = dma_channel_configure (i2c->dma_tx->channel,
                    (uint32_t) (&i2c->info->trans_info.tx_buf[0]),
                    (uint32_t) (&(i2c->reg->REG_DATA.all)),
                    i2c->info->trans_info.tx_num,
                    DMA_CH_CTLL_DST_WIDTH(_I2C_TX_DMA_WIDTH) | DMA_CH_CTLL_SRC_WIDTH(_I2C_TX_DMA_WIDTH) |\
                  DMA_CH_CTLL_DST_BSIZE(_I2C_TX_DMA_BSIZE) | DMA_CH_CTLL_SRC_BSIZE(_I2C_TX_DMA_BSIZE) |\
                  DMA_CH_CTLL_DST_FIX | DMA_CH_CTLL_SRC_INC | DMA_CH_CTLL_TTFC_M2P |\
                  DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                  DMA_CH_CFGL_CH_PRIOR(1), // config_low
                  DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_DST_PER(i2c->dma_tx->reqsel), // config_high
                  0, 0);

        if(stat == -1){
            return;
        }

    }
}

static void
i2cx_fifo_empty_handler(I2C_RESOURCES* i2c)
{
    if (i2c->info->pwr_state != CSK_POWER_FULL){
        return;
    }

    if (i2c->info->state & I2C_DRV_MASTER_TX){
        i2cx_master_fifo_write(i2c);
    } else if (i2c->info->state & I2C_DRV_SLAVE_TX){
        i2cx_slave_fifo_write(i2c);
    }
}

static void
i2cx_master_fifo_read(I2C_RESOURCES* i2c)
{
    uint32_t i = 0, read_fifo_count = 0;

    read_fifo_count =
            ((i2c->info->trans_info.rx_num - i2c->info->trans_info.rx_buf_pointer)
                    >= i2c->info->fifo_depth) ?
                    i2c->info->fifo_depth :
                    (i2c->info->trans_info.rx_num - i2c->info->trans_info.rx_buf_pointer);

    // I2C read a patch of data(FIFO_Depth) from FIFO,
    // it will be consumed empty if data is actually read out by driver
    for (i = 0; i < read_fifo_count; i++) {
        // I2C read data from FIFO through data port register
        i2c->info->trans_info.rx_buf[i2c->info->trans_info.rx_buf_pointer] = i2c->reg->REG_DATA.all & (BITSET_0X020_DATA_MASK);

        i2c->info->trans_info.rx_buf_pointer++;

        // If all data are read from the FIFO, disable the FIFO Full Interrupt. Otherwise, repeat
        if (i2c->info->trans_info.rx_buf_pointer == i2c->info->trans_info.rx_num)
        {
            // I2C disable the FIFO Full Interrupt in the Interrupt Enable Register
            i2c->reg->REG_INTEN.bit.FIFOFULL = 0x0;
        }
    }
}

static void
i2cx_slave_fifo_read(I2C_RESOURCES* i2c, uint8_t is_fifo_full)
{
    uint32_t i = 0, read_fifo_count = 0, curr_rx_data_count = 0;

    // slave rx data count is accumulated and depend on
    // master tx data count of one transaction(start-addr-data-stop),
    // possible larger than fifo length(4 bytes)
    // slave: If DMA is not enabled(FIFO mode), DataCnt is the number of
    // bytes transmitted/received from the bus master.
    // It is reset to 0 when the controller is addressed
    // and then increased by one for each byte of data
    // transmitted/received
     curr_rx_data_count = i2c->reg->REG_CTRL.bit.DATACNT;

    // error hit
    if (curr_rx_data_count > MAX_XFER_SZ)
    {
        assert(0);
    }

    if (is_fifo_full)
    {
        read_fifo_count = i2c->info->fifo_depth;
    }
    else
    {
        read_fifo_count = curr_rx_data_count - i2c->info->trans_info.slave_read_last_rx_data_count;
    }

    if (read_fifo_count > MAX_XFER_SZ)
    {
        assert(0);
    }

    // I2C read a patch of data(FIFO_Depth) from FIFO,
    // it will be consumed empty if data is actually read out by driver */
    for (i = 0; i < read_fifo_count; i++) {
        // I2C read data from FIFO through data port register
        i2c->info->trans_info.slave_read_mid_buf[i2c->info->trans_info.slave_read_last_rx_data_count] =
                (i2c->reg->REG_DATA.all & BITSET_0X020_DATA_MASK);

        i2c->info->trans_info.slave_read_last_rx_data_count++;

        if (i2c->info->trans_info.slave_read_last_rx_data_count == MAX_XFER_SZ)
        {
            // slave rx buffer overwrite
            i2c->info->status.slave_rx_over_flow = 0x1;
            i2c->info->trans_info.slave_read_last_rx_data_count = 0;
        }
    }
}

static void
i2cx_fifo_full_handler(I2C_RESOURCES* i2c)
{
    if (i2c->info->pwr_state != CSK_POWER_FULL) {
        return;
    }

    if (i2c->info->state & I2C_DRV_MASTER_RX) {
        i2cx_master_fifo_read(i2c);
    } else if (i2c->info->state & I2C_DRV_SLAVE_RX) {
        i2cx_slave_fifo_read(i2c, 1);
    }
}

static void
i2cx_slave_addr_hit_handler(I2C_RESOURCES* i2c)
{
    int32_t stat = 0;

    if (i2c->info->pwr_state != CSK_POWER_FULL)
    {
        return;
    }

    // I2C clear fifo first to prevent mistake i2cx_slave_fifo_read()
    // if the Completion Interrupt asserts, clear the FIFO and go next transaction.
    i2c->reg->REG_CMD.bit.CMD = _CMD_CLEAR_FIFO;

    // slave mode Rx: if address hit, fifo may not be full state
    if (i2c->info->state & I2C_DRV_SLAVE_RX)
    {
        // A new I2C data transaction(start-addr-data-stop)
        // clear slave read software
        i2c->info->trans_info.slave_read_mid_buf_pointer = 0;
        i2c->info->trans_info.slave_read_last_rx_data_count = 0;
        __builtin_memset (i2c->info->trans_info.slave_read_mid_buf, 0, sizeof(i2c->info->trans_info.slave_read_mid_buf));

        if (i2c->info->inter_en)
        {
            // I2C Enable the FIFO Full Interrupt in the Interrupt Enable Register
            i2c->reg->REG_INTEN.bit.FIFOFULL = 0x1;
        }
        else
        {
            // If DMA is enabled, DataCnt is the number of
            // bytes to transmit/receive. It will not be reset to 0
            // when the slave is addressed and it will be
            // decreased by one for each byte of data
            // transmitted/received.
            // fix bug of dma-slave 10bit address test(rx 3 bytes flash addr)
            i2c->reg->REG_CTRL.bit.DATACNT = (MAX_XFER_SZ & BITSET_0X024_DATACNT);

            dma_channel_select(
                &i2c->dma_rx->channel,
                i2c->dma_rx->cb_event,
                0,
                DMA_CACHE_SYNC_DST);
            if (i2c->dma_rx->channel == DMA_CHANNEL_ANY) {
                return;
            }

            stat = dma_channel_configure (i2c->dma_rx->channel,
                        (uint32_t) (&(i2c->reg->REG_DATA.all)),
                        (uint32_t) (&i2c->info->trans_info.slave_read_mid_buf[0]),
                        MAX_XFER_SZ,
                        DMA_CH_CTLL_DST_WIDTH(_I2C_RX_DMA_WIDTH) | DMA_CH_CTLL_SRC_WIDTH(_I2C_RX_DMA_WIDTH) |\
                      DMA_CH_CTLL_DST_BSIZE(_I2C_RX_DMA_BSIZE) | DMA_CH_CTLL_SRC_BSIZE(_I2C_RX_DMA_BSIZE) |\
                      DMA_CH_CTLL_DST_INC | DMA_CH_CTLL_SRC_FIX | DMA_CH_CTLL_TTFC_P2M |\
                      DMA_CH_CTLL_DMS(0) | DMA_CH_CTLL_SMS(0) | DMA_CH_CTLL_INT_EN, // control
                      DMA_CH_CFGL_CH_PRIOR(1), // config_low
                      DMA_CH_CFGH_FIFO_MODE | DMA_CH_CFGH_SRC_PER(i2c->dma_rx->reqsel), // config_high
                      0, 0);

            if(stat == -1){
                return;
            }
        }
    }
    // slave mode Tx: if address hit, fifo may not be empty state
    else if (i2c->info->state & I2C_DRV_SLAVE_TX)
    {
        if (i2c->info->inter_en){
            // I2C Enable the Byte Transmit Interrupt in the Interrupt Enable Register
            // for status.busy flag support
            i2c->reg->REG_INTEN.bit.BYTETRANS = 0x1;
            i2c->reg->REG_SETUP.bit.DMAEN = 0x0;
        }else{
            i2c->reg->REG_SETUP.bit.DMAEN = 0x1;

            i2c->reg->REG_INTEN.all |= (BITSET_0X014_CMPL | BITSET_0X014_ARBLOS);
        }
    }
}


void
i2c_irq_handler(I2C_RESOURCES* i2c)
{
    uint32_t iir, ier, ctrl,event = 0;
    iir = i2c->reg->REG_STATUS.all;
    ier = i2c->reg->REG_INTEN.all;
    ctrl = i2c->reg->REG_CTRL.all;

    // write 1 clear for those interrupts be able to W1C
    i2c->reg->REG_STATUS.all = (iir & BITSET_0X018_STATUS_CLEAR);

    // Here is the entry for slave mode driver to detect
    // slave RX/TX action depend on master TX/RX action.
    // Addr hit is W1C bit, so during payload transaction,
    // it is not set again.
    // A new I2C data transaction(start-addr-data-stop)
    if (iir & BITSET_0X018_ADDRHIT)
    {
        // slave mode
        if (i2c->reg->REG_SETUP.bit.MASTER == 0x0)
        {
            // Indicates that the address of the current
            // transaction is a general call address.
            // This status is only valid in slave mode.
            // A new I2C data transaction(start-addr-data-stop)
            if (iir & BITSET_0X018_GENCALL){
                i2c->info->status.general_call = 1;

                event |= CSK_I2C_EVENT_GENERAL_CALL;
            }

            if (((union I2C_REG_CTRL*)ctrl)->bit.DIR == I2C_SLAVE_RX)
            {

                // notify middleware to do slave rx action
                event |= CSK_I2C_EVENT_SLAVE_RECEIVE;

                //clear & set driver state to slave rx before data transaction
                i2c->info->state = I2C_DRV_SLAVE_RX;
            }
            else if (((union I2C_REG_CTRL*)ctrl)->bit.DIR == I2C_SLAVE_TX)
            {

                // notify middleware to do slave tx action
                event |= CSK_I2C_EVENT_SLAVE_TRANSMIT;

                //clear & set driver state to slave tx before data transaction
                i2c->info->state = I2C_DRV_SLAVE_TX;
            }

            // A new I2C data transaction(start-addr-data-stop)
            i2cx_slave_addr_hit_handler(i2c);
        }
    }

    if ((iir & BITSET_0X018_CMPL) && (ier & BITSET_0X014_CMPL))
    {
        i2c_cmpl_handler(i2c);

        event |= CSK_I2C_EVENT_TRANSFER_DONE;
    }
    else
    {
        event |= CSK_I2C_EVENT_TRANSFER_INCOMPLETE;
    }

    if ((iir & BITSET_0X018_FIFO_EMPTY) && (ier & BITSET_0X014_FIFOEMPTY))
    {
        i2cx_fifo_empty_handler(i2c);
    }

    if ((iir & BITSET_0X018_FIFOFULL) && (ier & BITSET_0X014_FIFOFULL))
    {
        i2cx_fifo_full_handler(i2c);
    }

    if ((iir & BITSET_0X018_ARBLOS) && (i2c->reg->REG_SETUP.bit.MASTER == 0x0))
    {
        i2c->info->status.arbitration_lost = 1;

        event |= CSK_I2C_EVENT_ARBITRATION_LOST;
    }

    if ((iir & BITSET_0X018_BYTETRANS) && (i2c->reg->REG_SETUP.bit.MASTER == 0x0))
    {
        // I2C clear fifo first to prevent mistake i2cx_slave_fifo_read()
        // if the Completion Interrupt asserts, clear the FIFO and go next transaction.
        i2c->reg->REG_CMD.bit.CMD = _CMD_CLEAR_FIFO;

        if (i2c->info->inter_en){
            i2c->info->trans_info.cmpl_count++;
        }

        // set on start of next slave operation,
        // cleared on slave tx 1 byte done or data transaction complete.
//        i2c->info->status.busy = 0;
        i2cx_slave_fifo_write(i2c);
    }

    // invoke callback function
    if (i2c->info->cb_event)
    {
        i2c->info->cb_event(event, i2c->info->workspace);
    }
}


static void i2cx_dma_tx_event (uint32_t event, I2C_RESOURCES* i2c)
{
    switch(event)
    {
        case DMA_EVENT_TRANSFER_COMPLETE:
        break;
        case DMA_EVENT_ERROR:
        default:
        break;
    }
}

static void i2cx_dma_rx_event (uint32_t event, I2C_RESOURCES* i2c)
{
    switch (event)
    {
        case DMA_EVENT_TRANSFER_COMPLETE:
        // clear busy bit on dma complete event as master dma rx
        i2c->info->status.busy = 0;
        break;
        case DMA_EVENT_ERROR:
        default:
        break;
    }
}

static void i2c0_dma_tx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param)
{
    i2cx_dma_tx_event((event & 0xff), &i2c0_resources);
}

static void i2c0_dma_rx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param)
{
    i2cx_dma_rx_event((event & 0xff), &i2c0_resources);
}

static void i2c0_irq_handler(void){
    i2c_irq_handler(&i2c0_resources);
}

static void i2c1_dma_tx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param)
{
    i2cx_dma_tx_event((event & 0xff), &i2c1_resources);
}

static void i2c1_dma_rx_event (uint32_t event, uint32_t xfer_bytes, uint32_t usr_param)
{
    i2cx_dma_rx_event((event & 0xff), &i2c1_resources);
}

static void i2c1_irq_handler(void){
    i2c_irq_handler(&i2c1_resources);
}

