/**
 * @file panic.h
 * @brief Kernel panic
 */

#ifndef PANIC_H
#define PANIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdnoreturn.h>

#include <cpp.h>

/**
 * @brief Cause a kernel panic with provided description
 * @param fmt Format string (printf-like)
 * @param ... Arguments for fmt
 *
 * This function never returns.
 */
noreturn void panic(const char * fmt, ...) _isr_available_;

#ifdef __cplusplus
}
#endif
#endif // PANIC_H
