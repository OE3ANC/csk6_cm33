#ifndef __FLASHROM__
#define __FLASHROM__
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
// #include "types.h"

/* 4 bytes address mode */
#define EXT_ADDR4_MODE

typedef enum
{
	RUN_WITH_INT = 0, // run with interrupt
	RUN_WITHOUT_INT	  // run without interrupt
} RUN_MOD;

typedef struct FLASH_DEV
{
	unsigned long base_addr;
	unsigned char d_width; // 1, 2, 4
	unsigned char sclk_div;
	unsigned char run_mod;
	unsigned char w_protect;
	unsigned long timeout;
	unsigned int ext_addr4_mode;
} FLASH_DEV;

typedef enum
{
	FLASH_SPI_1_INN = 0,
	FLASH_SPI_1_EXT,
	FLASH_SPI_2_PA04_PA07 = 10,
	FLASH_SPI_2_PA08_PA11,
	FLASH_SPI_3_PA12_PA15 = 20,
	FLASH_SPI_3_PA22_PA25,
	FLASH_SPI_3_PA31_PB02
} FLASH_ENUM;

#define INDIVIDUAL_BLK_PROTECT 0
/*--------------------------------------------*/
/* SPI Flash ROM definition                   */
/*--------------------------------------------*/
#define SPIROM_ID_VERSION 0x1420c2 /* MXIC 25L8006  */
/* #define SPIROM_ID_VERSION   0x1520c2   MXIC 25L1606  */
#define SPIROM_ID_MASK 0x00ffffff
#define SPIROM_OP_READ 0x03			/*-- read --*/
#define SPIROM_OP_QFAST_READ 0xeb	/*-- quad read in higher speed --*/
#define SPIROM_OP_QFAST_READA4 0xec /*-- quad read in higher speed --*/
#define SPIROM_OP_FAST_READ 0x0b	/*-- read in higher speed --*/
#define SPIROM_OP_FAST_READA4 0x0c	/*-- read in higher speed --*/
#define SPIROM_OP_DFAST_READ 0xbb	/*-- dual read in higher speed --*/
#define SPIROM_OP_DFAST_READA4 0xbc /*-- dual read in higher speed --*/
#define SPIROM_OP_RDID 0x9f			/*-- manufacturer/device ID read --*/
#define SPIROM_OP_READ_ID 0x90		/*-- manufacturer/device ID read --*/
#define SPIROM_OP_WREN 0x06			/*-- write enable --*/
#define SPIROM_OP_WRDI 0x04			/*-- write disable --*/
#define SPIROM_OP_PE 0x81			/*-- page erase --*/
#define SPIROM_OP_SE 0x20			/*-- sector erase --*/
#define SPIROM_OP_SEA4 0x21			/*-- sector erase --*/
#define SPIROM_OP_BE 0x52			/*-- block erase 32K --*/
#define SPIROM_OP_BEA4 0x5c			/*-- block erase 32K --*/
#define SPIROM_OP_BE2 0xd8			/*-- block erase 64K --*/
#define SPIROM_OP_BE2A4 0xdc		/*-- block erase 64K --*/
#define SPIROM_OP_PP 0x02			/*-- page program --*/
#define SPIROM_OP_PPA4 0x12			/*-- page program --*/
#define SPIROM_OP_QPP 0x32			/*-- quad page program --*/
#define SPIROM_OP_QPPA4 0x34		/*-- quad page program --*/
#define SPIROM_OP_RDSR 0x05			/*-- read status register --*/
#define SPIROM_OP_RDSR2 0x35		/*-- read status register2 --*/
#define SPIROM_OP_RDSR3 0x15		/*-- read status register3 --*/
#define SPIROM_OP_WRSR 0x01			/*-- write status register --*/
#define SPIROM_OP_WRSR2 0x31		/*-- write status register2 --*/
#define SPIROM_OP_WRSR3 0x11		/*-- write status register3 --*/
#define SPIROM_OP_SBLK 0x36			/*-- single block lock --*/
#define SPIROM_OP_SBULK 0x39		/*-- single block unlock --*/
#define SPIROM_OP_RDBLOCK 0x3C
#define SPIROM_OP_RDSCUR 0x2B
#define SPIROM_OP_WPSEL 0x68
#define SPIROM_OP_GBLK 0x7E
#define SPIROM_OP_GBULK 0x98

/*-- bit mask for status register --*/
#define SPIROM_SR_WIP_MASK 0x01	   /*-- write in progress --*/
#define SPIROM_SR_WEL_MASK 0x02	   /*-- write enable --*/
#define SPIROM_SR_BP_MASK 0x3C	   /*-- BP protection mode --*/
#define SPIROM_PAGE_SIZE 0x100	   /*-- page size in bytes --*/
#define SPIROM_SECTOR_SIZE 0x1000  /*-- sector size in bytes --*/
#define SPIROM_BLK32_SIZE 0x8000   /*-- block 32K in bytes --*/
#define SPIROM_BLK64_SIZE 0x10000  /*-- block 64K in bytes --*/
#define SPIROM_SECTOR_MASK 0xfffUL /*-- sector 4K mask --*/
#define SPIROM_BLK32_MASK 0x7fffUL /*-- block 32K mask --*/
#define SPIROM_BLK64_MASK 0xffffUL /*-- block 64K mask --*/
#define SPIROM_PAGE_SIZE 0x100	   /*-- page size --*/
#define SPIROM_PAGE_MASK 0xFF

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
#define SPIROM_CMD_ERASE_B32 0x10
#define SPIROM_CMD_ERASE_B64 0x11
#define SPIROM_CMD_RDST3 0x12
#define SPIROM_CMD_WRSR3 0x13
#define SPIROM_CMD_ERASE_PG 0x14

