#include <power.h>

#include <stdnoreturn.h>

#include <printk.h>

/* public: power.h */
WEAK noreturn void halt(void)
{
	// Default implementation
	pr_info("Kernel halting\n", 0);
	while (1);
}
