#define pr_fmt(fmt) "clock: " fmt

#include <clock.h>
#include "clock.h"

#include <errno.h>

#include <device.h>
#include <mm.h>
#include <module.h>
#include <cpp.h>

static size_t nb_clock = 0;
static const struct device * def_clock = NULL;
static struct module clock_mod = {
	.name = "clock",
};

static struct clock * get_clock(void)
{
	return (def_clock == NULL || !def_clock->available)
		? NULL : def_clock->driver_data;
}

static void new_def_clock(void)
{
	if (def_clock != NULL && def_clock->available)
		return;

	struct device_iter iter = device_iter_init("virtual", "clock");
	struct device * dev;
	while ((dev = device_iter_next(&iter)) != NULL && !dev->available);
	def_clock = dev;
}

static int reg(UNUSED const struct device * dev)
{
	new_def_clock();
	return 0;
}

static void unreg(const struct device * dev)
{
	if (dev->driver_data == def_clock)
		new_def_clock();
	kfree(dev->driver_data);
}

int clock_reg(
	const struct device ** dev,
	const struct device * parent,
	int (*new)(size_t msec, void (*callback)(void), size_t nbcall),
	int (*del)(void (*callback)(void))
)
{
	struct clock * clock = malloc(sizeof(*clock));
	if (clock == NULL)
		return -ENOMEM;
	clock->new = new;
	clock->del = del;

	int err = device_create(dev, &clock_mod, parent, "virtual", "clock",
				reg, unreg, clock, "clock%zu", nb_clock++);
	if (err)
		return err;

	return 0;
}

int clock_new(size_t msec, void (*callback)(void), size_t nbcall)
{
	struct clock * clock = get_clock();
	if (clock == NULL)
		return -ENOENT;
	return clock->new(msec, callback, nbcall);
}

int clock_del(void (*callback)(void))
{
	struct clock * clock = get_clock();
	if (clock == NULL)
		return -ENOENT;
	return clock->del(callback);
}
