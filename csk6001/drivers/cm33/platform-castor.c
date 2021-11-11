#include "hal/platform.h"
#include "hal/spiflash.h"
#include "hal/venus_ap.h"

#ifndef printf
#define printf(format, ...) ((void)0)
#endif

#ifndef inw
#define inw(reg) (*((volatile unsigned int *)(reg)))
#endif
#ifndef outw
#define outw(reg, data) ((*((volatile unsigned int *)(reg))) = (unsigned int)(data))
#endif

#define REG_SMU_BASE 0xF0100000
#define PWCTL_BASE 0xF1A00000
#define ANTOP_BASE 0xF1B00000

#define CPE_SPIB_BASE CMN_FLASHC_BASE /*0x46400000*/
#define CPE_SPI2_BASE SPI0_BASE       /*0x45400000*/
#define CPE_SPI3_BASE SPI1_BASE       /*0x45500000*/

#define REG_SMU_BASE_16 0x00F01000
#define CPE_SPIB_BASE_16 0x00F0B000

#define SMU_SYSID_AE100 0x41451
#define SMU_SYSID_AE210_16MB 0x41452
#define SMU_SYSID_AE210_4GB 0x41452
#define SMU_SYSID_AE300_4GB 0x41453

#define SPI_TX_FIFO 256 // bytes
#define SPI_RX_FIFO 256 // bytes

#define MEMMAP_AE100 0
#define MEMMAP_AE210_16MB 1
#define MEMMAP_AE210_4GB 2
#define MEMMAP_AE300_4GB 3
#define MEMMAP_MAX 4
/*===========================================*/
/*   SPI driver                              */
/*===========================================*/

/*======================================================*/
/* SPIB register definition  */
/*======================================================*/
#define SPIB_REG_VER(base) (base + 0x00)
#define SPIB_REG_IFSET(base) (base + 0x10)
#define SPIB_REG_PIO(base) (base + 0x14)
#define SPIB_REG_DCTRL(base) (base + 0x20)
#define SPIB_REG_CMD(base) (base + 0x24)
#define SPIB_REG_ADDR(base) (base + 0x28)
#define SPIB_REG_DATA(base) (base + 0x2c)
#define SPIB_REG_CTRL(base) (base + 0x30)
#define SPIB_REG_FIFOST(base) (base + 0x34)
#define SPIB_REG_INTEN(base) (base + 0x38)
#define SPIB_REG_INTST(base) (base + 0x3c)
#define SPIB_REG_REGTIMING(base) (base + 0x40)
#define SPIB_REG_MEMACCESS(base) (base + 0x50)
/*--Interface Set Reg*/
#define SPIB_IF_ADDLEN_MASK 0x00030000
#define SPIB_IF_DATALEN_MASK 0x00001f00
#define SPIB_IF_DATAMERGE_MASK 0x00000080
#define SPIB_IF_DIR_MASK 0x00000010
#define SPIB_IF_LSB_MASK 0x00000008
#define SPIB_IF_SLV_MASK 0x00000004
#define SPIB_IF_CPOL_MASK 0x00000002
#define SPIB_IF_CPHA_MASK 0x00000001

#define SPIB_IF_ADDLEN_OFFSET 16
#define SPIB_IF_DATALEN_OFFSET 8
#define SPIB_IF_DATAMERGE_OFFSET 7
#define SPIB_IF_DIR_OFFSET 4
#define SPIB_IF_LSB_OFFSET 3
#define SPIB_IF_SLV_OFFSET 2
#define SPIB_IF_CPOL_OFFSET 1
#define SPIB_IF_CPHA_OFFSET 0

