//#include <sys/time.h>
#ifdef __MINGW32__
#include <winsock2.h> // for Sleep()
#endif
#include <string.h>
//#include "ARMCM33_DSP_FP.h"
#include "hal/platform.h"
#include "hal/spiflash.h"
#include "hal/venus_ap.h"

//define it for not interrupt by isr or other task
#define SPIROM_NO_INTERRUPT

#ifndef printf
#define printf(format, ...) ((void)0)
#endif
#define INDIVIDUAL_BLK_PROTECT 0
/*--------------------------------------------*/
/* SPI Flash ROM definition                   */
/*--------------------------------------------*/
#define SPIROM_ID_VERSION 0x1420c2 /* MXIC 25L8006  */
/* #define SPIROM_ID_VERSION   0x1520c2   MXIC 25L1606  */
#define SPIROM_ID_MASK 0x00ffffff
#define SPIROM_OP_READ 0x03		  /*-- read --*/
#define SPIROM_OP_QFAST_READ 0xeb /*-- quad read in higher speed --*/
#define SPIROM_OP_FAST_READ 0x0b  /*-- read in higher speed --*/
#define SPIROM_OP_RDID 0x9f		  /*-- manufacturer/device ID read --*/
#define SPIROM_OP_READ_ID 0x90	  /*-- manufacturer/device ID read --*/
#define SPIROM_OP_WREN 0x06		  /*-- write enable --*/
#define SPIROM_OP_WRDI 0x04		  /*-- write disable --*/
#define SPIROM_OP_SE 0x20		  /*-- sector erase --*/
#define SPIROM_OP_BE 0x52		  /*-- block erase --*/
#define SPIROM_OP_PP 0x02		  /*-- page program --*/
#define SPIROM_OP_QPP 0x32		  /*-- quad page program --*/
#define SPIROM_OP_RDSR 0x05		  /*-- read status register --*/
#define SPIROM_OP_RDSR2 0x35	  /*-- read status register2 --*/
#define SPIROM_OP_WRSR 0x01		  /*-- write status register --*/
#define SPIROM_OP_WRSR2 0x31	  /*-- write status register2 --*/
#define SPIROM_OP_SBLK 0x36		  /*-- single block lock --*/
#define SPIROM_OP_SBULK 0x39	  /*-- single block unlock --*/
#define SPIROM_OP_RDBLOCK 0x3C
#define SPIROM_OP_RDSCUR 0x2B
#define SPIROM_OP_WPSEL 0x68
#define SPIROM_OP_GBLK 0x7E
#define SPIROM_OP_GBULK 0x98

/*-- bit mask for status register --*/
#define SPIROM_SR_WIP_MASK 0x01	  /*-- write in progress --*/
#define SPIROM_SR_WEL_MASK 0x02	  /*-- write enable --*/
#define SPIROM_SR_BP_MASK 0x3C	  /*-- BP protection mode --*/
#define SPIROM_SECTOR_SIZE 0x1000 /*-- sector size in bytes, must be 4byte-aligned --*/
#define SPIROM_PAGE_SIZE 0x100	  /*-- page size --*/
#define SPIROM_PAGEPROG_ADDR_MASK 0xFFFFFF00
/*-- SPI ROM cmd --*/
#define SPIROM_CMD_READ 0x0
#define SPIROM_CMD_RDID 0x1
#define SPIROM_CMD_RDST 0x2
#define SPIROM_CMD_WREN 0x3
#define SPIROM_CMD_WRDI 0x4
#define SPIROM_CMD_ERASE 0x5
#define SPIROM_CMD_PROGRAM 0x6
#define SPIROM_CMD_LOCK 0x7
#define SPIROM_CMD_UNLOCK 0x8
#define SPIROM_CMD_RDBLOCK 0x9
#define SPIROM_CMD_RDSCUR 0xA
#define SPIROM_CMD_WPSEL 0xB
#define SPIROM_CMD_CHIP_UNLOCK 0xC
#define SPIROM_CMD_WRSR 0xD
#define SPIROM_CMD_RDST2 0xE
#define SPIROM_CMD_WRSR2 0xF

#define SPIROM_REAL_SIZE 0x8000

#define FLASH_BLKSIZE SPIROM_SECTOR_SIZE * 16
#define FLASH_CHIPSIZE SPIROM_REAL_SIZE
#define FLASH_RETRY_TIMES 80000

