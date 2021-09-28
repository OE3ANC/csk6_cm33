/*
 * cache.h
 *
 *  Created on: Jul 10, 2020
 *
 */

#ifndef INCLUDE_CACHE_H_
#define INCLUDE_CACHE_H_

// check if memory address range (start ~ end) is cacheable
// NOTE: end = start + size, and the 'end' address is excluding from the range
int range_is_cacheable(unsigned long start, unsigned long size);

// write back from cache to memory
void dcache_clean_range(unsigned long start, unsigned long end);

// invalidate data in cache (NOT write back to memory)
void dcache_invalidate_range(unsigned long start, unsigned long end);

// write back from cache to memory and then invalidate data in cache
void dcache_flush_range(unsigned long start, unsigned long end);

// usually called before transferring data to memory (NOT cache-line-aligned) via DMA
void cache_dma_fast_inv_stage1(unsigned long start, unsigned long end);

// usually called after transferring data to memory  (NOT cache-line-aligned) via DMA
void cache_dma_fast_inv_stage2(unsigned long start, unsigned long end);

#endif /* INCLUDE_CACHE_H_ */
