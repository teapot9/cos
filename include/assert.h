#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
# define assert(ignore)((void) 0)
#else
# include <panic.h>
# define assert(expr) do { \
	if (!(expr)) \
		panic("assertion failed: %s in %s:%s [%s]", \
		      #expr, __FILE__, __LINE__, __func__); \
}
#endif

#ifndef __cplusplus
# define static_assert _Static_assert
#endif

#endif // ASSERT_H
