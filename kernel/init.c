#include <setup.h>

#include <stddef.h>

#include <lock.h>
#include <debug.h>
#include <printk.h>
#include <module.h>
#include <power.h>
#include <panic.h>
#include <cpu.h>
#include <sched.h>
#include <task.h>
#include <alloc.h>
#include <test/compiler.h>
#include <kconfig.h>

#define INITCALL_EARLY 0
#define INITCALL_CORE 1
#define INITCALL_DEVICE 2
#define INITCALL_MISC 3

extern initcall_entry_t __initcall_early[];
extern initcall_entry_t __initcall_core[];
extern initcall_entry_t __initcall_device[];
extern initcall_entry_t __initcall_misc[];
extern initcall_entry_t __initcall_end[];

extern constructor_t __constructors_start[];
extern constructor_t __constructors_end[];

static inline initcall_entry_t * initcall_start(size_t lvl)
{
	switch (lvl) {
	case INITCALL_EARLY:
		return __initcall_early;
	case INITCALL_CORE:
		return __initcall_core;
	case INITCALL_DEVICE:
		return __initcall_device;
	case INITCALL_MISC:
		return __initcall_misc;
	default:
		return __initcall_end;
	}
}

#define initcall_count 4
#define initcall_end(lvl) initcall_start(lvl + 1)

__attribute__((no_sanitize("all")))
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

#define maybe_kbreak_init(conf, lvl) do { \
	if (IS_ENABLED(conf) && level == lvl) \
		kbreak(); \
	} while (0)
	maybe_kbreak_init(CONFIG_BOOT_BREAKPOINT_CORE, INITCALL_CORE);
	maybe_kbreak_init(CONFIG_BOOT_BREAKPOINT_DEVICE, INITCALL_DEVICE);
	maybe_kbreak_init(CONFIG_BOOT_BREAKPOINT_MISC, INITCALL_MISC);

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

void constructors(void)
{
	for (constructor_t * fn = __constructors_start;
	     fn < __constructors_end; fn++) {
		constructor_t call = *fn;
		pr_debug("constructor: %p\n", call);
		call();
	}
}

#if IS_ENABLED(CONFIG_DEBUG)
static void compiler_tests(void)
{
	int ret;
	ret = compiler_test_cxx(COMPILER_TEST_VALUE);
	if (ret != COMPILER_TEST_VALUE)
		panic("C++ compiler test failed on line %d", ret);
}
#endif

static void setup(void)
{
#if IS_ENABLED(CONFIG_BOOT_BREAKPOINT_EARLY)
	kbreak();
#endif

	enable_nmi();
	restore_interrupts();
	constructors();
	kernel_initcalls_early();
#if IS_ENABLED(CONFIG_DEBUG)
	compiler_tests();
#endif

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

/* public: init.h */
void kernel_main(void)
{
	pr_info("Kernel main thread started\n", 0);
	setup();
}
