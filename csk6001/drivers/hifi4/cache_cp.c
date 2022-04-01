/*
 * cache_cp.c
 *
 *  Created on: Jul 10, 2020
 *
 */

#include "cache.h"
#include "venus_cp.h"
#include <xtensa/config/core.h>

//typedef struct {
//    unsigned long start_addr; // start address of memory region
//    unsigned long max_size; // available maximum size of address range
//    unsigned long cfg_size; // actually configured size of memory
//} MEM_RGN;
//
//static const MEM_RGN mem_rgn_list[] = {
//    { 0x20080000, 512*1024, 320*1024 }, // non-cacheable, same memory with AP SYS
//    { 0x48000000, 768*1024, 768*1024 }, // non-cacheable, same memory as Data RAM 1
//    { 0x5FDE0000, 128*1024, 16*1024 }, // non-cacheable, HiFi4 Data RAM 0
//    { 0x5FE00000, 1024*1024, 768*1024 }, // non-cacheable, HiFi4 Data RAM 1
//    { 0x5FF00000, 128*1024, 16*1024 }, // non-cacheable, HiFi4 Instruction RAM 0
//    { 0x60000000, 128*1024*1024, 8*1024*1024 }, // cacheable, PSRAM
//    { 0x68000000, 128*1024*1024, 8*1024*1024 }, // cacheable, FLASH
//};
//
//#define PSRAM_RGN_INX   4
//#define FLASH_RGN_INX   5


//
// Access Modes for Region-Protection (with and without Translation)
//
#if !defined(XCHAL_CA_BYPASS) // 4bits (1 nibble)

// FETCH: Exception, LOAD: No Allocate, STORE: Write-through/No write allocate
#define XCHAL_CA_ALLOC                  0x0
// FETCH: Allocate, LOAD: Allocate, STORE: Write-through/No write allocate
#define XCHAL_CA_WRITETHRU              0x1

// FETCH: Bypass, LOAD: Bypass, STORE: Bypass
#define XCHAL_CA_BYPASS                 0x2
// FETCH: Bypass, LOAD: Bypass, STORE: Bypass
#define XCHAL_CA_BYPASSBUF              0x6

// FETCH: Allocate, LOAD: Allocate, STORE: Write-back/Write allocate
#define XCHAL_CA_WRITEBACK              0x4
// FETCH: Allocate, LOAD: Allocate, STORE: Write-back/No write allocate
#define XCHAL_CA_WRITEBACK_NOALLOC      0x5

// FETCH: Exception, LOAD: Exception, STORE: Exception
#define XCHAL_CA_ILLEGAL                0xF

#endif // XCHAL_CA_BYPASS


// set/change cache attribute of memory address range (start ~ end)
// NOTE: end = start + size, and the 'end' address is excluding from the range
// cattr can be set XCHAL_CA_XXX macros, i.e.
//  0x2 = XCHAL_CA_BYPASS (cache disabled),
//  0x4 = XCHAL_CA_WRITEBACK(WB, cache enabled),
//  0x1 = XCHAL_CA_WRITETHRU (WT, cache enabled)
// return value: 0 indicate failure, 1 indicate success
int set_cache_attr(unsigned long start, unsigned long size, unsigned long cattr)
{
    int ret;
    ret = xthal_set_region_attribute((void*)start, size, cattr, 0);
    return (ret == XTHAL_SUCCESS ? 1 : 0);
}

// check if memory address range (start ~ end) is cacheable
// NOTE: end = start + size, and the 'end' address is excluding from the range
int range_is_cacheable(unsigned long start, unsigned long size)
{
    //FIXME: just return non-cacheable if dcache is disabled in whole
//    if (!is_dcache_enabled())
//        return 0;

//    // located in PSRAM
//    if (start >= mem_rgn_list[PSRAM_RGN_INX].start_addr && size <= mem_rgn_list[PSRAM_RGN_INX].max_size)
//        return 1;
//
//    // located in FLASH
//    if (start >= mem_rgn_list[FLASH_RGN_INX].start_addr && size <= mem_rgn_list[FLASH_RGN_INX].max_size)
//        return 1;

    // return cacheable if at least 1 512MB region's cache is enabled
    uint32_t attr, val, i, entry1, entry2;
    attr = xthal_get_cacheattr(); // read CACHEATTR register
    entry1 = start >> 29; // divided by 512M
    entry2 = (start + size - 1) >> 29; // divided by 512M
    for (i = entry1; i <= entry2; i++) { // traverse at most 8 512MB regions
        val = (attr >> (i << 2)) & 0xF;
        if (val != XCHAL_CA_BYPASS && val != XCHAL_CA_BYPASSBUF) // at least 1 region enabled
            return 1;
    }

    // return non-cacheable by default (for HIFI4 local memory, peripherals' registers etc.)
    return 0;
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
