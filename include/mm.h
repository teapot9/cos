#ifndef MM_H
#define MM_H

#include <stddef.h>

struct memmap_desc {
	enum memory_type {
		MEMORY_TYPE_RESERVED,
		MEMORY_TYPE_KERNEL_CODE,
		MEMORY_TYPE_KERNEL_DATA,
		MEMORY_TYPE_EFI_SERVICES,
		MEMORY_TYPE_AVAILABLE,
		MEMORY_TYPE_BAD,
		MEMORY_TYPE_ACPI_RECLAIMABLE,
		MEMORY_TYPE_ACPI_NVS,
		MEMORY_TYPE_HARDWARE,
		MEMORY_TYPE_PERSISTENT,
	} type;
	void * phy_start;
	void * virt_start;
	size_t size;
};

struct memmap {
	struct memmap_desc * desc;
	size_t desc_count;
};

void mm_init_early(void);

void mm_init(struct memmap map);

void * kmalloc(size_t size);

void kfree(const void * ptr);

void * krealloc(void * oldptr, size_t newsize);

#endif // MM_H
