#define pr_fmt(fmt) "efistub: " fmt

#include <firmware/efistub.h>
#include "efistub.h"
#include <platform_setup.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <device.h>
#include <console.h>
#include <module.h>
#include <print.h>
#include <firmware/efiapi/console.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>
#include <unicode.h>

#define BUFSIZ 256

static const struct device * eficon_dev = NULL;

static inline bool eficon_available(void)
{
	return eficon_dev != NULL && eficon_dev->available;
}

static int eficon_write(const char * string)
{
	uint16_t buffer[BUFSIZ];
	size_t size;
	efi_status_t status;

	if (string == NULL)
		return -EINVAL;

	while (*string) {
		size = utf_to_utf16_eol(buffer, string, BUFSIZ, u"\r\n");
		if (!size)
			return -EINVAL;

		if (!eficon_available())
			return -ENOENT;
		status = efistub_system_table()->con_out->output_string(
			efistub_system_table()->con_out, buffer
		);
		if (status != EFI_SUCCESS)
			return -EIO;
		string += size;
	}

	return 0;
}

static int eficon_reset(void)
{
	if (!eficon_available())
		return 0;

	efi_status_t ret = efistub_system_table()->con_out->reset(
		efistub_system_table()->con_out, 0
	);
	if (ret) {
		pr_err("Cannot reset EFI console, efi status = %d\n", ret);
		return -EIO;
	}
	return 0;
}

static int eficon_clear(void)
{
	efi_status_t status;

	if (!eficon_available())
		return -ENOENT;
	status = efistub_system_table()->con_out->clear_screen(
		efistub_system_table()->con_out
	);
	if (status != EFI_SUCCESS)
		return -EIO;
	return 0;
}

static const char * lastpos = NULL;

static int efistub_console_enable(const struct device * dev)
{
	if (!eficon_available())
		return -ENOTSUP;
	if (dev != eficon_dev)
		return -EINVAL;
	lastpos = NULL;
	return eficon_reset();
}

static void efistub_console_disable(const struct device * dev)
{
	if (!eficon_available())
		return;
	if (dev != eficon_dev)
		return;
	eficon_reset();
}

static void efistub_console_update(const struct device * dev)
{
	if (!eficon_available())
		return;
	if (dev != eficon_dev)
		return;

	const char * newpos = kmsg_next(lastpos);

	while (newpos != NULL) {
		eficon_write(kmsg_get_str(newpos));
		lastpos = newpos;
		newpos = kmsg_next(lastpos);
	}
}

static void efistub_console_clear(const struct device * dev)
{
	if (!eficon_available())
		return;
	if (dev != eficon_dev)
		return;
	if (eficon_clear())
		return;
	lastpos = NULL;
}

static int eficon_reg(const struct device * dev)
{
	int err;

	eficon_dev = dev;
	err = console_reg(dev, efistub_console_update, efistub_console_clear,
			  efistub_console_enable, efistub_console_disable);
	if (err) {
		eficon_dev = NULL;
		return err;
	}

	return 0;
}

static void eficon_unreg(__attribute__((__unused__)) const struct device * dev)
{
	eficon_dev = NULL;
}

/* public: platform_setup.h */
int efistub_console_init(void)
{
	int err = 0;

	err = device_create(NULL, &efistub_mod, efiboot_dev, "graphics", "con",
	                    eficon_reg, eficon_unreg, NULL, "eficon");
	if (err)
		return err;

	return 0;
}