#define FLASH_BLKSIZE SPIROM_SECTOR_SIZE * 16
#define FLASH_RETRY_TIMES 180000

/**
 *  @brief  initial spi flash device, default spi enum is FLASH_SPI_1_INN
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_init(FLASH_DEV *dev);

/**
 *  @brief  initial spi flash device
 *
 *  @return  0 on success, negative errno code on fail.
 */
int flash_init(FLASH_DEV *dev, unsigned char udc0, unsigned char udc1);

/**
 *  @brief  Read data from flash
 *
 *  @param  dev             : flash dev
 *  @param  offset          : Offset (byte aligned) to read
 *  @param  data            : Buffer to store read data
 *  @param  len             : Number of bytes to read.
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_read(FLASH_DEV *dev, off_t offset, void *data, size_t len);

/**
 *  @brief  Write buffer into flash memory.
 *
 *  Prior to the invocation of this API, the flash_write_protection_set needs
 *  to be called first to disable the write protection.
 *
 *  @param  dev             : flash device
 *  @param  offset          : starting offset for the write
 *  @param  data            : data to write
 *  @param  len             : Number of bytes to write
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_write(FLASH_DEV *dev, off_t offset, const void *data, size_t len);

/**
 *  @brief  Erase part or all of a flash memory
 *
 *  Acceptable values of erase size and offset are subject to
 *  hardware-specific multiples of page size and offset. Please check
 *  the API implemented by the underlying sub driver, for example by
 *  using flash_get_page_info_by_offs() if that is supported by your
 *  flash driver.
 *
 *  Prior to the invocation of this API, the flash_write_protection_set needs
 *  to be called first to disable the write protection.
 *
 *  @param  dev             : flash device
 *  @param  offset          : erase area starting offset
 *  @param  size            : size of area to be erased
 *
 *  @return  0 on success, negative errno code on fail.
 *
 *  @see flash_get_page_info_by_offs()
 *  @see flash_get_page_info_by_idx()
 */
int sflash_erase(FLASH_DEV *dev, off_t offset, size_t size);

/**
 *  @brief  Erase part or all of a flash memory
 *
 *  Acceptable values of erase size and offset are subject to
 *  hardware-specific multiples of page size and offset.
 *  Prior to the invocation of this API, the flash_write_protection_set needs
 *  to be called first to disable the write protection.
 *
 *  @param  dev             : flash device
 *  @param  offset          : erase area starting offset
 *  @param  size            : size of area to be erased
 *
 *  @return  0 on success, negative errno code on fail.
 */
int flash_erase_page(FLASH_DEV *dev, off_t offset, size_t size);

/**
 *  @brief  Enable or disable write protection for a flash memory
 *
 *  This API is required to be called before the invocation of write or erase
 *  API. Please note that on some flash components, the write protection is
 *  automatically turned on again by the device after the completion of each
 *  write or erase calls. Therefore, on those flash parts, write protection needs
 *  to be disabled before each invocation of the write or erase API. Please refer
 *  to the sub-driver API or the data sheet of the flash component to get details
 *  on the write protection behavior.
 *
 *  @param  dev             : flash device
 *  @param  enable          : enable or disable flash write protection
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_write_protection_set(FLASH_DEV *dev, bool enable);

/**
 *  @brief  Get the minimum write block size supported by the driver
 *
 *  The write block size supported by the driver might differ from the write
 *  block size of memory used because the driver might implements write-modify
 *  algorithm.
 *
 *  @param  dev flash device
 *
 *  @return  write block size in bytes.
 */
size_t sflash_get_write_block_size(FLASH_DEV *dev);

struct sflash_pages_info
{
	off_t start_offset; /* offset from the base of flash address */
	size_t size;
	uint32_t index;
};

/**
 *  @brief  Get the size and start offset of flash page at certain flash offset.
 *
 *  @param  dev flash device
 *  @param  offset Offset within the page
 *  @param  info Page Info structure to be filled
 *
 *  @return  0 on success, -EINVAL if page of the offset doesn't exist.
 */
int sflash_get_page_info_by_offs(FLASH_DEV *dev, off_t offset,
								 struct sflash_pages_info *info);

int spirom_cmd_send(FLASH_DEV *dev, unsigned int cmd, unsigned int addr,
					unsigned int bytes, unsigned int *pdata, unsigned int *Retdata);

int platform_init(FLASH_DEV *dev, unsigned char udc0, unsigned char udc1);

#endif
