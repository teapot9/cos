#include <setup.h>

#include <stddef.h>

#include <lock.h>
#include <debug.h>
#include <print.h>
#include <module.h>
#include <power.h>
#include <panic.h>
#include <cpu.h>
#include <sched.h>
#include <task.h>
#include <mm.h>

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

__attribute__((no_sanitize("undefined"))) // don't know how to fix type mismatch
static inline initcall_t initcall_from_entry(initcall_entry_t * entry)
{
	return (initcall_t) ((unsigned long) entry + *entry);
}

static inline int do_call(initcall_t call)
{
	return call();
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
		err = do_call(call);
		if (err)
			pr_warn("Initcall %p failed, errno = %d\n", call, err);
	}
	pr_info("Modules init: end level %d\n", level);
}

static void setup(void)
{
	enable_nmi();
	restore_interrupts();
	kernel_initcalls_early();
	// while (1)
		// pr_debug("printing string to debug fb\n", 0);
	sched_enable();
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

extern uint8_t _binary__tmp_albert_bin_start[];
extern uint8_t _binary__tmp_albert_bin_end[];

int pipo(int x[static 1]) {
	if (*x > 5)
		return 33;
}

/* public: init.h */
noreturn void kernel_main(void)
{
	pr_info("Kernel main thread started\n", 0);
	setup();
	int * y = malloc(2321);
	int x = pipo(y);
	halt();
	panic("Kernel should never exit main function\n");
}
