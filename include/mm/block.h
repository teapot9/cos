#ifndef _MM_BLOCK_H
#define _MM_BLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#define __end(a, s) (a == 0 ? (void *) s : (uint8_t *) a + s)

static inline bool is_next(void * a1, size_t s1, void * a2, size_t s2)
{
	return (a1 < a2 && __end(a1, s1) == a2)
		|| (a1 > a2 && __end(s2, s2) == a1);
}

static inline bool is_overlap(void * a1, size_t s1, void * a2, size_t s2)
{
	return (a1 < a2 && __end(a1, s1) > a2)
		|| (a1 > a2 && __end(a2, s2) > a1)
		|| a1 == a2;
}

/* Is memory block 2 inside block 1 */
static inline bool is_inside(void * a1, size_t s1, void * a2, size_t s2)
{
	return a1 <= a2
		&& __end(a1, s1)
		>= __end(a2, s2);
}

#ifdef __cplusplus
}
#endif
#endif // _MM_BLOCK_H
