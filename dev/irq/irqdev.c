#define pr_fmt(fmt) "irqdev: " fmt

#include <irq.h>
#include "irqdev.h"

#include <stddef.h>
#include <errno.h>

#include <print.h>
#include <mm.h>
#include <device.h>
#include <module.h>
#include <cpp.h>

static const struct module irqdev_mod = {
	.name = "irqdev",
};
static size_t total_irqdev_count = 0;
static const struct device * def_dev = NULL;
static bool irqs_state[IRQ_CNT] = {
	false, false, true, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
};
static bool irq_state = false;

static inline struct irqdev * get_dev(void)
{
	return def_dev == NULL ? NULL : def_dev->driver_data;
}

static void update_mask(void)
{
	struct irqdev * dev = get_dev();
	if (dev == NULL)
		return;
	dev->update(def_dev->parent, irqs_state);
}

static int set_default(const struct device * dev)
{
	int err;
	def_dev = dev;

	if (irq_state) {
		if ((err = irq_enable()))
			return err;
	}
	update_mask();
	return 0;
}

static void update_default(void)
{
	if (def_dev != NULL && def_dev->available)
		return;

	struct device_iter idev = device_iter_init("virtual", "irqdev");
	struct device * cur;
	while ((cur = device_iter_next(&idev)) != NULL) {
		if (!cur->available)
			continue;
		set_default(cur);
		return;
	}
	pr_err("no replacement irqdev has been found", 0);
}

static int reg(const struct device * dev)
{
	return def_dev == NULL ? set_default(dev) : 0;
}

static void unreg(const struct device * dev)
{
	kfree(dev->driver_data);
	if (dev == def_dev)
		update_default();
}

int irqdev_reg(
	const struct device ** dev,
	const struct device * parent,
	int (* enable)(const struct device * dev, bool irqs[IRQ_CNT]),
	void (* disable)(const struct device * dev),
	void (* update)(const struct device * dev, bool irqs[IRQ_CNT]),
	void (* eoi)(const struct device * dev, unsigned irq)
)
{
	int err;

	if (parent == NULL)
		return -EINVAL;

	struct irqdev * idev = malloc(sizeof(*idev));
	if (idev == NULL)
		return -ENOMEM;
	idev->enable = enable;
	idev->disable = disable;
	idev->update = update;
	idev->eoi = eoi;

	err = device_create(
		dev, &irqdev_mod, parent, "virtual", "irqdev",
		reg, unreg, idev, "irqdev%zu", total_irqdev_count++
	);
	if (err)
		return err;

	return 0;
}

/* public: irq.h */
int irq_enable(void)
{
	irq_state = true;

	struct irqdev * dev = get_dev();
	if (dev == NULL)
		return -ENOENT;
	return dev->enable(def_dev->parent, irqs_state);
}

/* public: irq.h */
void irq_disable(void)
{
	irq_state = false;

	struct irqdev * dev = get_dev();
	if (dev == NULL)
		return;
	dev->disable(def_dev->parent);
}

/* public: irq.h */
void irq_mask(unsigned irq)
{
	if (irq >= IRQ_CNT)
		return;
	if (irq == 2)
		pr_warn("disabling irq2 cascade\n", 0);
	irqs_state[irq] = false;
	update_mask();
}

/* public: irq.h */
void irq_unmask(unsigned irq)
{
	if (irq >= IRQ_CNT)
		return;
	irqs_state[irq] = true;
	update_mask();
}

/* public: irq.h */
void irq_eoi(unsigned irq)
{
	if (irq >= IRQ_CNT)
		return;
	struct irqdev * dev = get_dev();
	if (dev == NULL)
		return;
	dev->eoi(def_dev->parent, irq);
}
