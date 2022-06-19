/**
 * @file assert.h
 * @brief Assertion
 */

#ifndef __ASSERT_H
#define __ASSERT_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Assertion
 * @param expr Expression to test
 *
 * If expr == false, cause kernel panic. If expr == true or NDEBUG defined,
 * no-op.
 */
#ifdef NDEBUG
# define assert(expr)((void) 0)
#else
# include <panic.h>
# define assert(expr) do { \
	if (!(expr)) \
		panic("assertion failed: %s in %s:%s [%s]", \
		      #expr, __FILE__, __LINE__, __func__); \
} while (0)
#endif

#ifndef __cplusplus
# define static_assert _Static_assert
#endif

#ifdef __cplusplus
}
#endif
#endif // __ASSERT_H
