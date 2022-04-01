#include <debug.h>

#include <stdbool.h>

#include <print.h>

/* public: debug.h */
void kbreak(void)
{
#ifdef CONFIG_DEBUG
	bool cont = false;
	while (!cont);
#else
	pr_warn("Ignoring breakpoint\n", 0);
#endif
}
