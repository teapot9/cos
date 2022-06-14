#include <power.h>

#include <printk.h>
#include <asm/asm.h>

/* public: power.h */
noreturn void halt(void)
{
	pr_emerg("Kernel halting\n", 0);
	while (1) {
		asm volatile(intel(
			"cli\n"
			"hlt\n"
		));
	}
}
