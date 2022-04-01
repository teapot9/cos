#ifndef DEV_FIRMWARE_ACPI_ACPI_H
#define DEV_FIRMWARE_ACPI_ACPI_H

#include <stdint.h>
#include <stddef.h>

#include <cpp.h>

#define ACPI_RSDP_SIG "RSD PTR "
#define ACPI_REV_1 0
#define ACPI_REV_2 2

enum acpi_version {
	ACPI_VERSION_UNDEFINED = 0,
	ACPI_VERSION_1 = 1,
	ACPI_VERSION_2 = 2,
};

union rsdp {
	struct PACKED rsdp1 {
		char sig[8];
		uint8_t csum;
		char oem_id[6];
		uint8_t rev;
		uint32_t phy_rsdt_ptr;
	} rsdp1;
	struct PACKED rsdp2 {
		char sig[8];
		uint8_t csum;
		char oem_id[6];
		uint8_t rev;
		uint32_t phy_rsdt_ptr;
		uint32_t length;
		uint64_t phy_xsdt_ptr;
		uint8_t extended_csum;
		uint8_t _reserved0[3];
	} rsdp2;
};

struct PACKED description_header {
	char sig[4];
	uint32_t length;
	uint8_t rev;
	uint8_t csum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_rev;
	char creator_id[4];
	uint32_t creator_rev;
};

struct PACKED rsdt {
	struct description_header hdr;
	uint32_t phy_entry_ptr[];
};

struct PACKED xsdt {
	struct description_header hdr;
	uint64_t phy_entry_ptr[];
};

struct PACKED fadt {
	struct description_header hdr;
	uint32_t phy_firmware_ctrl;
	uint32_t phy_dsdt;
	uint8_t _reserved0;
	uint8_t preferred_pm_profile;
	uint16_t sci_int;
	uint32_t smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
};

void * acpi_rsdp_phy(void);
union rsdp * acpi_rsdp(void);
enum acpi_version acpi_version(void);
struct description_header * acpi_get_table(size_t index);

#endif // DEV_FIRMWARE_ACPI_ACPI_H
