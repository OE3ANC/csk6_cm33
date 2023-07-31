/*
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/sys/math_extras.h>
#include "priv_heap.c"
#include "heap.h"

void csk_free(void *ptr)
{
	struct k_heap **heap_ref;

	if (ptr != NULL)
	{
		heap_ref = ptr;
		ptr = --heap_ref;

		k_heap_free(*heap_ref, ptr);
	}
}

void *csk_malloc(size_t size)
{

	void *ret = priv_aligned_alloc(sizeof(void *), size);

	return ret;
}

void *csk_calloc(size_t nmemb, size_t size)
{
	void *ret;
	size_t bounds;

	if (size_mul_overflow(nmemb, size, &bounds))
	{

		return NULL;
	}

	ret = csk_malloc(bounds);
	if (ret != NULL)
	{
		(void)memset(ret, 0, bounds);
	}

	return ret;
}

void *csk_realloc(void *ptr, uint32_t bytes)
{
	void *ret = priv_aligned_realloc(ptr, sizeof(void *), bytes);

	return ret;
}

static void get_alloc_info(struct z_heap *h, size_t *alloc_bytes,
			   size_t *free_bytes)
{
	chunkid_t c;

	*alloc_bytes = 0;
	*free_bytes = 0;

	for (c = right_chunk(h, 0); c < h->end_chunk; c = right_chunk(h, c)) {
		if (chunk_used(h, c)) {
			*alloc_bytes += chunksz_to_bytes(h, chunk_size(h, c));
		} else if (!solo_free_header(h, c)) {
			*free_bytes += chunksz_to_bytes(h, chunk_size(h, c));
		}
	}
}

size_t csk_heap_free_size(void)
{
	size_t alloc_bytes, free_bytes = 0;
    struct sys_heap *heap = &(_PRIVATE_SYS_HEAP)->heap;
    get_alloc_info(heap->heap, &alloc_bytes, &free_bytes);
    return free_bytes;
}

void csk_heap_info(void)
{

	sys_heap_print_info(&(_PRIVATE_SYS_HEAP)->heap, 0);
}