unsigned int mapping_flash_id[10] = {GD25Q20C_ID, GD25Q32C_ID, MX25U1635E_ID, P25Q80H_ID, P25Q32H_ID, GD25PQ32C_ID, FT25F64B_ID, GD25Q64EGIG_ID};

//const char* mapping_flash_name[10] = { "GD25Q20C(2Mbit)", "GD25Q32C(32Mbit)", "MX25U1635E(16Mbit)", "P25Q80H(8Mbit)",
//    "P25Q32H(32Mbit)", "GD25PQ32C(32Mbit)", "FT25F64B(64Mbit)", "GD25Q64EGIG(64Mbit)" };

//flash_dev flash_MXIC = { mxic_check, mxic_erase, mxic_program, mxic_read, mxic_lock, mxic_unlock, FLASH_BLKSIZE,
//    FLASH_CHIPSIZE };
extern int mxic_check(unsigned long base);
extern int mxic_program(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize);
extern int mxic_erase(unsigned long base, unsigned int FlashAddr, unsigned int DataSize);
extern int mxic_read(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize);
extern int mxic_lock(unsigned long base, unsigned int FlashAddr, unsigned int DataSize);
extern int mxic_unlock(unsigned long base, unsigned int FlashAddr, unsigned int DataSize);
extern int mxic_set_wrsr(unsigned long base, unsigned int uiStat);
extern int mxic_enable_qd(unsigned long base);

extern const unsigned long spi_base[3];

static bool w_protect = true;
//extern char __executable_start;

#define NVS_RUN_MODE 0 //(((unsigned long)&__executable_start) & 0x80000000UL)

_EXT_RAM int sflash_init(void *dev)
{
	int ret;
	unsigned long old;

	if (NVS_RUN_MODE)
	{
		disable_GINT();
	}
	ret = platform_init(FLASH_SPI_1_INN, 100);
	if (NVS_RUN_MODE)
	{
		enable_GINT();
	}
	return ret;
}

_EXT_RAM int sflash_init2(void *dev, int spi_enum, int clk)
{
	int ret;
	unsigned long old;
	static int init_flag = 0;
	switch (spi_enum)
	{
	case FLASH_SPI_1_INN:
	case FLASH_SPI_1_EXT:
	case FLASH_SPI_2_PA04_PA07:
	case FLASH_SPI_2_PA08_PA11:
	case FLASH_SPI_3_PA12_PA15:
	case FLASH_SPI_3_PA22_PA25:
	case FLASH_SPI_3_PA31_PB02:
		if (dev)
		{
			((FLASH_DEV *)dev)->spi_enum = spi_enum;
			((FLASH_DEV *)dev)->base_addr = spi_base[spi_enum / 10];
			((FLASH_DEV *)dev)->w_protect = true;
		}
		break;
	default:
		return 1;
		break;
	}
	if (init_flag)
	{
		return 0;
	}

	if (NVS_RUN_MODE)
	{
		disable_GINT();
	}

	ret = platform_init(spi_enum, clk);
	if (NVS_RUN_MODE)
	{
		enable_GINT();
	}
	init_flag = 1;
	return ret;
}

_EXT_RAM void sflash_switch_inn(int clk)
{
	switch_to_inn(clk);
}

_EXT_RAM void sflash_switch_ext(int clk)
{
	switch_to_ext(clk);
}

_EXT_RAM int sflash_read(void *dev, off_t offset, void *data, size_t len)
{
	int ret, spi_enum;
	unsigned long old, base;
	if (dev)
	{
		base = ((FLASH_DEV *)dev)->base_addr;
		spi_enum = ((FLASH_DEV *)dev)->spi_enum;
	}
	else
	{
		base = spi_base[0];
		spi_enum = FLASH_SPI_1_INN;
	}

	if (spi_enum <= FLASH_SPI_1_EXT)
	{
		if (NVS_RUN_MODE)
		{
			disable_GINT();
		}

		ret = mxic_read(base, (unsigned int)offset, (unsigned char *)data, (unsigned int)len);
		if (NVS_RUN_MODE)
		{
			enable_GINT();
		}
	}
	else
	{
		ret = mxic_read(base, (unsigned int)offset, (unsigned char *)data, (unsigned int)len);
	}

	return ret;
}

