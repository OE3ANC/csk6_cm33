/*
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <kernel.h>
#include <string.h>
#include <sys/math_extras.h>
#include "priv_heap.c"

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

void csk_heap_info(void)
{

	sys_heap_print_info(&(_PRIVATE_SYS_HEAP)->heap, 0);
}
