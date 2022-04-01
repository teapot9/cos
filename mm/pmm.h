#ifndef MM_PMM_H
#define MM_PMM_H

#include <mm.h>
#include <stddef.h>
#include <stdbool.h>

extern bool pmm_is_init;

int early_pmap(void * paddr, size_t size);
void early_punmap(void * paddr, size_t size);

#endif // MM_PMM_H
