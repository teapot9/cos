#include <setup.h>

#include <stddef.h>

#include <print.h>
#include <module.h>
#include <power.h>

extern initcall_entry_t __initcall_early[];
extern initcall_entry_t __initcall_core[];
extern initcall_entry_t __initcall_device[];
extern initcall_entry_t __initcall_misc[];
extern initcall_entry_t __initcall_end[];

static inline initcall_entry_t * initcall_start(size_t lvl)
{
	switch (lvl) {
	case 0:
		return __initcall_early;
	case 1:
		return __initcall_core;
	case 2:
		return __initcall_device;
	case 3:
		return __initcall_misc;
	default:
		return __initcall_end;
	}
}

#define initcall_count 4
#define initcall_end(lvl) initcall_start(lvl + 1)

static inline initcall_t initcall_from_entry(initcall_entry_t * entry)
{
	return (initcall_t) ((unsigned long) entry + *entry);
}

static void kernel_initcall_level(size_t level)
{
	int err;

	if (level >= initcall_count) {
		pr_err("Initcall level %d > maximum (%d)\n",
		       level, initcall_count);
		return;
	}

	pr_info("Modules init: start level %d\n", level);
	for (initcall_entry_t * fn = initcall_start(level);
	     fn < initcall_end(level); fn++) {
		initcall_t call = initcall_from_entry(fn);
		pr_debug("Initcall: %p\n", call);
		err = call();
		if (err)
			pr_warn("Initcall %p failed, errno = %d\n", call, err);
	}
	pr_info("Modules init: end level %d\n", level);
}

static void setup(void)
{
	kernel_initcalls_early();
	kernel_initcalls();
}

/* public: init.h */
void kernel_initcalls_early(void)
{
	kernel_initcall_level(0);
}

/* public: init.h */
void kernel_initcalls(void)
{
	for (size_t i = 1; i < initcall_count; i++)
		kernel_initcall_level(i);
}

/* public: init.h */
noreturn void kernel_main(void)
{
	setup();
	halt();
}
