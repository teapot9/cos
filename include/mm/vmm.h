#ifndef _MM_VMM_H
#define _MM_VMM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include <types.h>

int vmap(pid_t pid, void * paddr, void * vaddr, size_t * size);
int vreset(pid_t pid, void * vaddr, size_t size,
           bool write, bool user, bool exec);
int vunmap(pid_t pid, void * vaddr, size_t size);

void * mmap(pid_t pid, void * paddr, size_t * size, size_t valign,
	    bool write, bool user, bool exec);
void * valloc(pid_t pid, size_t * size, size_t palign, size_t valign,
	      bool write, bool user, bool exec);
int vfree(pid_t pid, void * vaddr, size_t size);

extern uintn_t kernel_cr3;

#ifdef __cplusplus
}
#endif
#endif // _MM_VMM_H
