#ifndef MM_H
#define MM_H

#include <stddef.h>
#include <stdint.h>

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

struct memblock {
	void * addr;
	size_t size;
};

// kmm
void * kmalloc(size_t size);
void kfree(const void * ptr);
void * krealloc(void * oldptr, size_t newsize);

// vmm
void * virt_to_phys(void * vaddr);
int vmap(void * paddr, void * vaddr, size_t size);
struct memblock vmalloc(size_t size);
void * mmap(void * paddr, size_t size);
void vunmap(void * vaddr, size_t size);
void vfree(void * vaddr, size_t size);
uint64_t kcr3(void);

// pmm
int pmap(void * paddr, size_t size);
void * pmalloc(size_t size, size_t align);
void pfree(void * paddr, size_t size);

#endif // MM_H
