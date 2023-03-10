#ifndef _MM_PMM_H
#define _MM_PMM_H

#include <alloc.h>
#include <stddef.h>
#include <stdbool.h>

extern bool pmm_is_init;

int early_pmap(void * paddr, size_t size);
void early_punmap(void * paddr, size_t size);

#endif // _MM_PMM_H
