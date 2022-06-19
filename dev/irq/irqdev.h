#ifndef _DEV_IRQ_IRQDEV_H
#define _DEV_IRQ_IRQDEV_H

#include <stdbool.h>

#define IRQ_CNT 16

struct device;

struct irqdev {
	int (* enable)(const struct device * dev, bool irqs[IRQ_CNT]);
	void (* disable)(const struct device * dev);
	void (* update)(const struct device * dev, bool irqs[IRQ_CNT]);
	void (* eoi)(const struct device * dev, unsigned irq);
};

int irqdev_reg(
	const struct device ** dev,
	const struct device * parent,
	int (* enable)(const struct device * dev, bool irqs[IRQ_CNT]),
	void (* disable)(const struct device * dev),
	void (* update)(const struct device * dev, bool irqs[IRQ_CNT]),
	void (* eoi)(const struct device * dev, unsigned irq)
);

#endif // _DEV_IRQ_IRQDEV_H
