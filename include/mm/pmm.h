/**
 * @file mm/pmm.h
 * @brief Physical memory manager
 */

#ifndef __MM_PMM_H
#define __MM_PMM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <types.h>

/**
 * @brief Set physical memory as allocated to process
 * @param pid Owner
 * @param paddr Physical start address
 * @param size Size of memory block
 * @return errno
 */
int pmap(pid_t pid, void * paddr, size_t size);

/**
 * @brief Allocate physical memory
 * @param pid Owner
 * @param size Size of requested memory
 * @param align Memory alignment
 * @return Allocated physical memory address
 */
void * palloc(pid_t pid, size_t size, size_t align);

/**
 * @brief Free physical memory
 * @param pid Owner
 * @param paddr Physical start address
 * @param size Size of memory block
 */
void punmap(pid_t pid, void * paddr, size_t size);

/// Physical memory map
extern struct memmap memmap;

#ifdef __cplusplus
}
#endif
#endif
