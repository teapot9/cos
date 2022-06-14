#define pr_fmt(fmt) "serial: " fmt

#include "serial.h"

#include <errno.h>
#include <stddef.h>

#include <device.h>
#include <alloc.h>
#include <printk.h>
#include <kconfig.h>

static size_t serial_count = 0;

static int reg(const struct device * device)
{
	int err;
	struct serial * serial = device->driver_data;

	serial_reset(serial->base);

	// Check there is a serial port
	err = serial_init_port(serial->base);
	if (err) {
		pr_debug("port 0x%x: not a working serial port, errno = %d\n",
			 serial->base, err);
		return err;
	}

	pr_info("port 0x%x: new serial device [%s]\n",
	        serial->base, uart_id_str(serial->type));

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
	err = serial_console_reg(device);
	if (err)
		pr_err("port 0x%x: failed to create console, errno = %d\n",
		       serial->base, err);
	else
		pr_info("port 0x%x: serial console enabled\n", serial->base);
#endif

	return 0;
}

static void unreg(const struct device * device)
{
	kfree(device->driver_data);
}

int serial_reg(uint16_t base)
{
	int err;

	struct serial * serial = malloc(sizeof(*serial));
	if (serial == NULL)
		return -ENOMEM;

	serial->base = base;
	serial->type = serial_id(base);

	err = device_create(NULL, &serial_mod, NULL, "tty", "serial",
			    reg, unreg, serial, "ttyS%zu", serial_count++);
	if (err) {
		kfree(serial);
		return err;
	}

	return 0;
}