_EXT_RAM int sflash_write(void *dev, off_t offset, const void *data, size_t len)
{
	int ret, spi_enum;
	unsigned long old, base;
	bool protect;

	if (dev)
	{
		base = ((FLASH_DEV *)dev)->base_addr;
		protect = ((FLASH_DEV *)dev)->w_protect;
		spi_enum = ((FLASH_DEV *)dev)->spi_enum;
	}
	else
	{
		base = spi_base[0];
		protect = w_protect;
		spi_enum = FLASH_SPI_1_INN;
	}
	if (protect == false)
	{
		if (spi_enum <= FLASH_SPI_1_EXT)
		{
			if (NVS_RUN_MODE)
			{
				disable_GINT();
			}
			ret = mxic_program(base, (unsigned int)offset, (unsigned char *)data, (unsigned int)len);
			if (NVS_RUN_MODE)
			{
				enable_GINT();
			}
		}
		else
		{
			ret = mxic_program(base, (unsigned int)offset, (unsigned char *)data, (unsigned int)len);
		}
		return ret;
	}

	return -1;
}

_EXT_RAM int sflash_erase(void *dev, off_t offset, size_t size)
{
	int ret, spi_enum;
	unsigned long old, base;
	bool protect;

	if (dev)
	{
		base = ((FLASH_DEV *)dev)->base_addr;
		protect = ((FLASH_DEV *)dev)->w_protect;
		spi_enum = ((FLASH_DEV *)dev)->spi_enum;
	}
	else
	{
		base = spi_base[0];
		protect = w_protect;
		spi_enum = FLASH_SPI_1_INN;
	}

	if (protect == false)
	{
		if (spi_enum <= FLASH_SPI_1_EXT)
		{
			if (NVS_RUN_MODE)
			{
				disable_GINT();
			}
			ret = mxic_erase(base, (unsigned int)offset, (unsigned int)size);
			if (NVS_RUN_MODE)
			{
				enable_GINT();
			}
		}
		else
		{
			ret = mxic_erase(base, (unsigned int)offset, (unsigned int)size);
		}
		return ret;
	}
	return -1;
}

_EXT_RAM int sflash_write_protection_set(void *dev, bool enable)
{
	if (dev)
	{
		((FLASH_DEV *)dev)->w_protect = enable;
	}
	else
	{
		w_protect = enable;
	}
	return 0;
}

_EXT_RAM size_t sflash_get_write_block_size(void *dev)
{
	return 1;
}

_EXT_RAM int sflash_get_page_info_by_offs(void *dev, off_t offset, struct sflash_pages_info *info)
{
	info->index = offset / SPIROM_SECTOR_SIZE;
	info->size = SPIROM_SECTOR_SIZE;
	info->start_offset = offset & (~(SPIROM_SECTOR_SIZE - 1));
	return 0;
}

_EXT_RAM unsigned int spirom_prepare_cmd(unsigned int cmd, unsigned int addr)
{
	unsigned int b0 = (cmd & 0xff);
	unsigned int b1 = (((addr >> 16) & 0xff) << 8);
	unsigned int b2 = (((addr >> 8) & 0xff) << 16);
	unsigned int b3 = ((addr & 0xff) << 24);
	unsigned int word = (b0 | b1 | b2 | b3);
	return word;
}

_EXT_RAM unsigned int
spirom_cmd_send(unsigned long base, unsigned int cmd, unsigned int addr, unsigned int bytes, unsigned int *pdata, unsigned int *Retdata)
{
	unsigned int spib_dctrl;
	unsigned int op_addr;
	unsigned int data = 0;
	unsigned int spib_busy = 0;
	unsigned int spib_rx_empty = 0;
	/*-- wait if there is active transaction --*/
	spib_busy = spib_wait_spi(base);
	if (spib_busy != 0)
	{
		return 1;
	}
	/*-- clear tx/rx fifo --*/
	spib_clr_fifo(base);

	/*-- prepare opcode and address --*/
	switch (cmd)
	{
	case SPIROM_CMD_READ:
#ifdef FLASH_QUAD_MODE_EN
		if (base == CMN_FLASHC_BASE)
		{
			spib_dctrl = spib_prepare_dctrl2(0x1, 0x1, SPIB_TM_DY_RD, 0, 1, bytes - 1, 1, 2, 1);
			spib_exe_cmmd2(base, SPIROM_OP_QFAST_READ, addr, spib_dctrl);
		}
		else
		{
			spib_dctrl = spib_prepare_dctrl2(0x1, 0x1, SPIB_TM_DY_RD, 0, 0, bytes - 1, 0, 0, 0);
			spib_exe_cmmd2(base, SPIROM_OP_FAST_READ, addr, spib_dctrl);
		}
#else
		op_addr = spirom_prepare_cmd(SPIROM_OP_READ, addr);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 3, 0, bytes - 1);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
#endif
		spib_rx_data(base, pdata, bytes);
		break;
	case SPIROM_CMD_WREN:
		op_addr = SPIROM_OP_WREN;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_WRDI:
		op_addr = SPIROM_OP_WRDI;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_ERASE:
		op_addr = spirom_prepare_cmd(SPIROM_OP_SE, addr);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 3, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_PROGRAM:
#ifdef FLASH_QUAD_MODE_EN
		if (base == CMN_FLASHC_BASE)
		{
			spib_dctrl = spib_prepare_dctrl2(0x1, 0x1, SPIB_TM_WRonly, bytes - 1, 0, 0, 0, 2, 0);
			spib_exe_cmmd2(base, SPIROM_OP_QPP, addr, spib_dctrl);
		}
		else
		{
			op_addr = spirom_prepare_cmd(SPIROM_OP_PP, addr);
			spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 3 + bytes, 0, 0);
			spib_exe_cmmd(base, op_addr, spib_dctrl);
		}
