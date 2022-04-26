#ifndef __FLASHROM__
#define __FLASHROM__
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
//#include "types.h"

#define GD25Q20C_ID 0xc84012
#define GD25Q32C_ID 0xc84016
#define MX25U1635E_ID 0xc22535
#define P25Q80H_ID 0x856014
#define P25Q32H_ID 0x856016
#define GD25PQ32C_ID 0xd84016
#define FT25F64B_ID 0xb4017
#define GD25Q64EGIG_ID 0xc84017

typedef struct FLASH_DEV {
	unsigned long	base_addr;
	int				spi_enum;
	bool		    w_protect;
}FLASH_DEV;

typedef enum {
	FLASH_SPI_1_INN = 0,
	FLASH_SPI_1_EXT,
	FLASH_SPI_2_PA04_PA07 = 10,
	FLASH_SPI_2_PA08_PA11,
	FLASH_SPI_3_PA12_PA15 = 20,
	FLASH_SPI_3_PA22_PA25,
	FLASH_SPI_3_PA31_PB02
}FLASH_ENUM;

/**
 *  @brief  initial spi flash device, default spi enum is FLASH_SPI_1_INN
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_init(void);

/**
 *  @brief  initial spi flash device
 *  @param  dev             : flash dev (output)
 *  @param  spi_enum        : spi enum (input)
 *  @param  clk             : clock of flash, unit is MHz. eg. 100
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_init2(void *dev, int spi_enum, int clk);

/**
 *  @brief  flash switch select inner
 *  @param  clk             : clock of flash, unit is MHz. eg. 100
 *
 *  @return  none.
 */
void sflash_switch_inn(int clk);

/**
 *  @brief  flash switch select external
 *  @param  clk             : clock of flash, unit is MHz. eg. 100
 *
 *  @return  none.
 */
void sflash_switch_ext(int clk);

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
int sflash_read(void *dev, off_t offset, void *data, size_t len);

/**
 *  @brief  Write buffer into flash memory.
 *
 *  Prior to the invocation of this API, the sflash_write_protection_set needs
 *  to be called first to disable the write protection.
 *
 *  @param  dev             : flash device
 *  @param  offset          : starting offset for the write
 *  @param  data            : data to write
 *  @param  len             : Number of bytes to write
 *
 *  @return  0 on success, negative errno code on fail.
 */
int sflash_write(void *dev, off_t offset, const void *data, size_t len);

/**
 *  @brief  Erase part or all of a flash memory
 *
 *  Acceptable values of erase size and offset are subject to
 *  hardware-specific multiples of page size and offset. Please check
 *  the API implemented by the underlying sub driver, for example by
 *  using sflash_get_page_info_by_offs() if that is supported by your
 *  flash driver.
 *
 *  Prior to the invocation of this API, the sflash_write_protection_set needs
 *  to be called first to disable the write protection.
 *
 *  @param  dev             : flash device
 *  @param  offset          : erase area starting offset
 *  @param  size            : size of area to be erased
 *
 *  @return  0 on success, negative errno code on fail.
 *
 *  @see sflash_get_page_info_by_offs()
 *  @see sflash_get_page_info_by_idx()
 */
int sflash_erase(void *dev, off_t offset, size_t size);

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
int sflash_write_protection_set(void *dev, bool enable);

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
size_t sflash_get_write_block_size(void *dev);

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
int sflash_get_page_info_by_offs(void *dev, off_t offset,
								 struct sflash_pages_info *info);

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
/**
 *  @brief  Get the size and start offset of flash page of certain index.
 *
 *  @param  dev flash device
 *  @param  page_index Index of the page. Index are counted from 0.
 *  @param  info Page Info structure to be filled
 *
 *  @return  0 on success, -EINVAL  if page of the index doesn't exist.
 */
// __syscall int sflash_get_page_info_by_idx(struct device *dev, uint32_t page_index,
// 					 struct sflash_pages_info *info);

/**
 *  @brief  Get the total number of flash pages.
 *
 *  @param  dev flash device
 *
 *  @return  Number of flash pages.
 */
// __syscall size_t sflash_get_page_count(struct device *dev);

/**
 * @brief Callback type for iterating over flash pages present on a device.
 *
 * The callback should return true to continue iterating, and false to halt.
 *
 * @param info Information for current page
 * @param data Private data for callback
 * @return True to continue iteration, false to halt iteration.
 * @see sflash_page_foreach()
 */
//typedef bool (*sflash_page_cb)(const struct sflash_pages_info *info, void *data);

/**
 * @brief Iterate over all flash pages on a device
 *
 * This routine iterates over all flash pages on the given device,
 * ordered by increasing start offset. For each page, it invokes the
 * given callback, passing it the page's information and a private
 * data object.
 *
 * @param dev Device whose pages to iterate over
 * @param cb Callback to invoke for each flash page
 * @param data Private data for callback function
 */
//void sflash_page_foreach(struct device *dev, sflash_page_cb cb, void *data);
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

#endif