/*-- Data Control Reg --*/
#define SPIB_DCTRL_CMDEN_MASK 0x40000000
#define SPIB_DCTRL_ADDREN_MASK 0x20000000
#define SPIB_DCTRL_TRAMODE_MASK 0x0f000000
#define SPIB_DCTRL_WCNT_MASK 0x001ff000
#define SPIB_DCTRL_DYCNT_MASK 0x00000600
#define SPIB_DCTRL_RCNT_MASK 0x000001ff
#define SPIB_DCTRL_ADDRFMT_MASK 0x10000000
#define SPIB_DCTRL_DATAFMT_MASK 0xc00000
#define SPIB_DCTRL_TOKENEN_MASK 0x200000
#define SPIB_DCTRL_CMDEN_OFFSET 30
#define SPIB_DCTRL_ADDREN_OFFSET 29
#define SPIB_DCTRL_TRAMODE_OFFSET 24
#define SPIB_DCTRL_WCNT_OFFSET 12
#define SPIB_DCTRL_DYCNT_OFFSET 9
#define SPIB_DCTRL_RCNT_OFFSET 0
#define SPIB_DCTRL_ADDRFMT_OFFSET 28
#define SPIB_DCTRL_DATAFMT_OFFSET 22
#define SPIB_DCTRL_TOKENEN_OFFSET 21
/*-- Control Reg --*/
#define SPIB_CTRL_TXFRST_MASK 0x00000004
#define SPIB_CTRL_RXFRST_MASK 0x00000002
#define SPIB_CTRL_SPIRST_MASK 0x00000001
/*-- FIFO Status Reg --*/
#define SPIB_FIFOST_TXFFL_MASK 0x00800000
#define SPIB_FIFOST_TXFEM_MASK 0x00400000
#define SPIB_FIFOST_TXFVE_MASK 0x001f0000
#define SPIB_FIFOST_RXFFL_MASK 0x00008000
#define SPIB_FIFOST_RXFEM_MASK 0x00004000
#define SPIB_FIFOST_RXFVE_MASK 0x00001f00
#define SPIB_FIFOST_SPIBSY_MASK 0x00000001
#define SPIB_FIFOST_TXFFL_OFFSET 23
#define SPIB_FIFOST_TXFEM_OFFSET 22
#define SPIB_FIFOST_TXFVE_OFFSET 16
#define SPIB_FIFOST_RXFFL_OFFSET 15
#define SPIB_FIFOST_RXFEM_OFFSET 14
#define SPIB_FIFOST_RXFVE_OFFSET 8
#define SPIB_FIFOST_SPIBSY_OFFSET 0
#define SPIB_FIFOST_SPIBSYnRXFEM (SPIB_FIFOST_RXFEM_MASK | SPIB_FIFOST_SPIBSY_MASK)

unsigned int memmapping_spib_base[MEMMAP_MAX] = {
    CPE_SPIB_BASE_16, // AE100
    CPE_SPIB_BASE_16, // AE210_16MB
    CPE_SPIB_BASE,    // AE210_4GB
    CPE_SPIB_BASE,    // AE300
};

unsigned int memmapping_smu_base[MEMMAP_MAX] = {
    REG_SMU_BASE_16, // AE100
    REG_SMU_BASE_16, // AE210_16MB
    REG_SMU_BASE,    // AE210_4GB
    REG_SMU_BASE,    // AE300
};

unsigned int memmapping_system_id[MEMMAP_MAX] = {
    SMU_SYSID_AE100,      // AE100
    SMU_SYSID_AE210_16MB, // AE210_16MB
    SMU_SYSID_AE210_4GB,  // AE210_4GB
    SMU_SYSID_AE300_4GB,  // AE300
};

const char *memmapping_target_name[MEMMAP_MAX] = {
    "AE100",
    "AE210 with 16MB",
    "Castor with 4GB",
    "AE300",
};

/* Fast memory access with a constant address, EDM_CFG >= 0x1011 only */
unsigned int guiConstFastMode = 0;

//unsigned int platform_get_version_id(void);
unsigned int spib_ctrl = 0;
const unsigned long spi_base[3] = {CPE_SPIB_BASE, CPE_SPI2_BASE, CPE_SPI3_BASE};
unsigned int mem_mapping_mode = 2;

#define SMU_HPCLKSEL_1_4 (0x2 << 1)
#define SMU_HPCLKSEL_1_1 (0x0 << 1)
#define SMU_REG_CTRL (REG_SMU_BASE + 0x24)
#define SMU_REG_CLK (REG_SMU_BASE + 0x20)

#define PW_REG_CTRL (ANTOP_BASE + 0x2C)
#define PW_REG_CTRL2 (ANTOP_BASE + 0x34)

#define CPE_EFUSE_BASE (0xf1800000UL)
#define CPE_EFUSE_REG_BASE (CPE_EFUSE_BASE + 0x80)
#define BYTE4_FLASH_VENDOR_POS 3
#define BYTE4_FLASH_VENDOR_MSK (0x1UL << BYTE4_FLASH_VENDOR_POS)

_EXT_RAM void efuse_get_flash_vendor()
{
    //	unsigned int reg;
    //	reg = inw(CPE_EFUSE_REG_BASE + 4);
    //    printf("flash vendor reg=0x%x\n", reg);
}

