#define pr_fmt(fmt) "text: " fmt

#include <console.h>
#include "console.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include <lock.h>
#include <device.h>
#include <module.h>
#include <alloc.h>
#include <string.h>
#include <printk.h>

static size_t console_count = 0;
static size_t console_max = 0;
static const struct device ** consoles = NULL;
static size_t total_con_count = 0;
#define CONSOLE_DEFAULT (10)

static const struct module console_mod = {
	.name = "console",
};

static bool is_graphic(const struct device * dev)
{
	return !strcmp(dev->parent->class, "graphics");
}

static bool has_graphic(void)
{
	for (size_t i = 0; i < console_count; i++) {
		if (is_graphic(consoles[i]))
			return true;
	}
	return false;
}

static void con_clear(const struct device * dev)
{
	const struct device * backend = dev->parent;
	const struct console * console = dev->driver_data;
	console->clear(backend);
}

static void con_update(const struct device * dev)
{
	const struct device * backend = dev->parent;
	const struct console * console = dev->driver_data;
	console->update(backend);
}

static int con_enable(const struct device * dev)
{
	int err;
	const struct device * backend = dev->parent;
	const struct console * console = dev->driver_data;

	if (consoles == NULL || console_count >= console_max) {
		size_t newsize =
			console_max ? console_max * 2 : CONSOLE_DEFAULT;
		const struct device ** newdev =
			realloc(consoles, sizeof(*newdev) * newsize);
		if (newdev == NULL)
			return -ENOMEM;
		consoles = newdev;
		console_max = newsize;
	}

	consoles[console_count++] = dev;
	err = console->enable(backend);
	if (err)
		return err;

	con_clear(dev);
	con_update(dev);
	return 0;
}

static void con_disable(const struct device * dev, size_t index)
{
	int err;
	const struct device * backend = dev->parent;
	const struct console * console = dev->driver_data;

	while (index < --console_count)
		consoles[index] = consoles[index + 1];

	console->disable(backend);

	if (is_graphic(dev) && !has_graphic()) {
		struct device * cur;
		struct device_iter iter = device_iter_init("virtual", "con");
		while ((cur = device_iter_next(&iter)) != NULL) {
			if (cur != dev && is_graphic(cur)) {
				if ((err = con_enable(cur)))
					pr_warn("Could not enable graphic "
					        "console %p, errno %d\n",
						cur, err);
				break;
			}
		}
	}
}

static int con_reg(const struct device * dev)
{
	if (is_graphic(dev) && has_graphic())
		return 0;
	return con_enable(dev);
}

static void con_unreg(const struct device * dev)
{
	const struct console * console = dev->driver_data;

	size_t i = 0;
	while (i < console_count && consoles[i] != dev)
		i++;
	if (i < console_count)
		con_disable(dev, i);

	kfree(console);
}

/* public: console.h */
int console_reg(
	const struct device * parent,
	void (*update)(const struct device *),
	void (*clear)(const struct device *),
	int (*enable)(const struct device *),
	void (*disable)(const struct device *)
)
{
	int err;

	struct console * console = malloc(sizeof(*console));
	if (console == NULL)
		return -ENOMEM;
	console->update = update;
	console->clear = clear;
	console->enable = enable;
	console->disable = disable;

	err = device_create(
		NULL, &console_mod, parent, "virtual", "con",
		con_reg, con_unreg, console, "con%zu", total_con_count++
	);
	if (err)
		return err;

	return 0;
}

/* public: console.h */
void console_clear(void)
{
	if (consoles == NULL)
		return;

	for (size_t i = 0; i < console_count; i++)
		con_clear(consoles[i]);
}

/* public: console.h */
void console_reset(void)
{
	console_clear();
	console_update();
}

/* public: console.h */
void console_update(void)
{
	static struct spinlock nblock = nblock_init();
	if (!nblock_lock(&nblock))
		return;

	if (consoles == NULL)
		goto exit;

	for (size_t i = 0; i < console_count; i++)
		con_update(consoles[i]);
exit:
	nblock_unlock(&nblock);
}
