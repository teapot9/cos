#ifndef _MM_ALIGN_H
#define _MM_ALIGN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline void * aligned_up(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = mod ? ia - mod + align : ia;
	return aligned;
}

static inline void * aligned_down(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = mod ? ia - mod : ia;
	return aligned;
}

static inline void * aligned(void * a, size_t align)
{
	return aligned_up(a, align);
}

static inline size_t align_diff_down(void * a, size_t align)
{
	return (uint8_t *) a - (uint8_t *) aligned_down(a, align);
}

static inline size_t align_diff_up(void * a, size_t align)
{
	return (uint8_t *) aligned_up(a, align) - (uint8_t *) a;
}

static inline size_t align_diff(void * a, size_t align)
{
	return align_diff_up(a, align);
}

static inline bool is_aligned(void * a, size_t align)
{
	return !((uintptr_t) a % align);
}

#ifdef __cplusplus
}
#endif
#endif // _MM_ALIGN_H
