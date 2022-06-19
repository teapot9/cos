/**
 * @file alloc.h
 * @brief Kernel memory allocation from heap
 */

#ifndef __ALLOC_H
#define __ALLOC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <types.h>

#define DEFAULT_MEMORY_ALLOC_ALIGN __BIGGEST_ALIGNMENT__

/**
 * @brief Allocate kernel memory (simple)
 * @param size Requested memory (bytes)
 * @return Allocated memory pointer
 */
void * malloc(size_t size)
	__attribute__((malloc, alloc_size(1)));
#define malloc(s) __builtin_assume_aligned( \
	__builtin_malloc(s), DEFAULT_MEMORY_ALLOC_ALIGN \
)

/**
 * @brief Allocate kernel memory
 * @param size Requested memory (bytes)
 * @param align Memory alignment
 * @return Allocated memory pointer
 */
void * kmalloc(size_t size, size_t align)
	__attribute__((malloc, alloc_size(1)));

/**
 * @brief Re-allocate kernel memory (simple)
 * @param oldptr Old allocated memory address
 * @param newsize New memory size
 * @return Re-allocated memory pointer
 */
void * realloc(void * oldptr, size_t newsize)
	__attribute__((alloc_size(2)));
#define realloc(p, s) __builtin_assume_aligned( \
	__builtin_realloc(p, s), DEFAULT_MEMORY_ALLOC_ALIGN \
)

/**
 * @brief Re-allocate kernel memory
 * @param oldptr Old allocated memory address
 * @param newsize New memory size
 * @param align Memory alignment
 * @return Re-allocated memory pointer
 */
void * krealloc(void * oldptr, size_t newsize, size_t align)
	__attribute__((alloc_size(2)));

/**
 * @brief Free kernel memory (simple)
 * @param ptr Allocated memory pointer
 */
void free(void * ptr);
#define free(p) __builtin_free(p)

/**
 * @brief Free kernel memory
 * @param ptr Allocated memory pointer
 */
void kfree(const void * ptr);

#ifdef __cplusplus
}
#endif
#endif // __ALLOC_H