#else
		op_addr = spirom_prepare_cmd(SPIROM_OP_PP, addr);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 3 + bytes, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
#endif
		spib_tx_data(base, pdata, bytes);
		break;
	case SPIROM_CMD_RDID:
		op_addr = SPIROM_OP_RDID;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 0, 0, 2);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		spib_rx_empty = spib_wait_rx_empty(base);
		if (spib_rx_empty == 0)
		{
			data = spib_get_data(base);
		}
		break;
	case SPIROM_CMD_RDST:
		op_addr = SPIROM_OP_RDSR;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		spib_rx_empty = spib_wait_rx_empty(base);
		if (spib_rx_empty == 0)
		{
			data = spib_get_data(base);
		}
		break;
	case SPIROM_CMD_RDST2:
		op_addr = SPIROM_OP_RDSR2;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		spib_rx_empty = spib_wait_rx_empty(base);
		if (spib_rx_empty == 0)
		{
			data = spib_get_data(base);
		}
		break;
	case SPIROM_CMD_LOCK:
		op_addr = spirom_prepare_cmd(SPIROM_OP_SBLK, addr);
		// spib_dctrl = spib_prepare_dctrl (0x0, 0x0, SPIB_TM_WRonly, 0, 0, 0);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 3, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_UNLOCK:
		op_addr = spirom_prepare_cmd(SPIROM_OP_SBULK, addr);
		// spib_dctrl = spib_prepare_dctrl (0x0, 0x0, SPIB_TM_WRonly, 0, 0, 0);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 3, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_RDBLOCK:
		op_addr = spirom_prepare_cmd(SPIROM_OP_RDBLOCK, addr);
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 3, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		spib_rx_empty = spib_wait_rx_empty(base);
		if (spib_rx_empty == 0)
		{
			data = spib_get_data(base);
		}
		break;
	case SPIROM_CMD_RDSCUR:
		op_addr = SPIROM_OP_RDSCUR;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WR_RD, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		spib_rx_empty = spib_wait_rx_empty(base);
		if (spib_rx_empty == 0)
		{
			data = spib_get_data(base);
		}
		break;
	case SPIROM_CMD_CHIP_UNLOCK:
		op_addr = SPIROM_OP_GBULK;
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 0, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_WRSR:
		op_addr = (SPIROM_OP_WRSR | (addr << 8));
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 1, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	case SPIROM_CMD_WRSR2:
		op_addr = (SPIROM_OP_WRSR2 | (addr << 8));
		spib_dctrl = spib_prepare_dctrl(0x0, 0x0, SPIB_TM_WRonly, 1, 0, 0);
		spib_exe_cmmd(base, op_addr, spib_dctrl);
		break;
	default:
		printf("spirom_cmd_send: wrong cmd\n");
		return 1;
		break;
	}
	spib_busy = spib_wait_spi(base);
	//errors
	if (spib_busy || spib_rx_empty)
	{
		return 1;
	}

	*Retdata = data;
	return 0;
}

