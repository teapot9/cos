#define pr_fmt(fmt) "pic: " fmt
#include "pic.h"

#include "irqdev.h"
#include <cpu.h>
#include <asm/io.h>
#include <printk.h>
#include <module.h>
#include <device.h>

static bool is_init = false;
static const struct module pic_mod = {
	.name = "pic",
};

static void remap(unsigned pic1, unsigned pic2)
{
	struct icw1 icw1 = {.icw4 = true, .init = true, 0};
	struct icw4 icw4 = {.m8086 = true, 0};

	outb(PIC1_CMD, *(uint8_t *) &icw1);
	io_wait();
	outb(PIC1_DATA, pic1);
	io_wait();
	outb(PIC1_DATA, 1 << 2);
	io_wait();
	outb(PIC1_DATA, *(uint8_t *) &icw4);
	io_wait();

	outb(PIC2_CMD, *(uint8_t *) &icw1);
	io_wait();
	outb(PIC2_DATA, pic2);
	io_wait();
	outb(PIC2_DATA, 1 << 1);
	io_wait();
	outb(PIC2_DATA, *(uint8_t *) &icw4);
	io_wait();
}

static void unmask(unsigned i)
{
	unsigned pic = i >= 8 ? PIC2_DATA : PIC1_DATA;
	unsigned offset = i >= 8 ? i - 8 : i;
	outb(pic, inb(pic) & ~(1 << offset));
}

static void mask(unsigned i)
{
	unsigned pic = i >= 8 ? PIC2_DATA : PIC1_DATA;
	unsigned offset = i >= 8 ? i - 8 : i;
	outb(pic, inb(pic) | 1 << offset);
}

static void eoi(_unused_ const struct device * dev, unsigned irq)
{
	struct icw1 cmd = {.eoi = true, 0};
	if (irq >= 8)
		outb(PIC2_CMD, *(uint8_t *) &cmd);
	outb(PIC1_CMD, *(uint8_t *) &cmd);
}

static void update(_unused_ const struct device * dev, bool irqs[IRQ_CNT])
{
	for (unsigned i = 0; i < IRQ_CNT; i++) {
		if (irqs[i])
			unmask(i);
		else
			mask(i);
	}
}

static int enable(const struct device * dev, bool irqs[IRQ_CNT])
{
	update(dev, irqs);
	return 0;
}

static void disable(_unused_ const struct device * dev)
{
	outb(PIC2_DATA, 0xFF);
	outb(PIC1_DATA, 0xFF);
}

static int reg(const struct device * dev)
{
	disable_interrupts();
	remap(0x20, 0x28);
	disable(dev);
	restore_interrupts();

	return irqdev_reg(NULL, dev, enable, disable, update, eoi);
}

static void unreg(_unused_ const struct device * dev)
{
}

static int pic_init(void)
{
	if (is_init)
		return 0;

	const struct device * dev;
	int err = device_create(
		&dev, &pic_mod, NULL, "platform", "pic", reg, unreg, NULL, "pic"
	);
	if (err)
		return err;

	is_init = true;
	pr_info("initialized\n", 0);
	return 0;
}
module_init(pic_init, early);
