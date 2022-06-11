#ifndef _MM_VMM_H
#define _MM_VMM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include <types.h>

/**
 * @brief Create virtual <-> physical memory mapping
 * @param pid Owner (memory space)
 * @param paddr Physical address
 * @param vaddr Virtual address
 * @param size Size of mapping
 * @return errno
 */
int vmap(pid_t pid, void * paddr, void * vaddr, size_t * size);

/**
 * @brief Reconfigure virtual memory
 * @param pid Owner (memory space)
 * @param vaddr Virtual address
 * @param size Size of memory to reconfigure
 * @param write Write permission
 * @param user User-space permission
 * @param exec Execute permission
 *
 * Dirty and access bits will be reset to false; permissions will be
 * reconfigured.
 */
int vreset(pid_t pid, void * vaddr, size_t size,
           bool write, bool user, bool exec);

/**
 * @brief Remove virtual <-> physical memory mapping
 * @param pid Owner (memory space)
 * @param vaddr Virtual address
 * @param size Size of memory to unmap
 * @return errno
 */
int vunmap(pid_t pid, void * vaddr, size_t size);

/**
 * @brief Allocate virtual memory and map it to a physical memory
 * @param pid Owner (memory space)
 * @param paddr Physical address
 * @param size Size of memory to map
 * @param valign Virtual memory requested alignment
 * @param write Write permission
 * @param user User-space permission
 * @param exec Execute permission
 * @return Allocated virtual memory address
 */
void * mmap(pid_t pid, void * paddr, size_t * size, size_t valign,
	    bool write, bool user, bool exec);

/**
 * @brief Allocate physical + virtual memory and create mapping
 * @param pid Owner (memory space)
 * @param size Size of memory to map
 * @param palign Physical memory requested alignment
 * @param valign Virtual memory requested alignment
 * @param write Write permission
 * @param user User-space permission
 * @param exec Execute permission
 * @param return Allocated virtual memory address
 */
void * valloc(pid_t pid, size_t * size, size_t palign, size_t valign,
	      bool write, bool user, bool exec);

/**
 * @brief Unmap and free virtual memory
 * @param pid Owner (memory space)
 * @param vaddr Virtual address
 * @param size Size of memory to free
 * @return errno
 */
int vfree(pid_t pid, void * vaddr, size_t size);

/// Kernel CR3 register value
extern uintn_t kernel_cr3;

#ifdef __cplusplus
}
#endif
#endif // _MM_VMM_H
