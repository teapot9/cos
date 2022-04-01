#ifndef DRIVERS_FIRMWARE_EFISTUB_EFISTUB_H
#define DRIVERS_FIRMWARE_EFISTUB_EFISTUB_H

#include <stdbool.h>

#include <firmware/efiapi/efiapi.h>
#include <string.h>

static inline bool efi_guid_t_eq(efi_guid_t a, efi_guid_t b)
{
	return a.data1 == b.data1 && a.data2 == b.data2 && a.data3 == b.data3
		&& memcmp(a.data4, b.data4, sizeof(a.data4)) == 0;
}

bool efistub_is_init(void);

extern const struct module efistub_mod;
extern const struct device * efiboot_dev;

void efistub_parse_configuration_table(void);

#endif // DRIVERS_FIRMWARE_EFISTUB_EFISTUB_H
