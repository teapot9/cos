#define pr_fmt(fmt) "serial: " fmt

#include "serial.h"

#include <stddef.h>

#include <cpp.h>
#include <device.h>
#include <console.h>
#include <printk.h>

#define VT100_RESET "\033c"
#define VT100_CLEAR "\033[2J\033[1;1H"

static const char * lastpos = NULL;

static inline void reset(const struct device * dev)
{
	struct serial * serial = dev->driver_data;
	serial_write(serial->base, VT100_RESET);
}

static int enable(const struct device * dev)
{
	struct serial * serial = dev->driver_data;
	int err = serial_init_port(serial->base);
	if (err)
		return err;
	reset(dev);
	lastpos = NULL;
	return 0;
}

static void disable(const struct device * dev)
{
	struct serial * serial = dev->driver_data;
	serial_reset(serial->base);
}

static void update(const struct device * dev)
{
	struct serial * serial = dev->driver_data;

	const char * newpos = kmsg_next(lastpos);

	while (newpos != NULL) {
		serial_write(serial->base, kmsg_get_str(newpos));
		lastpos = newpos;
		newpos = kmsg_next(lastpos);
	}
}

static void clear(const struct device * dev)
{
	struct serial * serial = dev->driver_data;
	serial_write(serial->base, VT100_CLEAR);
}

int serial_console_reg(const struct device * dev)
{
	return console_reg(dev, update, clear, enable, disable);
}
