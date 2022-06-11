/**
 * @file mm/paging.h
 * @brief VMM paging functions
 */

#ifndef MM_PAGING_H
#define MM_PAGING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <task.h>

/**
 * @brief Convert virtual address to physical
 * @param pid PID of the VM space
 * @param vaddr Virtual address
 * @return Physical address
 */
void * virt_to_phys(pid_t pid, void * vaddr);

/**
 * @brief Convert virtual address to physical, with current process memory space
 * @param vaddr Virtual address
 * @return Physical address
 */
void * virt_to_phys_current(void * vaddr);

/**
 * @brief Map virtual to physical memory
 * @param pid PID of the VM space
 * @param vaddr Virtual address
 * @param size Size of mapping
 * @param paddr Physical address
 * @return errno
 */
int page_map(pid_t pid, void * vaddr, size_t * size, void * paddr);

/**
 * @brief Set memory mapping information
 * @param pid PID of the VM space
 * @param vaddr Virtual address
 * @param size Size of mapping
 * @param write Write permission
 * @param user User ring permission
 * @param exec Execute permission
 * @param accessed Accessed bit
 * @param dirty Dirty bit
 * @return errno
 */
int page_set(pid_t pid, void * vaddr, size_t size, bool write, bool user,
             bool exec, bool accessed, bool dirty);

/**
 * @brief Unmap memory
 * @param pid PID of the VM space
 * @param vaddr Virtual address
 * @param size Size of mapping
 * @return errno
 */
int page_unmap(pid_t pid, void * vaddr, size_t size);

/**
 * @brief Find unmapped memory
 * @param pid PID of the VM space
 * @param size Size of searched memory
 * @param align Alignment of searched memory
 * @param start Start searching from this address
 * @return Found free memory address (NULL if not found)
 */
void * page_find_free(pid_t pid, size_t size, size_t align, void * start);

#ifdef __cplusplus
}
#endif
#endif // MM_PAGING_H
