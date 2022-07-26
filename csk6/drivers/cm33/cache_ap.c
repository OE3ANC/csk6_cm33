/*
 * cache_ap.c
 *
 *  Created on: Jul 10, 2020
 *
 */
#include <assert.h>
#include "toolchain/gcc.h"
#include "ARMCM33_DSP_FP.h"
#include "venus_ap.h"
#include "cachel1_armv7.h"
#include "hal/cache.h"

#define NO_CACHE_DEBUG   0

// check if memory address range (start ~ end) is cacheable
// NOTE: end = start + size, and the 'end' address is excluding from the range
int range_is_cacheable(unsigned long start, unsigned long size)
{
    //FIXME: Now we just simply check cacheable attribute by the memory arrange
    // on specific platform, and it's a rough implementation...
    // Actually we SHOULD check it based MPU registers. Please refer to mpu.c...

    // FLASH address space 1, size 8M BYTES
    #define flash_start_addr1    0x18000000
    #define flash_end_addr1      0x187fffff

    // FLASH address space 2, size 8M BYTES
    #define flash_start_addr2    0x38000000
    #define flash_end_addr2      0x387fffff

    // PSRAM address space, size 8M BYTES
    #define psram_start_addr    0x30000000
    #define psram_end_addr      0x307fffff

    #define addr_range_count    3

    static const uint32_t addr_ranges[] = {
            flash_start_addr1, (flash_end_addr1 + 1),
            flash_start_addr2, (flash_end_addr2 + 1),
            psram_start_addr, (psram_end_addr + 1)
    };

    uint32_t i, j, end;

    assert(size > 0);
    end = start + size - 1;
    for (i = 0; i < addr_range_count; i++) {
        j = i << 1;
        if (start >= addr_ranges[j] && start < addr_ranges[j+1])
            return 1;
        if (end >= addr_ranges[j] && end < addr_ranges[j+1])
            return 1;
    }
    return 1;
}


// write back from cache to memory
_FAST_FUNC_RO
void dcache_clean_range(unsigned long start, unsigned long end)
{
#if (NO_CACHE_DEBUG == 0)
	uint32_t line_mask = __SCB_DCACHE_LINE_SIZE - 1;
	start = start & (~line_mask);
	end = (end + line_mask) & (~line_mask);
    SCB_CleanDCache_by_Addr((uint32_t *)start, (end - start));
#endif
}


// invalidate data in cache (NOT write back to memory)
void dcache_invalidate_range(unsigned long start, unsigned long end)
{
#if (NO_CACHE_DEBUG == 0)
	uint32_t line_mask = __SCB_DCACHE_LINE_SIZE - 1;
	assert((start & line_mask) == 0);
	assert((end & line_mask) == 0);
	ARG_UNUSED(line_mask);
	SCB_InvalidateDCache_by_Addr((uint32_t *)start, (end - start));
#endif
}


// write back from cache to memory and then invalidate data in cache
void dcache_flush_range(unsigned long start, unsigned long end)
{
#if (NO_CACHE_DEBUG == 0)
	uint32_t line_mask = __SCB_DCACHE_LINE_SIZE - 1;
	start = start & (~line_mask);
	end = (end + line_mask) & (~line_mask);
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)start, (end - start));
#endif
}

// usually called before transferring data to memory (NOT cache-line-aligned) via DMA
_FAST_FUNC_RO
void cache_dma_fast_inv_stage1(unsigned long start, unsigned long end)
{
#if (NO_CACHE_DEBUG == 0)
	unsigned long line_size;
	unsigned long old_start = start;
	unsigned long old_end = end;
	line_size = __SCB_DCACHE_LINE_SIZE;
	start = start & (~(line_size - 1));
	end = (end + line_size - 1) & (~(line_size - 1));
	if (start == end)
		return;
	if (start != old_start) {
		SCB_CleanDCache_by_Addr((uint32_t *)start, line_size);
	}
	if (end != old_end) {
		SCB_CleanDCache_by_Addr((uint32_t *)(end - line_size), line_size);
	}
	SCB_InvalidateDCache_by_Addr((uint32_t *)start, (end - start));
#endif
}

void unaligned_cache_line_move(unsigned char* src, unsigned char* dst, unsigned long len)
{
#if (NO_CACHE_DEBUG == 0)
    int i;
    unsigned char* src_p = (unsigned char*) src;
    unsigned char* dst_p = (unsigned char*) dst;
    for (i = 0; i < len; ++i) {
        *(dst_p + i) = *(src_p + i);
    }
#endif
}

// usually called after transferring data to memory  (NOT cache-line-aligned) via DMA
_FAST_FUNC_RO
void cache_dma_fast_inv_stage2(unsigned long start, unsigned long end)
{
#if (NO_CACHE_DEBUG == 0)
	unsigned long line_size;
	unsigned long old_start = start;
	unsigned long old_end = end;
	static unsigned char cache_line_buf[32];

	line_size = __SCB_DCACHE_LINE_SIZE;
	start = start & (~(line_size - 1));
	end = (end + line_size - 1) & (~(line_size - 1));
	if (start == end)
		return;

	int use_lock = 0;
	// interrupt enabled, and not in interrupt context

	if(start != old_start || end != old_end)
		use_lock = 1;

	if (use_lock) {
		hal_global_irq_disable();
	}

	if (start != old_start) {
		unaligned_cache_line_move((unsigned char*) start, cache_line_buf, old_start - start);
		SCB_InvalidateDCache_by_Addr((uint32_t *)start, line_size);
		unaligned_cache_line_move(cache_line_buf, (unsigned char*) start, old_start - start);
	}
	if (end != old_end) {
		unaligned_cache_line_move((unsigned char*) old_end, cache_line_buf, end - old_end);
		SCB_InvalidateDCache_by_Addr((uint32_t *)(end - line_size), line_size);
		unaligned_cache_line_move(cache_line_buf, (unsigned char*) old_end, end - old_end);
	}

	if (use_lock) {
		hal_global_irq_enable();
	}
#endif
}