_EXT_RAM int mxic_unlock_chip(unsigned long base)
{
	unsigned int j, RetData, timeout = FLASH_RETRY_TIMES;
	/*-- write enable --*/
	spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &RetData);

	/*-- get enable status --*/
	for (j = 1; j < timeout; j++)
	{
		spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
		if ((RetData & SPIROM_SR_WIP_MASK) == 0)
			break;
	}
	if ((RetData & SPIROM_SR_WEL_MASK) == 0)
		printf("mxic_unlock_chip: (ULock) status %x, write enable is not set\n", RetData);

	/*-- Chip-UnLock --*/
	spirom_cmd_send(base, SPIROM_CMD_CHIP_UNLOCK, 0x0, 0, NULL, &RetData);

	/*-- get ULock status --*/
	for (j = 1; j < timeout; j++)
	{
		spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
		if ((RetData & SPIROM_SR_WIP_MASK) == 0)
			break;
	}
	if ((RetData & SPIROM_SR_WIP_MASK) != 0)
	{
		printf("mxic_unlock_chip: status %x, ULock is still in progress\n", RetData);
		return 1;
	}
	spirom_cmd_send(base, SPIROM_CMD_RDBLOCK, 0, 0, NULL, &RetData);

	if ((RetData & 0xFF) == 0xFF)
	{
		printf("mxic_unlock_chip: ULock-chip fail\n");
		return 1;
	}
	return 0;
}
// TODO: modify this function to target-specific function - check flash

unsigned int FlashType;
_EXT_RAM int mxic_check(unsigned long base)
{
	unsigned int result, RetData, FlashId, i;
	unsigned char temp[4];
	unsigned int SCLK_DIV = 0xff; //SCLK is the same as the SPI clock source

	RetData = (spib_get_regtiming(base) & (~0xFF));
	spib_set_regtiming(base, RetData | SCLK_DIV);
	SCLK_DIV = spib_get_regtiming(base);
	printf("mxic_check: SCLK_DIV=%x\n", SCLK_DIV);

	result = spirom_cmd_send(base, SPIROM_CMD_RDID, 0x0, 0, NULL, (unsigned int *)temp);
	if (result != 0)
	{
		printf("ERROR: read spi rom id fail\n");
		return 1;
	}

	FlashId = temp[0] << 16 | temp[1] << 8 | temp[2];
	//    for(i = 0; i < 10; i++) {
	//        if(FlashId == mapping_flash_id[i]) {
	//            FlashType = i;
	//            break;
	//        }
	//    }
	//    if(i == 10) {
	//        printf("Flash type is unknown 0x%08x\n", FlashId);
	////        terminate();
	//        return -1;
	//    }
	//    printf("Detect flash is %s\n", mapping_flash_name[FlashType]);
	printf("Flash type is 0x%08x\n", FlashId);
	return 0;
}
// TODO: modify this function to target-specific function - erase flash
_EXT_RAM int mxic_erase(unsigned long base, unsigned int FlashAddr, unsigned int DataSize)
{
	unsigned int EraseAddrStart, EraseSize, EraseSectorCnt, EraseSectorIndex, i, j;
	unsigned int result, RetData, timeout = FLASH_RETRY_TIMES;

	EraseAddrStart = (FlashAddr / SPIROM_SECTOR_SIZE) * SPIROM_SECTOR_SIZE;
	EraseSize = (FlashAddr - EraseAddrStart) + DataSize;
	EraseSectorCnt = (EraseSize + (SPIROM_SECTOR_SIZE - 1)) / SPIROM_SECTOR_SIZE;
	EraseSectorIndex = (EraseAddrStart / SPIROM_SECTOR_SIZE);

	for (i = 0; i < EraseSectorCnt; i++)
	{
		/*---------------------*/
		/*-- ERASE procedure   */
		/*---------------------*/
		for (j = 1; j < timeout; j++)
		{
			/*-- write enable --*/
			result = spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_erase: (erase) enable write fail\n");
				return 1;
			}
			/*-- get enable status --*/
			result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_erase: (erase) get write enable status fail\n");
				return 1;
			}
			if (RetData & SPIROM_SR_BP_MASK)
			{
				printf("mxic_erase: Flash block locked.\n");
				return 1;
			}
			else if (RetData & SPIROM_SR_WEL_MASK)
				break;
		}
		if ((RetData & SPIROM_SR_WEL_MASK) == 0)
		{
			printf("mxic_erase: (erase) status %x, write enable is not set\n", RetData);
			return 1;
		}
		/*-- erase --*/
		result = spirom_cmd_send(base, SPIROM_CMD_ERASE, EraseAddrStart, 0, NULL, &RetData);
		if (result != 0)
		{
			printf("mxic_erase: rom erase fail\n");
			return 1;
		}

		/*-- get erase status --*/
		for (j = 1; j < timeout; j++)
		{
			// if (j >= 2)
			//  printf("mxic_erase: retry-RDST\n");
			result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_erase: get erase status fail\n");
				return 1;
			}
			if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) == 0)
				break;
		}
		if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) != 0)
		{
			printf("mxic_erase: status %x, erase is still in progress\n", RetData);
			return 1;
		}
		EraseAddrStart += SPIROM_SECTOR_SIZE;
		EraseSectorIndex++;
	}
	return 0;
}
// TODO: modify this function to target-specific function - program flash
_EXT_RAM int mxic_program_pages(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize)
{
	unsigned int result, RetData, PageCnt, addr, i; // k;
	unsigned int j, timeout = FLASH_RETRY_TIMES;
	unsigned int g_times = 0;

	if (FlashAddr & (~SPIROM_PAGEPROG_ADDR_MASK))
	{
		//        fprintf(pLogFile, "mxic_program: addr is NOT page-align!!");
		//        fflush(pLogFile);
		return 1;
	}
	PageCnt = (DataSize + (SPIROM_PAGE_SIZE - 1)) / SPIROM_PAGE_SIZE;
	printf("flash program addr: 0x%08x page size=%d\n", FlashAddr, PageCnt);
	/*---------------------------*/
	/*-- PAGE PROGRAM procedure  */
	/*---------------------------*/
	for (i = 0; i < PageCnt; i++)
	{
		/*-- write enable --*/
		for (j = 1; j < timeout; j++)
		{
			// if (j >= 2)
			//  printf("mxic_program: retry-WREN\n");
			result = spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_program: (program) page %d enable write fail\n", i);
				return 1;
			}
			/*-- get enable status --*/
			result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_program: get (program) page %d write enable status fail\n", i);
				return 1;
			}
			if (RetData & SPIROM_SR_BP_MASK)
			{
				printf("mxic_program: Flash block locked.\n");
				return 1;
			}
			else if (RetData & SPIROM_SR_WEL_MASK)
				break;
		}
		if ((RetData & SPIROM_SR_WEL_MASK) == 0)
		{
			printf("mxic_program: (program) page %d, write enable is not set (status %x)\n", i, RetData);
			return 1;
		}

		addr = FlashAddr + (i * SPIROM_PAGE_SIZE);

		// printf("send program cmd......\n");
		result = spirom_cmd_send(base, SPIROM_CMD_PROGRAM, addr, SPIROM_PAGE_SIZE, (unsigned int *)start, &RetData);
		if (result != 0)
		{
			printf("mxic_program: (program) page %d fail\n", i);
			return 1;
		}
		/*-- ckeck completion --*/
		for (j = 1; j < timeout; j++)
		{
			result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
			if (result != 0)
			{
				printf("mxic_program: get (program) page %d write enable status fail\n", i);
				return 1;
			}
			if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) == 0)
				break;
		}
		if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) != 0)
		{
			printf("mxic_program: (program) page %d, write enable is not clear (status %x)\n", i, RetData);
			return 1;
		}

		g_times++;

		start += SPIROM_PAGE_SIZE;
	} /* for (i = 0; i < PageCnt; i++) */
	return 0;
}

