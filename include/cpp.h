/**
 * @file cpp.h
 * @brief C preprocessor helpers
 */

#ifndef __CPP_H
#define __CPP_H
#ifdef __cplusplus
extern "C" {
#endif

#define __stringify(x) #x
/// Stringify data
#define stringify(x) __stringify(x)


/// Get type of variable
#define typeof(x) __typeof__(x)
/// Check 2 variables have compatible types
#define same_type(x, y) __builtin_types_compatible_p(typeof(x), typeof(y))
/// Mark if() condition as likely to be true
#define likely(x) __builtin_expect((x), 1)
/// Mark if() condition as unlikely to be true
#define unlikely(x) __builtin_expect((x), 0)

/// Mark element as unused
#define _unused_ __attribute__((unused))
/// Mark element as used
#define _used_ __attribute__((used))
/// Mark struct as packed
#define _packed_ __attribute__((packed))
/// Mark function as cold (unlikely to be executed)
#define _cold_ __attribute__((cold))
/// Mark function as hot (run often)
#define _hot_ __attribute__((hot))
/// Mark symbol as weak (overriden by other identical symbols)
#define _weak_ __attribute__((weak))
/// Put the data in the provided section
#define _section_(x) __attribute__((section(x)))
/// Make function available in interrupt service routines
#define _isr_available_ __attribute__((no_caller_saved_registers))
/// Set data alignment
#define _aligned_(x) __attribute__((aligned(x)))

#ifdef __cplusplus
}
#endif
#endif // __CPP_H
