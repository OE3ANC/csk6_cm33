/*
 * cache_ap.c
 *
 *  Created on: Jul 10, 2020
 *
 */
#include <assert.h>
#include "hal/ARMCM33_DSP_FP.h"
#include "hal/venus_ap.h"
#include "cachel1_armv7.h"

#define NO_CACHE_DEBUG   0

// check if memory address range (start ~ end) is cacheable
// NOTE: end = start + size, and the 'end' address is excluding from the range
int range_is_cacheable(unsigned long start, unsigned long size)
{
    //TODO:
    return 1;
}


// write back from cache to memory
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
	(void)line_mask; /* Prevent gcc unused warning */
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
		disable_GINT();
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
		enable_GINT();
	}
#endif
}
