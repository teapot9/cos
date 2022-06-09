#ifndef _MM_PMM_H
#define _MM_PMM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <types.h>

int pmap(pid_t pid, void * paddr, size_t size);
void * palloc(pid_t pid, size_t size, size_t align);
void punmap(pid_t pid, void * paddr, size_t size);
extern struct memmap memmap;

#ifdef __cplusplus
}
#endif
#endif
