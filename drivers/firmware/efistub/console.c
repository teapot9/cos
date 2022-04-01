#include <firmware/efistub.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <console.h>
#include <print.h>
#include <firmware/efiapi/console.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>
#include <unicode.h>

#define BUFSIZ 256

bool efistub_console_is_init = false;
static void * efistub_console_id = NULL;

int efistub_console_init(struct driver * driver)
{
	int err = 0;

	if (driver == NULL || efistub_system_table == NULL)
		return -EINVAL;
	if (efistub_system_table->con_out->reset(
		efistub_system_table->con_out, 0
	    ) != EFI_SUCCESS)
		return -EIO;

	err = console_add(efistub_console_id,
	                  efistub_console_update, efistub_console_reset);
	if (err)
		return err;

	efistub_console_is_init = true;
	return 0;
}

int efistub_console_disable(void)
{
	int err = 0;

	if (!efistub_console_is_init)
		return -EINVAL;

	if ((err = console_remove(efistub_console_id)))
		return err;
	efistub_console_is_init = false;
	return 0;
}

int efistub_console_clear(void)
{
	if (!efistub_console_is_init)
		return -EINVAL;
	if (!efistub_is_init)
		return -EINVAL;
	if (efistub_system_table->con_out->clear_screen(
		efistub_system_table->con_out
	    ) != EFI_SUCCESS)
		return -EIO;
	return 0;
}

static int efistub_console_write(const char * string)
{
	uint16_t buffer[BUFSIZ];
	size_t size;
	efi_status_t status;

	if (!efistub_is_init)
		return -EINVAL;
	if (!efistub_console_is_init)
		return -EINVAL;
	if (string == NULL)
		return -EINVAL;

	while (*string) {
		size = utf_to_utf16_eol(buffer, string, BUFSIZ, u"\r\n");
		if (!size)
			return -EINVAL;

		status = efistub_system_table->con_out->output_string(
			efistub_system_table->con_out, buffer
		);
		if (status != EFI_SUCCESS)
			return -EIO;
		string += size;
	}

	return 0;
}

static const char * lastpos = NULL;

void efistub_console_update(__attribute__((unused)) void * id)
{
	if (!efistub_console_is_init)
		return;

	const char * newpos = kmsg_next(lastpos);

	while (newpos != NULL) {
		efistub_console_write(kmsg_get_str(newpos));
		lastpos = newpos;
		newpos = kmsg_next(lastpos);
	}
}

void efistub_console_reset(__attribute__((unused)) void * id)
{
	if (!efistub_console_is_init)
		return;
	if (efistub_console_clear())
		return;
	lastpos = NULL;
}