_EXT_RAM void efuse_set_flash_vendor(unsigned int v)
{
    //	unsigned int tmp;
    //	tmp = inw(CPE_EFUSE_REG_BASE + 4);
    //    tmp &= (~BYTE4_FLASH_VENDOR_MSK);
    //    tmp |= ((v & 0x1) << BYTE4_FLASH_VENDOR_POS);
    //    outw(CPE_EFUSE_REG_BASE + 4, tmp);
}

_EXT_RAM void power_set_perp_clk_div0(int clk)
{
    //    unsigned int tmp, tmp2;
    //    extern unsigned long g_syspll_div;
    //    if(clk > 100 || clk <= 0) {
    //    	clk = 100;
    //    }
    //    tmp = inw(PW_REG_CTRL) & (~0xff000000UL);
    //    tmp2 = CPUFREQ() / (clk * 1000000) + 1;		//set spi_clk to below 100MHz as fast as possible
    //	outw(PW_REG_CTRL, tmp | (tmp2 << 24));
}

_EXT_RAM void power_set_perp_clk_div2(int clk)
{
    //    unsigned int tmp, tmp2;
    //    extern unsigned long g_syspll_div;
    //    if(clk > 100 || clk <= 0) {
    //    	clk = 100;
    //    }
    //    tmp = inw(PW_REG_CTRL2) & (~0x000000ffUL);
    //    tmp2 = CPUFREQ() / (clk * 1000000) + 1;		//set spi_clk to below 100MHz as fast as possible
    //	outw(PW_REG_CTRL2, tmp | tmp2);
}

_EXT_RAM void iomux_set(int io)
{
    //	uint32_t tmp;
    //
    //	tmp = inw(ANALOG_BASE + 0x48);
    //	switch(io) {
    //	case FLASH_SPI_1_INN:	//inner flash
    //		efuse_set_flash_vendor(0);
    //		gpio_func_sel(12, 0);
    //		gpio_func_sel(13, 0);
    //		gpio_func_sel(14, 0);
    //		gpio_func_sel(15, 0);
    //		gpio_func_sel(16, 0);
    //		gpio_func_sel(17, 0);
    //
    //		gpio_func_sel(0, 3);
    //		gpio_func_sel(1, 3);
    //		gpio_func_sel(2, 3);
    //		gpio_func_sel(28, 3);
    //		gpio_func_sel(29, 3);
    //		gpio_func_sel(30, 3);
    //		outw(ANALOG_BASE + 0x48, (tmp | (1UL << 8)));
    //		break;
    //	case FLASH_SPI_1_EXT: //external flash 1
    //		efuse_set_flash_vendor(1);
    //		gpio_func_sel(0, 0);
    //		gpio_func_sel(1, 0);
    //		gpio_func_sel(2, 0);
    //		gpio_func_sel(28, 0);
    //		gpio_func_sel(29, 0);
    //		gpio_func_sel(30, 0);
    //
    //		gpio_func_sel(12, 1);
    //		gpio_func_sel(13, 1);
    //		gpio_func_sel(14, 1);
    //		gpio_func_sel(15, 1);
    //		gpio_func_sel(16, 1);
    //		gpio_func_sel(17, 1);
    //		//clear qpsi_psram_bond
    //		outw(ANALOG_BASE + 0x48, (tmp & (~(1UL << 8)))); // for qflash
    //		break;
    //	case FLASH_SPI_2_PA04_PA07: //external spi2 PA4~PA7
    //		gpio_func_sel(4, 2);
    //		gpio_func_sel(5, 2);
    //		gpio_func_sel(6, 2);
    //		gpio_func_sel(7, 2);
    //		break;
    //	case FLASH_SPI_2_PA08_PA11: //external spi2 PA8~PA11
    //		gpio_func_sel(8, 3);
    //		gpio_func_sel(9, 3);
    //		gpio_func_sel(10, 3);
    //		gpio_func_sel(11, 3);
    //		break;
    //	case FLASH_SPI_3_PA12_PA15: //external spi3 PA12~PA15
    //		gpio_func_sel(12, 3);
    //		gpio_func_sel(13, 3);
    //		gpio_func_sel(14, 3);
    //		gpio_func_sel(15, 3);
    //		break;
    //	case FLASH_SPI_3_PA22_PA25: //external spi3 PA22~PA25
    //		gpio_func_sel(22, 3);
    //		gpio_func_sel(23, 3);
    //		gpio_func_sel(24, 3);
    //		gpio_func_sel(25, 3);
    //		break;
    //	case FLASH_SPI_3_PA31_PB02: //external spi3 PA31~PB2
    //		gpio_func_sel(31, 3);
    //		gpio_func_sel(31 + 1, 3);
    //		gpio_func_sel(31 + 2, 3);
    //		gpio_func_sel(31 + 3, 3);
    //		break;
    //	default: // ToDo
    //		break;
    //	}
}

