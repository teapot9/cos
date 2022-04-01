#include <panic.h>

#include <stdarg.h>
#include <print.h>
#include <power.h>

noreturn void panic(const char * fmt, ...)
{
	va_list ap;
	char error[1024];

	va_start(ap, fmt);
	vsnprintf(error, 1024, fmt, ap);
	va_end(ap);

	pr_emerg("Kernel panic: %s\n", error);
	halt();
}
