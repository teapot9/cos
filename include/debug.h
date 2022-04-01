#ifndef DEBUG_H
#define DEBUG_H

#include <print.h>

#ifdef CONFIG_DEBUG
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

#endif // DEBUG_H
