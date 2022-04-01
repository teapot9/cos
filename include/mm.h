#ifndef MM_H
#define MM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <task.h>

struct memblock {
	void * addr;
	size_t size;
};

// kmm
void * malloc(size_t size);
void free(void * ptr);
void * realloc(void * oldptr, size_t newsize);
void * kmalloc(size_t size, size_t align);
void kfree(const void * ptr);
void * krealloc(void * oldptr, size_t newsize, size_t align);

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
