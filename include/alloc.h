#ifndef MM_H
#define MM_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <types.h>

#define DEFAULT_MEMORY_ALLOC_ALIGN __BIGGEST_ALIGNMENT__

void * malloc(size_t size)
	__attribute__((malloc, alloc_size(1)));
#define malloc(s) __builtin_assume_aligned( \
	__builtin_malloc(s), DEFAULT_MEMORY_ALLOC_ALIGN \
)
void * kmalloc(size_t size, size_t align)
	__attribute__((malloc, alloc_size(1)));

void * realloc(void * oldptr, size_t newsize)
	__attribute__((alloc_size(2)));
#define realloc(p, s) __builtin_assume_aligned( \
	__builtin_realloc(p, s), DEFAULT_MEMORY_ALLOC_ALIGN \
)
void * krealloc(void * oldptr, size_t newsize, size_t align)
	__attribute__((alloc_size(2)));

void free(void * ptr);
#define free(p) __builtin_free(p)
void kfree(const void * ptr);

#ifdef __cplusplus
}
#endif
#endif // MM_H
