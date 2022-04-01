#ifndef MM_HELPER_H
#define MM_HELPER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline bool is_next(void * a1, size_t s1, void * a2, size_t s2)
{
	uint8_t * ia1 = a1;
	uint8_t * ia2 = a2;
	return (ia1 < ia2 && ia1 + s1 == ia2)
		|| (ia1 > ia2 && ia2 + s2 == ia1);
}

static inline bool is_overlap(void * a1, size_t s1, void * a2, size_t s2)
{
	uint8_t * ia1 = a1;
	uint8_t * ia2 = a2;
	return (ia1 < ia2 && ia1 + s1 > ia2)
		|| (ia1 > ia2 && ia2 + s2 > ia1)
		|| ia1 == ia2;
}

/* Is memory block 2 inside block 1 */
static inline bool is_inside(void * a1, size_t s1, void * a2, size_t s2)
{
	return a1 <= a2
		&& (uint8_t *) a1 + s1
		>= (uint8_t *) a2 + s2;
}

static inline void * aligned_up(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = ia - mod + (mod ? align : 0);
	return aligned;
}

static inline void * aligned(void * a, size_t align)
{
	return aligned_up(a, align);
}

static inline void * aligned_down(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = ia - mod;
	return aligned;
}

static inline size_t align_diff_down(void * a, size_t align)
{
	return (uint8_t *) a - (uint8_t *) aligned_down(a, align);
}

#define align_diff align_diff_up
static inline size_t align_diff_up(void * a, size_t align)
{
	return (uint8_t *) aligned_up(a, align) - (uint8_t *) a;
}

static inline bool is_aligned(void * a, size_t align)
{
	return !((uintptr_t) a % align);
}

#endif // MM_HELPER_H
