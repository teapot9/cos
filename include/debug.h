/**
 * @file debug.h
 * @brief Debugging
 */

#ifndef __DEBUG_H
#define __DEBUG_H
#ifdef __cplusplus
extern "C" {
#endif

#include <printk.h>
#include <kconfig.h>

/**
 * @brief Breakpoint
 *
 * CONFIG_DEBUG enabled: suspend execution until debugger resume.
 * CONFIG_DEBUG disabled: print warning and continue execution.
 */
#if IS_ENABLED(CONFIG_DEBUG)
# define kbreak() do { \
	pr_debug("reached breakpoint: %s:%d [%s]\n", \
	         __FILE__, __LINE__, __func__); \
	_kbreak(); \
	} while(0)
#else // !CONFIG_DEBUG
# define kbreak() do { \
	pr_warn("skipping breakpoint: %s:%d [%s]\n", \
	         __FILE__, __LINE__, __func__); \
	} while(0)
#endif // CONFIG_DEBUG

/**
 * @brief Breakpoint real function
 *
 * This function is only defined if CONFIG_DEBUG is enabled.
 * This is a non-inlined function that implements a basic breakpoint
 * (infinite loop waiting for debugger to set a boolean to true).
 */
#if IS_ENABLED(CONFIG_DEBUG)
void _kbreak(void);
#endif // CONFIG_DEBUG

#ifdef __cplusplus
}
#endif
#endif // __DEBUG_H
