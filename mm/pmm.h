#ifndef MM_PMM_H
#define MM_PMM_H

#include <mm.h>
#include <stddef.h>
#include <stdbool.h>

extern struct memblock * first_free_block;
extern struct memblock * first_used_block;

bool pmm_is_init(void);

int pmm_init(void);

int early_pmap(void * paddr, size_t size);

void * early_pmalloc(size_t size, size_t align);

void early_pfree(void * paddr, size_t size);

#endif // MM_PMM_H
