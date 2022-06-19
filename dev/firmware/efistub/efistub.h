#ifndef _DEV_FIRMWARE_EFISTUB_EFISTUB_H
#define _DEV_FIRMWARE_EFISTUB_EFISTUB_H

#include <stdbool.h>

#include <firmware/efiapi/efiapi.h>
#include <string.h>
#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/loaded_image.h>

struct efistub_data {
	efi_handle_t image_handle;
	const efi_system_table_t * system_table;
	const efi_loaded_image_protocol_t * image_proto;
};

static inline bool efi_guid_t_eq(efi_guid_t a, efi_guid_t b)
{
	return a.data1 == b.data1 && a.data2 == b.data2 && a.data3 == b.data3
		&& memcmp(a.data4, b.data4, sizeof(a.data4)) == 0;
}

bool efistub_is_init(void);

extern const struct module efistub_mod;
extern const struct device * efiboot_dev;

void efistub_parse_configuration_table(void);

#endif // _DEV_FIRMWARE_EFISTUB_EFISTUB_H
