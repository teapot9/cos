#ifndef MM_EARLY_H
#define MM_EARLY_H

#include <stdbool.h>
#include <stddef.h>

#include <mm.h>

int register_free_pmem(void * paddr, size_t size);
int register_used_pmem(void * paddr, size_t size);

int pmm_acpi_register(void * paddr, size_t size);
bool pmm_acpi_iter(struct memblock * prev);
void pmm_acpi_free(void);

#endif // MM_EARLY_H
