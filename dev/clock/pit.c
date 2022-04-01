#define pr_fmt(fmt) "pit: " fmt

#include "pit.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

#include <irq.h>
#include <isr.h>
#include <device.h>
#include <cpu.h>
#include <asm/io.h>
#include <print.h>
#include <mm.h>
#include <module.h>
#include "clock.h"

static struct module pit_mod = {
	.name = "pit",
};

static size_t us_boot = 0;
static size_t jiffy_usec = 0;
static struct timer_list * timers = NULL;

// These will be set during pit_init
static size_t jiffies_trigger = SIZE_MAX;
static unsigned reload_value = -1;

static inline size_t ms_boot(void)
{
	return us_boot / 1000;
}

static void remove_timer(struct timer_list * t)
{
	if (t->prec != NULL)
		t->prec->next = t->next;
	kfree(t);
}

static void do_timer_calls(void)
{
	for (struct timer_list * t = timers; t != NULL; t = t->next) {
		if (ms_boot() - t->last < t->msec)
			continue;
		t->last = ms_boot();
		t->callback();
		if (t->nb_call && unlikely(!--t->nb_call))
			remove_timer(t);
	}
}

static void irq0_callback(void)
{
	static size_t jiffies = 0;
	jiffies++;
	if (unlikely(jiffies > jiffies_trigger)) {
		us_boot += jiffies * jiffy_usec;
		jiffies = 0;
		do_timer_calls();
	}
}

#define USEC_TRIGGER 1000 // 1ms
static void update_jiffy_usec(void)
{
	// how many usec per jiffy
	jiffy_usec = 1000000 / (PIT_FREQUENCY / reload_value);
	// how many jiffy before at least USEC_TRIGGER usec elapsed
	jiffies_trigger = USEC_TRIGGER / jiffy_usec + 1;
}

static void set_reload(unsigned val)
{
	struct pit_cmd cmd = {
		.bcd_mode = false,
		.op = OP_SQUARE_WAVE,
		.access = ACCESS_LOBYTE_HIBYTE,
		.channel = 0,
	};

	disable_interrupts();
	outb(PIT_IO_CMD, *(uint8_t *) &cmd);
	outb(PIT_IO_CHAN0, val & 0xFF);
	outb(PIT_IO_CHAN0, (val >> 8) & 0xFF);
	restore_interrupts();

	reload_value = val;
	update_jiffy_usec();
}

static unsigned get_reload(void)
{
	struct pit_cmd cmd = {
		.bcd_mode = false,
		.op = OP_SQUARE_WAVE,
		.access = ACCESS_LATCH_COUNT_VALUE_CMD,
		.channel = 0,
	};

	disable_interrupts();
	outb(PIT_IO_CMD, *(uint8_t *) &cmd);
	unsigned val = inb(PIT_IO_CHAN0);
	val |= inb(PIT_IO_CHAN0) << 8;
	restore_interrupts();

	if (val != reload_value)
		pr_crit("current reload value different from saved one: "
		        "%u != %u\n", val, reload_value);
	return val;
}

static int new(size_t msec, void (*callback)(void), size_t nbcall)
{
	if (msec == 0 || callback == NULL)
		return -EINVAL;

	struct timer_list * t = kmalloc(sizeof(*t));
	if (t == NULL)
		return -ENOMEM;

	t->last = 0;
	t->msec = msec;
	t->callback = callback;
	t->nb_call = nbcall;
	t->next = timers;
	t->prec = NULL;
	timers = t;
	return 0;
}

static int del(void (*callback)(void))
{
	for (struct timer_list * t = timers; t != NULL; t = t->next) {
		if (t->callback == callback)
			remove_timer(t);
	}
	return 0;
}

static int reg(const struct device * dev)
{
	int err;

	set_reload(PIT_DEFAULT_DIV);
	err = clock_reg(NULL, dev, new, del);
	if (err) {
		pr_err("failed to create clock device, errno = %d\n", err);
		return err;
	}

	err = isr_reg(irq_to_isr(0), irq0_callback);
	if (err) {
		pr_err("failed to register ISR handler, errno = %d\n", err);
		return err;
	}
	irq_unmask(0); // maybe in isr_reg (detect if irq)

	return 0;
}

static void unreg(UNUSED const struct device * dev)
{
}

static int pit_init(void)
{
	const struct device * dev;
	int err;

	err = device_create(&dev, &pit_mod, NULL, "clock", "pit",
			    reg, unreg, NULL, "pit");
	if (err) {
		pr_crit("failed to create pit device, errno = %d\n", err);
		return err;
	}

	return 0;
}
module_init(pit_init, early);
