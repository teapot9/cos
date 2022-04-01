#define pr_fmt(fmt) "serial: " fmt

#include "serial.h"

#include <stddef.h>

#include <cpp.h>
#include <device.h>
#include <console.h>
#include <print.h>

static const char * lastpos = NULL;

static int enable(const struct device * dev)
{
	struct serial * serial = dev->driver_data;
	int err = serial_init_port(serial->base);
	if (err)
		return err;
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

static void clear(UNUSED const struct device * dev)
{
}

int serial_console_reg(const struct device * dev)
{
	return console_reg(dev, update, clear, enable, disable);
}
