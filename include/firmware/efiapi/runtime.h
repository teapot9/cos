#ifndef __FIRMWARE_EFIAPI_RUNTIME_H
#define __FIRMWARE_EFIAPI_RUNTIME_H
#ifdef __cplusplus
extern "C" {
#endif

#include <firmware/efiapi/efiapi.h>

/* Definitions */

#define EFI_VARIABLE_NON_VOLATILE 0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS 0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE 0x00000040

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t _pad1;
	uint32_t nanosecond;
	int16_t time_zone;
	uint8_t day_light;
	uint8_t _pad2;
} efi_time_t;

#define EFI_TIME_ADJUST_DAYLIGHT 0x01
#define EFI_TIME_IN_DAYLIGHT 0x02

#define EFI_UNSPECIFIED_TIMEZONE 0x07FF

typedef struct {
	uint32_t resolution;
	uint32_t accuracy;
	efi_bool sets_to_zero;
} efi_time_capabilities_t;
#define EFI_OPTIONAL_PTR 0x00000001

#define EFI_OS_INDICATIONS_BOOT_TO_FW_UI 0x0000000000000001
#define EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION 0x0000000000000002
#define EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED 0x0000000000000004
#define EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED 0x0000000000000008
#define EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED 0x0000000000000010
#define EFI_OS_INDICATIONS_START_OS_RECOVERY 0x0000000000000020
#define EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY 0x0000000000000040

typedef enum {
	EFI_RESET_COLD,
	EFI_RESET_WARM,
	EFI_RESET_SHUTDOWN,
	EFI_RESET_PLATFORM_SPECIFIC,
} efi_reset_type_t;

typedef struct {
	uint64_t length;
	union {
		efi_physical_address_t data_block;
		efi_physical_address_t continuation_pointer;
	} value;
} efi_capsule_block_descriptor_t;

#define EFI_CAPSULE_FLAGS_PERSIST_ACROSS_RESET 0x00010000
#define EFI_CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE 0x00020000
#define EFI_CAPSULE_FLAGS_INITIATE_RESET 0x00040000

typedef struct {
	efi_guid_t capsule_guid;
	uint32_t header_size;
	uint32_t flags;
	uint32_t capsule_image_size;
} efi_capsule_header_t;

/* Variable services */

typedef efi_status_t (EFIABI * efi_get_variable_t) (
	uint16_t * variable_name, efi_guid_t * vendor_guid, uint32_t * attributes,
	efi_uintn * data_size, void * data
);

typedef efi_status_t (EFIABI * efi_get_next_variable_name_t) (
	efi_uintn * variable_name_size, uint16_t * variable_name,
	efi_guid_t * vendor_guid
);

typedef efi_status_t (EFIABI * efi_set_variable_t) (
	uint16_t * variable_name, efi_guid_t * vendor_guid, uint32_t attributes,
	efi_uintn data_size, void * data
);

typedef efi_status_t (EFIABI * efi_query_variable_info_t) (
	uint32_t attributes, uint64_t * maximum_variable_storage_size,
	uint64_t * remaining_variable_storage_size, uint64_t * maximum_variable_size
);

/* Time services */

typedef efi_status_t (EFIABI * efi_get_time_t) (
	efi_time_t * time, efi_time_capabilities_t * capabilities
);

typedef efi_status_t (EFIABI * efi_set_time_t) (efi_time_t * time);

typedef efi_status_t (EFIABI * efi_get_wakeup_time_t) (
	efi_bool * enabled, efi_bool * pending, efi_time_t * time
);

typedef efi_status_t (EFIABI * efi_set_wakeup_time_t) (
	efi_bool enable, efi_time_t * time
);

/* Virtual memory services */

typedef efi_status_t (EFIABI * efi_set_virtual_address_map_t) (
	efi_uintn memory_map_size, efi_uintn descriptor_size,
	uint32_t descriptor_version, efi_memory_descriptor_t * virtual_map
);

typedef efi_status_t (EFIABI * efi_convert_pointer_t) (
	efi_uintn debug_disposition, void ** address
);

/* Miscellaneous services */

typedef void (EFIABI __attribute__ ((noreturn)) * efi_reset_system_t) (
	efi_reset_type_t reset_type, efi_status_t reset_status,
	efi_uintn data_size, void * reset_data
);

typedef efi_status_t (EFIABI * efi_get_next_high_monotonic_count_t) (
	uint32_t * high_count
);

typedef efi_status_t (EFIABI * efi_update_capsule_t) (
	efi_capsule_header_t ** capsule_header_array,
	efi_uintn capsule_count, efi_physical_address_t scatter_gather_list
);

typedef efi_status_t (EFIABI * efi_query_capsule_capabilities_t) (
	efi_capsule_header_t ** capsule_header_array, efi_uintn capsule_count,
	uint64_t * maximum_capsule_size, efi_reset_type_t * reset_type
);

/* EFI runtime services table */

#define EFI_RUNTIME_SERVICES_SIGNATURE 0x56524553544e5552

typedef struct _efi_runtime_services {
	efi_table_header_t hdr;
	// Time services
	efi_get_time_t get_time;
	efi_set_time_t set_time;
	efi_get_wakeup_time_t get_wakeup_time;
	efi_set_wakeup_time_t set_wakeup_time;
	// Virtual memory services
	efi_set_virtual_address_map_t set_virtual_address_map;
	efi_convert_pointer_t convert_pointer;
	// Variable services
	efi_get_variable_t get_variable;
	efi_get_next_variable_name_t get_next_variable_name;
	efi_set_variable_t set_variable;
	// Miscellaneous services
	efi_get_next_high_monotonic_count_t get_next_high_monotonic_count;
	efi_reset_system_t reset_system;
	// Capsule services
	efi_update_capsule_t update_capsule;
	efi_query_capsule_capabilities_t query_capsule_capabilities;
	// Miscellaneous services
	efi_query_variable_info_t query_variable_info;
} efi_runtime_services_t;

#ifdef __cplusplus
}
#endif
#endif // __FIRMWARE_EFIAPI_RUNTIME_H
