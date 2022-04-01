#ifndef MM_H
#define MM_H

#include <stddef.h>
#include <stdint.h>

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
extern struct memmap memmap;
extern struct memlist used_blocks;
extern struct memlist free_blocks;

#endif // MM_H