//only handle program in a page
_EXT_RAM int _mxic_program(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize)
{
	unsigned int result, RetData, MaxSize, *pdata; // k;
	unsigned int j, timeout = FLASH_RETRY_TIMES;
	unsigned char data[SPIROM_PAGE_SIZE] __attribute__((aligned(4)));

	MaxSize = ((FlashAddr + SPIROM_PAGE_SIZE) & SPIROM_PAGEPROG_ADDR_MASK) - FlashAddr;
	if (DataSize > MaxSize)
	{
		//too many data to write
		return 1;
	}
	/*---------------------------*/
	/*-- PAGE PROGRAM procedure  */
	/*---------------------------*/
	/*-- write enable --*/
	for (j = 1; j < timeout; j++)
	{
		// if (j >= 2)
		//  printf("mxic_program: retry-WREN\n");
		result = spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &RetData);
		if (result != 0)
		{
			printf("mxic_program: (program) page %d enable write fail\n", i);
			return 1;
		}
		/*-- get enable status --*/
		result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
		if (result != 0)
		{
			printf("mxic_program: get (program) page %d write enable status fail\n", i);
			return 1;
		}
		if (RetData & SPIROM_SR_BP_MASK)
		{
			printf("mxic_program: Flash block locked.\n");
			return 1;
		}
		else if (RetData & SPIROM_SR_WEL_MASK)
			break;
	}
	if ((RetData & SPIROM_SR_WEL_MASK) == 0)
	{
		printf("mxic_program: (program) page %d, write enable is not set (status %x)\n", i, RetData);
		return 1;
	}

	if ((unsigned int)start & 0x3)
	{
		//data address is not 4 bytes aligned
		memcpy(data, start, DataSize);
		pdata = (unsigned int *)data;
	}
	else
	{
		pdata = (unsigned int *)start;
	}
	// printf("send program cmd......\n");
	result = spirom_cmd_send(base, SPIROM_CMD_PROGRAM, FlashAddr, DataSize, pdata, &RetData);
	if (result != 0)
	{
		printf("mxic_program: (program) page %d fail\n", i);
		return 1;
	}
	/*-- ckeck completion --*/
	for (j = 1; j < timeout; j++)
	{
		result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
		if (result != 0)
		{
			printf("mxic_program: get (program) page %d write enable status fail\n", i);
			return 1;
		}
		if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) == 0)
			break;
	}
	if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) != 0)
	{
		printf("mxic_program: (program) page %d, write enable is not clear (status %x)\n", i, RetData);
		return 1;
	}

	return 0;
}