extern int mxic_check(unsigned long base);
extern int mxic_enable_qd(unsigned long base);

_EXT_RAM void set_io_clk(int io, int clk)
{
    iomux_set(io);
    if (io <= FLASH_SPI_1_EXT)
    {
        power_set_perp_clk_div0(clk);
    }
    else
    {
        power_set_perp_clk_div2(clk);
    }
}

_EXT_RAM void switch_to_inn(int clk)
{
    set_io_clk(FLASH_SPI_1_INN, clk);
    mxic_enable_qd(spi_base[0]);
}

_EXT_RAM void switch_to_ext(int clk)
{
    set_io_clk(FLASH_SPI_1_EXT, clk);
    mxic_enable_qd(spi_base[0]);
}

/* for 24 bit platform */
_EXT_RAM int platform_init(int spi, int clk)
{
    int ret;
    if (spi < FLASH_SPI_1_INN || spi > FLASH_SPI_3_PA31_PB02)
    { //default device is spi0
        spi = FLASH_SPI_1_INN;
    }
    set_io_clk(spi, clk); // defalut config. io: 0-inner, 1-external; clk: 100MHz
    spib_set_ifset(spi_base[spi / 10], ((2 << SPIB_IF_ADDLEN_OFFSET) & SPIB_IF_ADDLEN_MASK) |
                                           ((7 << SPIB_IF_DATALEN_OFFSET) & SPIB_IF_DATALEN_MASK) |
                                           ((1 << SPIB_IF_DATAMERGE_OFFSET) & SPIB_IF_DATAMERGE_MASK) | ((0 << SPIB_IF_DIR_OFFSET) & SPIB_IF_DIR_MASK) |
                                           ((0 << SPIB_IF_LSB_OFFSET) & SPIB_IF_LSB_MASK) | ((0 << SPIB_IF_SLV_OFFSET) & SPIB_IF_SLV_MASK) |
                                           ((0 << SPIB_IF_CPOL_OFFSET) & SPIB_IF_CPOL_MASK) | ((0 << SPIB_IF_CPHA_OFFSET) & SPIB_IF_CPHA_MASK));

    //check flash ID
    ret = mxic_check(spi_base[spi / 10]);
#ifdef FLASH_QUAD_MODE_EN
    if (spi <= FLASH_SPI_1_EXT)
    {
        //let flash support quad mode
        ret = mxic_enable_qd(spi_base[0]);
        if (ret == 0)
        {
            //set spi read mode as Quad mode
            outw(SPIB_REG_MEMACCESS(spi_base[0]), 0x5);
        }
    }
#endif
    return ret;
}

//extern FILE* pLogFile;
//_EXT_RAM unsigned int platform_get_version_id(void)
//{
//    return 0;
//}

/*--------------------------------------------*/
/* SPIB function                              */
/*--------------------------------------------*/
_EXT_RAM unsigned int spib_get_ifset(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_IFSET(base));
    return reg;
}

_EXT_RAM void spib_set_ifset(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_IFSET(base), reg);
}

_EXT_RAM unsigned int spib_get_pio(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_PIO(base));
    return reg;
}

_EXT_RAM void spib_set_pio(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_PIO(base), reg);
}

_EXT_RAM unsigned int spib_get_ctrl(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_CTRL(base));
    return reg;
}

_EXT_RAM void spib_set_ctrl(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_CTRL(base), reg);
}

_EXT_RAM unsigned int spib_get_fifost(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_FIFOST(base));
    return reg;
}

_EXT_RAM unsigned int spib_get_inten(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_INTEN(base));
    return reg;
}

_EXT_RAM void spib_set_inten(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_INTEN(base), reg);
}

_EXT_RAM unsigned int spib_get_intst(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_INTST(base));
    return reg;
}

_EXT_RAM void spib_set_intst(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_INTST(base), reg);
}

_EXT_RAM unsigned int spib_get_dctrl(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_DCTRL(base));
    /*check_timeout("read SPIB_REG_DCTRL failed");*/
    return reg;
}

_EXT_RAM void spib_set_dctrl(unsigned long base, unsigned int reg)
{
    outw(SPIB_REG_DCTRL(base), reg);
}

