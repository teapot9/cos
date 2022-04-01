#include <firmware/efistub.h>

#include <stdbool.h>
#include <stddef.h>

#include <print.h>

bool efistub_is_init = false;
bool efistub_boot_services = false;
efi_handle_t efistub_image_handle = NULL;
efi_system_table_t * efistub_system_table = NULL;

static struct driver efistub_driver = {
	.name = "efistub",
	.init = efistub_init,
};

void efistub_init(void)
{
	if (efistub_is_init)
		return;
	if (efistub_image_handle == NULL || efistub_system_table == NULL)
		return;
	if (efistub_console_init(&efistub_driver))
		return;
	efistub_is_init = true;
	efistub_boot_services = true;
}