_EXT_RAM int mxic_program(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize)
{
	unsigned int result, addr, i, FirstWrite, LastWrite, MidWrite; // k;
	unsigned char *data = start;

	//size of writing in the first page
	FirstWrite = ((FlashAddr + SPIROM_PAGE_SIZE) & SPIROM_PAGEPROG_ADDR_MASK) - FlashAddr;
	if (DataSize > FirstWrite)
	{
		//size of writing in the middle pages
		MidWrite = (DataSize - FirstWrite) & SPIROM_PAGEPROG_ADDR_MASK;
	}
	else
	{
		//all data in one page
		MidWrite = 0;
		FirstWrite = DataSize;
	}
	if (DataSize > FirstWrite + MidWrite)
	{
		//size of writing in the last page
		LastWrite = DataSize - FirstWrite - MidWrite;
	}
	else
	{
		LastWrite = 0;
	}

	//program
	addr = FlashAddr;
	if (FirstWrite)
	{
		result = _mxic_program(base, addr, data, FirstWrite);
		addr += FirstWrite;
		data += FirstWrite;
		if (result)
		{
			goto ERR;
		}
	}
	if (MidWrite)
	{
		for (i = 0; i < MidWrite; i += SPIROM_PAGE_SIZE)
		{
			result = _mxic_program(base, addr, data, SPIROM_PAGE_SIZE);
			addr += SPIROM_PAGE_SIZE;
			data += SPIROM_PAGE_SIZE;
			if (result)
			{
				goto ERR;
			}
		}
	}
	if (LastWrite)
	{
		result = _mxic_program(base, addr, data, LastWrite);
		if (result)
		{
			goto ERR;
		}
	}

	return 0;
ERR:
	return 1;
}

// TODO: modify this function to target-specific function - read flash
_EXT_RAM int _mxic_read(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize)
{
	unsigned int result, RetData, CurrSize;
	/*-- SPIB_DCTRL_RCNT_MASK(0x1ff) --*/
	while (DataSize)
	{
		if (DataSize >= 0x200)
			CurrSize = 0x200;
		else
			CurrSize = DataSize;
		result = spirom_cmd_send(base, SPIROM_CMD_READ, FlashAddr, CurrSize, (unsigned int *)start, &RetData);
		if (result != 0)
		{
			printf("Flash_Read: fail\n");
			return 1;
		}
		FlashAddr += CurrSize;
		start += CurrSize;
		DataSize -= CurrSize;
	}
	return 0;
}