_EXT_RAM unsigned int spib_get_cmd(unsigned long base)
{
    return inw(SPIB_REG_CMD(base));
}

_EXT_RAM void spib_set_cmd(unsigned long base, unsigned int cmd)
{
    outw(SPIB_REG_CMD(base), cmd);
}

_EXT_RAM unsigned int spib_get_addr(unsigned long base)
{
    return inw(SPIB_REG_ADDR(base));
}

_EXT_RAM void spib_set_addr(unsigned long base, unsigned int addr)
{
    outw(SPIB_REG_ADDR(base), addr);
}

_EXT_RAM unsigned int spib_get_data(unsigned long base)
{
    return inw(SPIB_REG_DATA(base));
}

_EXT_RAM void spib_set_data(unsigned long base, unsigned int data)
{
    outw(SPIB_REG_DATA(base), data);
}

_EXT_RAM unsigned int spib_get_regtiming(unsigned long base)
{
    return inw(SPIB_REG_REGTIMING(base));
}

_EXT_RAM void spib_set_regtiming(unsigned long base, unsigned int data)
{
    outw(SPIB_REG_REGTIMING(base), data);
}

_EXT_RAM unsigned int spib_prepare_dctrl(unsigned int cmden,
                                         unsigned int addren,
                                         unsigned int tm,
                                         unsigned int wcnt,
                                         unsigned int dycnt,
                                         unsigned int rcnt)
{
    unsigned int v[8];
    unsigned int i;
    unsigned int dctrl = 0x0;

    v[0] = ((cmden << SPIB_DCTRL_CMDEN_OFFSET) & SPIB_DCTRL_CMDEN_MASK);
    v[1] = ((addren << SPIB_DCTRL_ADDREN_OFFSET) & SPIB_DCTRL_ADDREN_MASK);
    v[2] = ((tm << SPIB_DCTRL_TRAMODE_OFFSET) & SPIB_DCTRL_TRAMODE_MASK);
    v[3] = ((wcnt << SPIB_DCTRL_WCNT_OFFSET) & SPIB_DCTRL_WCNT_MASK);
    v[4] = ((dycnt << SPIB_DCTRL_DYCNT_OFFSET) & SPIB_DCTRL_DYCNT_MASK);
    v[5] = ((rcnt << SPIB_DCTRL_RCNT_OFFSET) & SPIB_DCTRL_RCNT_MASK);

    for (i = 0; i < 6; i++)
        dctrl |= v[i];
    // printf("dctrl = %x\n", dctrl);
    return dctrl;
}

_EXT_RAM unsigned int spib_prepare_dctrl2(unsigned int cmden,
                                          unsigned int addren,
                                          unsigned int tm,
                                          unsigned int wcnt,
                                          unsigned int dycnt,
                                          unsigned int rcnt,
                                          unsigned int addrfmt,
                                          unsigned int datafmt,
                                          unsigned int tokenen)
{
    unsigned int v[9];
    unsigned int i;
    unsigned int dctrl = 0x0;

    v[0] = ((cmden << SPIB_DCTRL_CMDEN_OFFSET) & SPIB_DCTRL_CMDEN_MASK);
    v[1] = ((addren << SPIB_DCTRL_ADDREN_OFFSET) & SPIB_DCTRL_ADDREN_MASK);
    v[2] = ((tm << SPIB_DCTRL_TRAMODE_OFFSET) & SPIB_DCTRL_TRAMODE_MASK);
    v[3] = ((wcnt << SPIB_DCTRL_WCNT_OFFSET) & SPIB_DCTRL_WCNT_MASK);
    v[4] = ((dycnt << SPIB_DCTRL_DYCNT_OFFSET) & SPIB_DCTRL_DYCNT_MASK);
    v[5] = ((rcnt << SPIB_DCTRL_RCNT_OFFSET) & SPIB_DCTRL_RCNT_MASK);
    v[6] = (addrfmt << SPIB_DCTRL_ADDRFMT_OFFSET) & SPIB_DCTRL_ADDRFMT_MASK;
    v[7] = (datafmt << SPIB_DCTRL_DATAFMT_OFFSET) & SPIB_DCTRL_DATAFMT_MASK;
    v[8] = (tokenen << SPIB_DCTRL_TOKENEN_OFFSET) & SPIB_DCTRL_TOKENEN_MASK;

    for (i = 0; i < 9; i++)
        dctrl |= v[i];
    // printf("dctrl = %x\n", dctrl);
    return dctrl;
}

