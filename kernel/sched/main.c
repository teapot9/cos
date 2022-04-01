#define pr_fmt(fmt) "sched: " fmt

#include <sched.h>
#include "sched.h"

#include <stdbool.h>

#include <asm/asm.h>
#include <cpp.h>
#include <print.h>
#include <clock.h>
#include <cpu.h>
#include <task.h>
#include <panic.h>
#include <module.h>

static bool is_enabled = false;

void do_switch(enum tstate reason)
{
	struct cpu * cpu = cpu_current();
	struct thread * cur = cpu_running(cpu);
	struct thread * next;

	task_set_state(cur, reason);
	int err = sched_next(&next, cur);
	if (err)
		panic("no task to switch to, errno = %d", err);

	task_switch(cpu, next);
}

static void callback_yield(void)
{
	do_switch(TASK_WAITING);
}

static void callback_timer(void)
{
	do_switch(TASK_READY);
}

static int sched_init(void)
{
	int err;
	pr_debug("init\n", 0);

	err = clock_new(300, callback_timer, 0);
	if (err)
		return err;
	err = isr_reg(ISR_YIELD, callback_yield);
	if (err)
		return err;
	return 0;
}
module_init(sched_init, early);

bool sched_enabled(void)
{
	return is_enabled;
}

/* public: sched.h */
int sched_enable(void)
{
	is_enabled = true;
	return 0;
}

/* public: sched.h */
void sched_disable(void)
{
	is_enabled = false;
}

/* public: sched.h */
void sched_yield(void)
{
	if (!sched_enabled())
		return;
	asm volatile (intel("int " stringify(ISR_YIELD) "\n"));
}
