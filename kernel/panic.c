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

#include <stdint.h>
#if UINT32_MAX == UINTPTR_MAX
# define STACK_CHK_GUARD 0xe2dee396
#else
# define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

noreturn void __stack_chk_fail(void)
{
	panic("Stack smashing detected");
}