_EXT_RAM unsigned int spib_get_version(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_VER(base));
    return reg;
}

_EXT_RAM unsigned int spib_get_busy(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_FIFOST(base));
    // printf("spib_get_busy ===0x%08x\n", reg);
    return (reg & SPIB_FIFOST_SPIBSY_MASK);
}

_EXT_RAM unsigned int spib_wait_spi(unsigned long base)
{
    unsigned int i;
    unsigned int timeout = 1000;

    for (i = 1; i < timeout; i++)
    {
        if (spib_get_busy(base) == 0)
            return 0;
    }
    return 1;
}

_EXT_RAM unsigned int spib_get_rx_empty(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_FIFOST(base));
    return (reg & SPIB_FIFOST_RXFEM_MASK);
}

_EXT_RAM unsigned int spib_wait_rx_empty(unsigned long base)
{
    unsigned int i;
    unsigned int timeout = 1000;

    for (i = 1; i < timeout; i++)
    {
        if (spib_get_rx_empty(base) == 0)
            return 0;
    }
    return 1;
}

_EXT_RAM unsigned int spib_get_rx_entries(unsigned long base)
{
    unsigned int reg = inw(SPIB_REG_FIFOST(base));
    unsigned int RetData;

    RetData = ((reg & SPIB_FIFOST_RXFVE_MASK) >> SPIB_FIFOST_RXFVE_OFFSET);
    return (RetData);
}

_EXT_RAM void spib_clr_fifo(unsigned long base)
{
    unsigned int spib_ctrl = inw(SPIB_REG_CTRL(base));
    spib_ctrl |= (SPIB_CTRL_TXFRST_MASK | SPIB_CTRL_RXFRST_MASK);
    spib_set_ctrl(base, spib_ctrl);
}
#define guiWithMultiout 0

_EXT_RAM void spib_exe_cmmd(unsigned long base, unsigned int op_addr, unsigned int spib_dctrl)
{
    /*-- execute command --*/
    if (guiWithMultiout == 0)
    {
        spib_set_data(base, op_addr);     /*-- push flash command into tx fifo --*/
        spib_set_dctrl(base, spib_dctrl); /*-- set dctrl --*/
        spib_set_cmd(base, 0x0);          /*-- set dummy command to trigger transation start --*/
    }
}

_EXT_RAM void spib_exe_cmmd2(unsigned long base, unsigned int op, unsigned int addr, unsigned int spib_dctrl)
{
    /*-- execute command --*/
    spib_set_dctrl(base, spib_dctrl);
    spib_set_addr(base, addr);
    spib_set_cmd(base, op);
}

_EXT_RAM void spib_rx_data(unsigned long base, unsigned int *pRxdata, int RxBytes)
{
    unsigned int i, RxWords = 0;
    unsigned int *p_dst_buffer = (unsigned int *)pRxdata;

    if (guiWithMultiout == 0)
    {
        /*-- wait completion --*/
        while (spib_get_busy(base) != 0)
        {
            if (spib_get_rx_empty(base) == 0)
            {
                RxWords = spib_get_rx_entries(base);
                // printf("spib_get_rx_entries: %d\n", RxWords);
                for (i = 0; i < RxWords; i++)
                {
                    *p_dst_buffer++ = inw(SPIB_REG_DATA(base));
                }
            }
        }
        RxWords = spib_get_rx_entries(base);
        for (i = 0; i < RxWords; i++)
        {
            *p_dst_buffer++ = inw(SPIB_REG_DATA(base));
        }
    }
}

_EXT_RAM void spib_tx_data(unsigned long base, unsigned int *pTxdata, int TxBytes)
{
    unsigned int i, j;
    unsigned int TxWords = (TxBytes + 3) / 4;
    unsigned int *p_src_buffer = (unsigned int *)pTxdata;
    unsigned int timeout = 100;
    unsigned int spib_tx_full;

    if (guiWithMultiout == 0)
    {
        for (i = 0; i < TxWords; i++)
        {
            for (j = 0; j < timeout; j++)
            {
                spib_tx_full = (spib_get_fifost(base) & SPIB_FIFOST_TXFFL_MASK);

                if (spib_tx_full == 0)
                {
                    break;
                }
            }
            if (spib_tx_full == 1)
            {
                printf("spib_set_fifo: write fifo timeout\n");
                return;
            }
            spib_set_data(base, *p_src_buffer++);
        }
    }
}
