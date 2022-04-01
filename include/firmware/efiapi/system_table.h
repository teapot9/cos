#ifndef FIRMWARE_EFIAPI_SYSTEM_TABLE_H
#define FIRMWARE_EFIAPI_SYSTEM_TABLE_H

#include <firmware/efiapi/efiapi.h>

/* System table */

typedef struct {
	efi_guid_t vendor_guid;
	void * vendor_table;
} efi_configuration_table_t;

#define EFI_ACPI_20_TABLE_GUID \
	{0x8868e871, 0xe4f1, 0x11d3, \
	 {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}}
#define EFI_ACPI_TABLE_GUID EFI_ACPI_20_TABLE_GUID
#define ACPI_10_TABLE_GUID \
	{0xeb9d2d30, 0x2d88, 0x11d3, \
	 {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
#define ACPI_TABLE_GUID ACPI_10_TABLE_GUID
#define SAL_SYSTEM_TABLE_GUID \
	{0xeb9d2d32, 0x2d88, 0x11d3, \
	 {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
#define SMBIOS_TABLE_GUID \
	{0xeb9d2d31, 0x2d88, 0x11d3, \
	 {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
#define SMBIOS3_TABLE_GUID \
	{0xf2fd1544, 0x9794, 0x4a2c, \
	 {0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94}}
#define MPS_TABLE_GUID \
	{0xeb9d2d2f, 0x2d88, 0x11d3, \
	 {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
#define EFI_PROPERTIES_TABLE_GUID \
	{0x880aaca3, 0x4adc, 0x4a04, \
	 {0x90, 0x79, 0xb7, 0x47, 0x34, 0x8, 0x25, 0xe5}}

# define EFI_PROPERTIES_TABLE_VERSION 0x00010000

typedef struct {
	uint32_t version;
	uint32_t length;
	uint64_t memory_protection_attribute;
} efi_properties_table_t;

#define EFI_PROPERTIES_RUNTIME_MEMORY_PROTECTION_NON_EXECUTABLE_PE_DATA 0x1

#define EFI_SYSTEM_TABLE_SIGNATURE 0x5453595320494249

#define EFI_2_60_SYSTEM_TABLE_REVISION  ((2 << 16) | 60)
#define EFI_2_50_SYSTEM_TABLE_REVISION  ((2 << 16) | 50)
#define EFI_2_40_SYSTEM_TABLE_REVISION  ((2 << 16) | 40)
#define EFI_2_31_SYSTEM_TABLE_REVISION  ((2 << 16) | 31)
#define EFI_2_30_SYSTEM_TABLE_REVISION  ((2 << 16) | 30)
#define EFI_2_20_SYSTEM_TABLE_REVISION  ((2 << 16) | 20)
#define EFI_2_10_SYSTEM_TABLE_REVISION  ((2 << 16) | 10)
#define EFI_2_00_SYSTEM_TABLE_REVISION  ((2 << 16) | 0)
#define EFI_1_10_SYSTEM_TABLE_REVISION  ((1 << 16) | 10)
#define EFI_1_02_SYSTEM_TABLE_REVISION  ((1 << 16) | 2)

typedef struct {
	efi_table_header_t hdr;
	uint16_t *firmware_vendor;
	uint32_t firmware_revision;
	efi_handle_t console_in_handle;
	struct _efi_simple_text_input_protocol *con_in;
	efi_handle_t console_out_handle;
	struct _efi_simple_text_output_protocol *con_out;
	efi_handle_t standard_error_handle;
	struct _efi_simple_text_output_protocol *stderr;
	struct _efi_runtime_services *runtime_services;
	struct _efi_boot_services *boot_services;
	efi_uintn number_of_table_entries;
	efi_configuration_table_t *configuration_table;
} efi_system_table_t;

#endif // FIRMWARE_EFIAPI_SYSTEM_TABLE_H
