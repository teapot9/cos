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
void * kmalloc(size_t size);
void kfree(const void * ptr);
void * krealloc(void * oldptr, size_t newsize);

// vmm
struct page_perms {
	bool write;
	bool user;
	bool exec;
};

int vmap(pid_t pid, void * paddr, void * vaddr, size_t size,
         struct page_perms perms);
int vunmap(pid_t pid, void * vaddr, size_t size);
void * mmap(pid_t pid, void * paddr, size_t size, size_t valign,
	    struct page_perms perms);
void * valloc(pid_t pid, size_t size, size_t palign, size_t valign,
	      struct page_perms perms);
int vfree(pid_t pid, void * vaddr, size_t size);
int vinit(pid_t pid, void * vaddr, size_t size, struct page_perms perms);

// pmm
int pmap(pid_t pid, void * paddr, size_t size);
void * palloc(pid_t pid, size_t size, size_t align);
void punmap(pid_t pid, void * paddr, size_t size);
extern struct memmap memmap;

#endif // MM_H
