/*
 * cache_cp.c
 *
 *  Created on: Jul 10, 2020
 *
 */

#include "hal/cache.h"
#include "hal/venus_cp.h"


// check if memory address range (start ~ end) is cacheable
// NOTE: end = start + size, and the 'end' address is excluding from the range
int range_is_cacheable(unsigned long start, unsigned long size)
{
    //TODO:
    return 1; //FIXME:
}


// write back from cache to memory
void dcache_clean_range(unsigned long start, unsigned long end)
{
    uint32_t line_mask = XCHAL_DCACHE_LINESIZE - 1;
    start = start & (~line_mask);
    end = (end + line_mask) & (~line_mask);
    xthal_dcache_region_writeback((void*)start, (end - start));
}


// invalidate data in cache (NOT write back to memory)
void dcache_invalidate_range(unsigned long start, unsigned long end)
{
    uint32_t line_mask = XCHAL_DCACHE_LINESIZE - 1;
    start = start & (~line_mask);
    end = (end + line_mask) & (~line_mask);
    xthal_dcache_region_invalidate((void*)start, (end - start));
 }


// write back from cache to memory and then invalidate data in cache
void dcache_flush_range(unsigned long start, unsigned long end)
{
    uint32_t line_mask = XCHAL_DCACHE_LINESIZE - 1;
    start = start & (~line_mask);
    end = (end + line_mask) & (~line_mask);
    xthal_dcache_region_writeback_inv((void*)start, (end - start));
}


// usually called before transferring data to memory (NOT cache-line-aligned) via DMA
void cache_dma_fast_inv_stage1(unsigned long start, unsigned long end)
{
    unsigned long line_size;
    unsigned long old_start = start;
    unsigned long old_end = end;
    line_size = XCHAL_DCACHE_LINESIZE;
    start = start & (~(line_size - 1));
    end = (end + line_size - 1) & (~(line_size - 1));
    if (start == end)
        return;
    if (start != old_start) {
        xthal_dcache_region_writeback((void *)start, line_size);
    }
    if (end != old_end) {
        xthal_dcache_region_writeback((void *)(end - line_size), line_size);
    }
    xthal_dcache_region_invalidate((void *)start, (end - start));
}

void unaligned_cache_line_move(unsigned char* src, unsigned char* dst, unsigned long len)
{
    int i;
    unsigned char* src_p = (unsigned char*) src;
    unsigned char* dst_p = (unsigned char*) dst;
    for (i = 0; i < len; ++i) {
        *(dst_p + i) = *(src_p + i);
    }
}

// usually called after transferring data to memory  (NOT cache-line-aligned) via DMA
void cache_dma_fast_inv_stage2(unsigned long start, unsigned long end)
{
    unsigned long line_size;
    unsigned long old_start = start;
    unsigned long old_end = end;
    static unsigned char cache_line_buf[XCHAL_DCACHE_LINESIZE];

    line_size = XCHAL_DCACHE_LINESIZE;
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
        xthal_dcache_region_invalidate((void *)start, line_size);
        unaligned_cache_line_move(cache_line_buf, (unsigned char*) start, old_start - start);
    }
    if (end != old_end) {
        unaligned_cache_line_move((unsigned char*) old_end, cache_line_buf, end - old_end);
        xthal_dcache_region_invalidate((void *)(end - line_size), line_size);
        unaligned_cache_line_move(cache_line_buf, (unsigned char*) old_end, end - old_end);
    }

    if (use_lock) {
        enable_GINT();
    }
}
