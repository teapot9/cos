/**
 * @file mm/block.h
 * @brief Memory block helpers
 */

#ifndef __MM_BLOCK_H
#define __MM_BLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define __end(a, s) (a == 0 ? (void *) s : (uint8_t *) a + s)

/**
 * @brief Test if two blocks of memory are adjacent
 * @param a1 Start of block 1
 * @param s1 Size of block 1
 * @param a2 Start of block 2
 * @param s2 Size of block 2
 * @return Boolean
 */
static inline bool is_next(void * a1, size_t s1, void * a2, size_t s2)
{
	return (a1 < a2 && __end(a1, s1) == a2)
		|| (a1 > a2 && __end(s2, s2) == a1);
}

/**
 * @brief Test if two blocks of memory are overlaping
 * @param a1 Start of block 1
 * @param s1 Size of block 1
 * @param a2 Start of block 2
 * @param s2 Size of block 2
 * @return Boolean
 */
static inline bool is_overlap(void * a1, size_t s1, void * a2, size_t s2)
{
	return (a1 < a2 && __end(a1, s1) > a2)
		|| (a1 > a2 && __end(a2, s2) > a1)
		|| a1 == a2;
}

/**
 * @brief Test if memory block 2 is inside block 1
 * @param a1 Start of block 1
 * @param s1 Size of block 1
 * @param a2 Start of block 2
 * @param s2 Size of block 2
 * @return Boolean
 */
static inline bool is_inside(void * a1, size_t s1, void * a2, size_t s2)
{
	return a1 <= a2
		&& __end(a1, s1)
		>= __end(a2, s2);
}

#ifdef __cplusplus
}
#endif
#endif // __MM_BLOCK_H
