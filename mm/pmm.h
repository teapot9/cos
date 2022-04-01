#ifndef MM_PMM_H
#define MM_PMM_H

#include <stddef.h>

struct pmem_block {
	size_t size;
	struct pmem_block * next;
};

void pmm_init(struct memmap map);

void * pmalloc(size_t size, size_t align);

void * pzalloc(size_t size, size_t align);

void pfree(void * ptr, size_t size);

void print_memmap(struct memmap map);

#endif // MM_PMM_H