_EXT_RAM int mxic_read(unsigned long base, unsigned int FlashAddr, unsigned char *start, unsigned int DataSize)
{
	unsigned int result, RetData, FirstRead, MidRead, LastRead, addr;
	unsigned char data[4] __attribute__((aligned(4)));
	unsigned char *pdata = start;

	//handle non 4 bytes aligned buffer and size
	FirstRead = 4 - ((unsigned int)start & 0x3);
	if (FirstRead == 4)
	{
		FirstRead = 0;
	}
	if (DataSize > FirstRead)
	{
		LastRead = (DataSize - FirstRead) & 0x3;
		MidRead = DataSize - FirstRead - LastRead;
	}
	else
	{
		FirstRead = DataSize;
		MidRead = 0;
		LastRead = 0;
	}
	addr = FlashAddr;
	if (FirstRead)
	{
		//non 4 bytes aligned read
		result = spirom_cmd_send(base, SPIROM_CMD_READ, addr, FirstRead, (unsigned int *)data, &RetData);
		memcpy(pdata, data, FirstRead);
		pdata += FirstRead;
		addr += FirstRead;
		if (result)
		{
			goto ERR;
		}
	}
	if (MidRead)
	{
		//4 bytes aligned read
		result = _mxic_read(base, addr, pdata, MidRead);
		pdata += MidRead;
		addr += MidRead;
		if (result)
		{
			goto ERR;
		}
	}
	if (LastRead)
	{
		//non 4 bytes aligned read
		result = spirom_cmd_send(base, SPIROM_CMD_READ, addr, LastRead, (unsigned int *)data, &RetData);
		memcpy(pdata, data, LastRead);
		if (result)
		{
			goto ERR;
		}
	}

	return 0;
ERR:
	return 1;
}

// TODO: modify this function to target-specific function - lock flash
_EXT_RAM int mxic_lock(unsigned long base, unsigned int FlashAddr, unsigned int DataSize)
{
#if INDIVIDUAL_BLK_PROTECT
#else
	unsigned int RetData = 0;

	spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
	mxic_set_wrsr(base, RetData | 0x3C);
	spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
	printf("mxic_lock: status = %08x \n", RetData);
#endif
	return 0;
}
// TODO: modify this function to target-specific function - unlock flash
_EXT_RAM int mxic_unlock(unsigned long base, unsigned int FlashAddr, unsigned int DataSize)
{
#if INDIVIDUAL_BLK_PROTECT
#else
	unsigned int RetData = 0;

	spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
	mxic_set_wrsr(base, RetData & ~0x3C);
	spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
	printf("mxic_unlock: status = %08x \n", RetData);
#endif
	return 0;
}

_EXT_RAM int mxic_set_wrsr(unsigned long base, unsigned int uiStat)
{
	unsigned int i, j, RetData, timeout = FLASH_RETRY_TIMES;

	for (i = 0; i < timeout; i++)
	{
		/*-- write enable --*/
		spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &RetData);
		/*-- get enable status --*/
		for (j = 0; j < timeout; j++)
		{
			spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
			if ((RetData & SPIROM_SR_WIP_MASK) == 0)
				break;
		}
		if (RetData & SPIROM_SR_WEL_MASK)
			break;
	}
	/*-- set WRSR --*/
	spirom_cmd_send(base, SPIROM_CMD_WRSR, uiStat, 0, NULL, &RetData);

	/*-- get status --*/
	for (j = 0; j < timeout; j++)
	{
		spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &RetData);
		if ((RetData & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) == 0)
			break;
	}
	// printf("mxic_set_wrsr: uiStat=%x RetData=%x\n", uiStat, RetData);
	return 0;
}

_EXT_RAM int mxic_enable_qd(unsigned long base)
{
	unsigned int i, j;
	unsigned int result, buf1, buf2;

	//	result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &buf1);
	result = spirom_cmd_send(base, SPIROM_CMD_RDST2, 0x0, 0, NULL, &buf2);
	if (result)
	{
		//error
		return 1;
	}
	if (buf2 & 0x2)
	{ //if it is in QE mode, return
		return 0;
	}
	else
	{
		buf2 |= 0x2;
		for (i = 0; i < FLASH_RETRY_TIMES; i++)
		{
			/*-- write enable --*/
			spirom_cmd_send(base, SPIROM_CMD_WREN, 0x0, 0, NULL, &buf1);
			/*-- get enable status --*/
			for (j = 0; j < FLASH_RETRY_TIMES; j++)
			{
				spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &buf1);
				if ((buf1 & SPIROM_SR_WIP_MASK) == 0)
					break;
			}
			if (buf1 & SPIROM_SR_WEL_MASK)
				break;
		}

		spirom_cmd_send(base, SPIROM_CMD_WRSR2, buf2, 0, NULL, &buf1);
		/*-- get enable status --*/
		for (j = 0; j < FLASH_RETRY_TIMES; j++)
		{
			result = spirom_cmd_send(base, SPIROM_CMD_RDST, 0x0, 0, NULL, &buf1);
			if (result)
			{
				return 1;
			}
			if ((buf1 & (SPIROM_SR_WIP_MASK | SPIROM_SR_WEL_MASK)) == 0)
				break;
		}
	}
	return 0;
}
