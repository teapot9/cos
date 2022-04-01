#include <panic.h>

#include <stdarg.h>
#include <print.h>
#include <power.h>

#define BUFSIZ 512

noreturn void panic(const char * fmt, ...)
{
	va_list ap;
	char error[BUFSIZ];

	va_start(ap, fmt);
	vsnprintf(error, BUFSIZ, fmt, ap);
	va_end(ap);

	pr_emerg("Kernel panic: %s\n", error);
	halt();
}
