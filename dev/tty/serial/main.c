#define pr_fmt(fmt) "serial: " fmt

#include "serial.h"
#include <setup.h>

#include <module.h>
#include <print.h>

static bool is_init = false;

struct module serial_mod = {
	.name = "serial",
};

/* public: setup.h */
int serial_init(void)
{
	if (is_init)
		return 0;
	pr_debug("init\n", 0);

	uint16_t ports[] = {STANDARD_BASE_COM1, STANDARD_BASE_COM2,
	                    STANDARD_BASE_COM3, STANDARD_BASE_COM4};

	for (int i = 0; i < 4; i++) {
		int err;
		if ((err = serial_reg(ports[i])))
			continue;
		pr_debug("port 0x%x: found serial port\n", ports[i]);
	}

	is_init = true;
	return 0;
}
module_init(serial_init, device);
