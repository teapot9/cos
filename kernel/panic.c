#include <panic.h>

#include <stdarg.h>
#include <print.h>
#include <power.h>
#include <cpu.h>

#define BUFSIZ 512

noreturn void panic(const char * fmt, ...)
{
	va_list ap;
	char error[BUFSIZ];

	disable_interrupts();
	disable_nmi();

	va_start(ap, fmt);
	vsnprintf(error, BUFSIZ, fmt, ap);
	va_end(ap);

	pr_emerg("\n========================================\n", 0);
	pr_emerg("Kernel panic: %s\n", error);
	halt();
}
