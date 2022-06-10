/**
 * @file mm/align.h
 * @brief Memory alignment helpers
 */

#ifndef _MM_ALIGN_H
#define _MM_ALIGN_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Align a pointer to upper address
 * @param a Pointer
 * @param align Alignment
 * @return Aligned pointer
 */
static inline void * aligned_up(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = (uint8_t *) a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = mod ? ia - mod + align : ia;
	return aligned;
}

/**
 * @brief Align a pointer to lower address
 * @param a Pointer
 * @param align Alignment
 * @return Aligned pointer
 */
static inline void * aligned_down(void * a, size_t align)
{
	if (align == 0)
		return a;
	uint8_t * ia = (uint8_t *) a;
	size_t mod = (size_t) ia % align;
	uint8_t * aligned = mod ? ia - mod : ia;
	return aligned;
}

/**
 * @brief Difference between pointer and a down-aligned pointer
 * @param a Pointer
 * @param align Alignment
 * @return abs(a - align_down(a))
 */
static inline size_t align_diff_down(void * a, size_t align)
{
	return (uint8_t *) a - (uint8_t *) aligned_down(a, align);
}

/**
 * @brief Difference between pointer and a up-aligned pointer
 * @param a Pointer
 * @param align Alignment
 * @return abs(a - align_up(a))
 */
static inline size_t align_diff_up(void * a, size_t align)
{
	return (uint8_t *) aligned_up(a, align) - (uint8_t *) a;
}

/**
 * @brief Test if pointer is aligned
 * @param a Pointer
 * @param align Alignment
 * @return Boolean
 */
static inline bool is_aligned(void * a, size_t align)
{
	return !((uintptr_t) a % align);
}

#ifdef __cplusplus
}
#endif
#endif // _MM_ALIGN_H
