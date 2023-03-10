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
# ifndef BOOTLOADER
#  include <asm/asm.h>
#  define kbreak() do { \
	pr_debug("reached breakpoint: %s:%d [%s]\n", \
	         __FILE__, __LINE__, __func__); \
	asm volatile (intel("int3\n")); \
	} while (0)
# else // !BOOTLOADER
static inline void kbreak() {
	pr_debug("reached breakpoint\n", 0);
	bool stop = false;
	while (!stop);
}
# endif // BOOTLOADER
#else // CONFIG_DEBUG
# define kbreak() do { \
	pr_warn("skipping breakpoint: %s:%d [%s]\n", \
	         __FILE__, __LINE__, __func__); \
	while (0)
#endif // !CONFIG_DEBUG

#ifdef __cplusplus
}
#endif
#endif // __DEBUG_H
