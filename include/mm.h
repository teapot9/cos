#ifndef MM_H
#define MM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <task.h>

#define DEFAULT_MEMORY_ALLOC_ALIGN __BIGGEST_ALIGNMENT__

struct memblock {
	void * addr;
	size_t size;
};

// kmm
void * malloc(size_t size)
	__attribute__((malloc, alloc_size(1)));
#define malloc(s) __builtin_assume_aligned( \
	__builtin_malloc(s), DEFAULT_MEMORY_ALLOC_ALIGN \
)
void * kmalloc(size_t size, size_t align)
	__attribute__((malloc, alloc_size(1)));
#define kmalloc(s, a) __builtin_assume_aligned(kmalloc(s, a), a)

void * realloc(void * oldptr, size_t newsize)
	__attribute__((alloc_size(2)));
#define realloc(p, s) __builtin_assume_aligned( \
	__builtin_realloc(p, s), DEFAULT_MEMORY_ALLOC_ALIGN \
)
void * krealloc(void * oldptr, size_t newsize, size_t align)
	__attribute__((alloc_size(2)));
#define krealloc(p, s, a) __builtin_assume_aligned(krealloc(p, s, a), a)

void free(void * ptr);
#define free(p) __builtin_free(p)
void kfree(const void * ptr);
#define kfree(p) kfree(p)

// vmm
int vmap(pid_t pid, void * paddr, void * vaddr, size_t * size);
int vreset(pid_t pid, void * vaddr, size_t size,
           bool write, bool user, bool exec);
int vunmap(pid_t pid, void * vaddr, size_t size);

void * mmap(pid_t pid, void * paddr, size_t * size, size_t valign,
	    bool write, bool user, bool exec);
void * valloc(pid_t pid, size_t * size, size_t palign, size_t valign,
	      bool write, bool user, bool exec);
int vfree(pid_t pid, void * vaddr, size_t size);

// pmm
int pmap(pid_t pid, void * paddr, size_t size);
void * palloc(pid_t pid, size_t size, size_t align);
void punmap(pid_t pid, void * paddr, size_t size);
extern struct memmap memmap;

#endif // MM_H
