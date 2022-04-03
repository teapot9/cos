#define pr_fmt(fmt) "efistub: " fmt

#include <firmware/efistub.h>
#include "efistub.h"

#include <errno.h>

#include <print.h>
#include <unicode.h>
#include <kconfig.h>

#if IS_ENABLED(CONFIG_ACPI)
# include <firmware/acpi.h>
#endif

const char * efistub_firmware_cmdline(void)
{
	char * cmdline = NULL;
	size_t size;

	if (efistub_image_proto() == NULL)
		return NULL;

	if (efistub_image_proto()->load_options_size == 0
	    || efistub_image_proto()->load_options == NULL)
		return NULL;

	size = efistub_image_proto()->load_options_size * 2;
	cmdline = malloc(size);
	if (cmdline == NULL) {
		pr_err("Failed to allocate %zu bytes for the EFI "
		       "cmdline buffer", size);
		kfree(cmdline);
		return NULL;
	}

	if (utf16_to_utf(cmdline, efistub_image_proto()->load_options, size)
	    == 0) {
		pr_err("Failed to convert UTF-16 to UTF-8 UEFI cmdline\n", 0);
		kfree(cmdline);
		return NULL;
	}

	return cmdline;
}

void efistub_parse_configuration_table(void)
{
	const efi_system_table_t * sys = efistub_system_table();
	if (sys == NULL)
		return;
	efi_configuration_table_t * cfg = sys->configuration_table;
	if (cfg == NULL)
		return;

	for (efi_uintn i = 0; i < sys->number_of_table_entries; i++) {
		if (efi_guid_t_eq(cfg[i].vendor_guid,
		                  (efi_guid_t) EFI_ACPI_20_TABLE_GUID)) {
#if 0
#if IS_ENABLED(CONFIG_ACPI)
			acpi_register_rsdp(cfg[i].vendor_table);
#endif
#endif
		} else if (efi_guid_t_eq(cfg[i].vendor_guid,
		                         (efi_guid_t) ACPI_10_TABLE_GUID)) {
#if 0
#if IS_ENABLED(CONFIG_ACPI)
			acpi_register_rsdp(cfg[i].vendor_table);
#endif
#endif
		}
	}
}
