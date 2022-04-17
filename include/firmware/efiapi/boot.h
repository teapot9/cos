#ifndef FIRMWARE_EFIAPI_BOOT_H
#define FIRMWARE_EFIAPI_BOOT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <firmware/efiapi/device_path.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>

/* Definitions */

typedef void * efi_event_t;

typedef enum {
	EFI_TIMER_CANCEL,
	EFI_TIMER_PERIODIC,
	EFI_TIMER_RELATIVE,
} efi_timer_delay_t;

#define EVT_TIMER 0x80000000
#define EVT_RUNTIME 0x40000000
#define EVT_NOTIFY_WAIT 0x00000100
#define EVT_NOTIFY_SIGNAL 0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES 0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 0x60000202

#define EFI_EVENT_GROUP_EXIT_BOOT_SERVICES \
	{0x27abf055, 0xb1b8, 0x4c26, 0x80, 0x48, 0x74, 0x8f, 0x37, \
	 0xba, 0xa2, 0xdf}
#define EFI_EVENT_GROUP_VIRTUAL_ADDRESS_CHANGE \
	{0x13fa7698, 0xuint8_t31, 0x49c7, 0x87, 0xea, 0x8f, 0x43, 0xfc, \
	 0xc2, 0x51, 0x96}
#define EFI_EVENT_GROUP_MEMORY_MAP_CHANGE \
	{0x78bee926, 0x692f, 0x48fd, 0x9e, 0xdb, 0x01, 0x42, 0x2e, \
	 0xf0, 0xd7, 0xab}
#define EFI_EVENT_GROUP_READY_TO_BOOT \
	{0x7ce88fb3, 0x4bd7, 0x4679, 0x87, 0xa8, 0xa8, 0xd8, 0xde, \
	 0xe5,0xd, 0x2b}

typedef efi_uintn efi_tpl_t;

#define EFI_TPL_APPLICATION 4
#define EFI_TPL_CALLBACK 8
#define EFI_TPL_NOTIFY 16
#define EFI_TPL_HIGH_LEVEL 31

typedef enum {
	EFI_ALLOCATE_ANY_PAGES,
	EFI_ALLOCATE_MAX_ADDRESS,
	EFI_ALLOCATE_ADDRESS,
	EFI_MAX_ALLOCATE_TYPE,
} efi_allocate_type_t;

typedef enum {
	EFI_NATIVE_INTERFACE,
} efi_interface_type_t;

typedef enum {
	EFI_ALL_HANDLES,
	EFI_BY_REGISTER_NOTIFY,
	EFI_BY_PROTOCOL,
} efi_locate_search_type_t;

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL 0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL 0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL 0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER 0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE 0x00000020

typedef struct {
	efi_handle_t agent_handle;
	efi_handle_t controller_handle;
	uint32_t attributes;
	uint32_t open_count;
} efi_open_protocol_entry_t;

/* Event, timer and task priority services */

typedef void (EFIABI * efi_event_notify_t) (
	efi_event_t event, void * context
);

typedef efi_status_t (EFIABI * efi_create_event_t) (
	uint32_t type, efi_tpl_t notify_tpl, efi_event_notify_t notify_function,
	void * notify_context, efi_event_t * event
);

typedef efi_status_t (EFIABI * efi_create_event_ex_t) (
	uint32_t type, efi_tpl_t notify_tpl, efi_event_notify_t notify_function,
	const void * notify_context, const efi_guid_t event_group,
	efi_event_t * event
);

typedef efi_status_t (EFIABI * efi_close_event_t) (efi_event_t event);

typedef efi_status_t (EFIABI * efi_signal_event_t) (efi_event_t event);

typedef efi_status_t (EFIABI * efi_wait_for_event_t) (
	efi_uintn number_of_events, efi_event_t * event, efi_uintn * index
);

typedef efi_status_t (EFIABI * efi_check_event_t) (efi_event_t event);

typedef efi_status_t (EFIABI * efi_set_timer_t) (
	efi_event_t event, efi_timer_delay_t type, uint64_t trigger_time
);

typedef efi_tpl_t (EFIABI * efi_raise_tpl_t) (efi_tpl_t new_tpl);

typedef void (EFIABI * efi_restore_tpl_t) (efi_tpl_t old_tpl);

/* Memory allocation services */

typedef efi_status_t (EFIABI * efi_allocate_pages_t) (
	efi_allocate_type_t type, efi_memory_type_t memory_type,
	efi_uintn pages, efi_physical_address_t * memory
);

typedef efi_status_t (EFIABI * efi_free_pages_t) (
	efi_physical_address_t memory, efi_uintn pages
);

typedef efi_status_t (EFIABI * efi_get_memory_map_t) (
	efi_uintn * memory_map_size, efi_memory_descriptor_t * memory_map,
	efi_uintn * map_key, efi_uintn * descriptor_size,
	uint32_t * descriptor_version
);

typedef efi_status_t (EFIABI * efi_allocate_pool_t) (
	efi_memory_type_t pool_type, efi_uintn size, void * buffer
);

typedef efi_status_t (EFIABI * efi_free_pool_t) (void * buffer);

/* Protocol handler services */

typedef efi_status_t (EFIABI * efi_install_protocol_interface_t) (
	efi_handle_t * handle, efi_guid_t * protocol,
	efi_interface_type_t interface_type, void * interface
);

typedef efi_status_t (EFIABI * efi_uninstall_protocol_interface_t) (
	efi_handle_t handle, efi_guid_t * protocol, void * interface
);

typedef efi_status_t (EFIABI * efi_reinstall_protocol_interface_t) (
	efi_handle_t handle, efi_guid_t * protocol,
	void * old_interface, void * new_interface
);

typedef efi_status_t (EFIABI * efi_register_protocol_notify_t) (
	efi_guid_t * protocol, efi_event_t event, void ** registration
);

typedef efi_status_t (EFIABI * efi_locate_handle_t) (
	efi_locate_search_type_t search_type, efi_guid_t * protocol,
	void * search_key, efi_uintn * buffer_size, efi_handle_t * buffer
);

typedef efi_status_t (EFIABI * efi_handle_protocol_t) (
	efi_handle_t handle, efi_guid_t * protocol, void ** interface
);

typedef efi_status_t (EFIABI * efi_locate_device_path_t) (
	efi_guid_t * protocol, efi_device_path_protocol_t ** device_path,
	efi_handle_t * device
);

typedef efi_status_t (EFIABI * efi_open_protocol_t) (
	efi_handle_t handle, efi_guid_t * protocol, void ** interface,
	efi_handle_t agent_handle, efi_handle_t controller_handle,
	uint32_t attributes
);

typedef efi_status_t (EFIABI * efi_close_protocol_t) (
	efi_handle_t handle, efi_guid_t * protocol,
	efi_handle_t agent_handle, efi_handle_t controller_handle
);

typedef efi_status_t (EFIABI * efi_open_protocol_information_t) (
	efi_handle_t handle, efi_guid_t * protocol,
	efi_open_protocol_entry_t ** entry_buffer, efi_uintn * entry_count
);

typedef efi_status_t (EFIABI * efi_connect_controller_t) (
	efi_handle_t controller_handle, efi_handle_t * driver_image_handle,
	efi_device_path_protocol_t * remaining_device_path, efi_bool recursive
);

typedef efi_status_t (EFIABI * efi_disconnect_controller_t) (
	efi_handle_t controller_handle, efi_handle_t driver_image_handle,
	efi_handle_t child_handle
);

typedef efi_status_t (EFIABI * efi_protocols_per_handle_t) (
	efi_handle_t handle, efi_guid_t *** protocol_buffer,
	efi_uintn * protocol_buffer_count
);

typedef efi_status_t (EFIABI * efi_locate_handle_buffer_t) (
	efi_locate_search_type_t search_type, efi_guid_t * protocol,
	void * search_key, efi_uintn * no_handles, efi_handle_t ** buffer
);

typedef efi_status_t (EFIABI * efi_locate_protocol_t) (
	efi_guid_t * protocol, void * registration, void ** interface
);

typedef efi_status_t (EFIABI * efi_install_multiple_protocol_interfaces_t) (
	efi_handle_t handle, ...
);

typedef efi_status_t (EFIABI * efi_uninstall_multiple_protocol_interfaces_t) (
	efi_handle_t handle, ...
);

/* Image services */

typedef efi_status_t (EFIABI * efi_image_load_t) (
	efi_bool boot_policy,
	efi_handle_t parent_image_handle,
	efi_device_path_protocol_t * device_path,
	void * source_buffer,
	efi_uintn source_size,
	efi_handle_t * image_handle
);

typedef efi_status_t (EFIABI * efi_image_start_t) (
	efi_handle_t image_handle,
	efi_uintn * exit_data_size,
	uint16_t ** exit_data
);

typedef efi_status_t (EFIABI * efi_image_unload_t) (
	efi_handle_t image_handle
);

typedef efi_status_t (EFIABI * efi_image_entry_point_t) (
	efi_handle_t image_handle,
	efi_system_table_t * system_table
);

typedef efi_status_t (EFIABI * efi_exit_t) (
	efi_handle_t image_handle,
	efi_status_t exit_status,
	efi_uintn exit_data_size,
	uint16_t * exit_data
);

typedef efi_status_t (EFIABI * efi_exit_boot_services_t) (
	efi_handle_t image_handle,
	efi_uintn map_key
);

/* Miscellaneous boot services*/

typedef efi_status_t (EFIABI * efi_set_watchdog_timer_t) (
	efi_uintn timeout, uint64_t watchdog_code, efi_uintn data_size,
	uint16_t * watchdog_data
);

typedef efi_status_t (EFIABI * efi_stall_t) (efi_uintn microseconds);

typedef void (EFIABI * efi_copy_mem_t) (
	void * destination, void * source, efi_uintn length
);

typedef void (EFIABI * efi_set_mem_t) (
	void * buffer, efi_uintn size, uint8_t value
);

typedef efi_status_t (EFIABI * efi_get_next_monotonic_count_t) (uint64_t * count);

typedef efi_status_t (EFIABI * efi_install_configuration_table_t) (
	efi_guid_t * guid, void * table
);

typedef efi_status_t (EFIABI * efi_calculate_crc32_t) (
	void * data, efi_uintn data_size, uint32_t * crc32
);

/* EFI boot serices structure */

#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42

typedef struct _efi_boot_services {
	efi_table_header_t hdr;
	// Task priority services
	efi_raise_tpl_t raise_tpl;
	efi_restore_tpl_t restore_tpl;
	// Memory services
	efi_allocate_pages_t allocate_pages;
	efi_free_pages_t free_pages;
	efi_get_memory_map_t get_memory_map;
	efi_allocate_pool_t allocate_pool;
	efi_free_pool_t free_pool;
	// Event and timer services
	efi_create_event_t create_event;
	efi_set_timer_t set_timer;
	efi_wait_for_event_t wait_for_event;
	efi_signal_event_t signal_event;
	efi_close_event_t close_event;
	efi_check_event_t check_event;
	// Protocol handler services
	efi_install_protocol_interface_t install_protocol_interface;
	efi_reinstall_protocol_interface_t reinstall_protocol_interface;
	efi_uninstall_protocol_interface_t uninstall_protocol_interface;
	efi_handle_protocol_t handle_protocol;
	void * _reserved;
	efi_register_protocol_notify_t register_protocol_notify;
	efi_locate_handle_t locate_handle;
	efi_locate_device_path_t locate_device_path;
	efi_install_configuration_table_t install_configuration_table;
	// Image services
	efi_image_load_t load_image;
	efi_image_start_t start_image;
	efi_exit_t exit;
	efi_image_unload_t unload_image;
	efi_exit_boot_services_t exit_boot_services;
	// Miscellaneous services
	efi_get_next_monotonic_count_t get_next_monotonic_count;
	efi_stall_t stall;
	efi_set_watchdog_timer_t set_watchdog_timer;
	// Driver support services
	efi_connect_controller_t connect_controller;
	efi_disconnect_controller_t disconnect_controler;
	// Open and close protocol services
	efi_open_protocol_t open_protocol;
	efi_close_protocol_t close_protocol;
	efi_open_protocol_information_t open_protocol_information;
	// Library services
	efi_protocols_per_handle_t protocols_per_handle;
	efi_locate_handle_buffer_t locate_handle_buffer;
	efi_locate_protocol_t locate_protocol;
	efi_install_multiple_protocol_interfaces_t
		install_multiple_protocol_interfaces;
	efi_uninstall_multiple_protocol_interfaces_t
		uninstall_multiple_protocol_interfaces;
	// Hash services
	efi_calculate_crc32_t calculate_crc32;
	// Miscellaneous services
	efi_copy_mem_t copy_mem;
	efi_set_mem_t set_mem;
	efi_create_event_ex_t create_event_ex;
} efi_boot_services_t ;

#ifdef __cplusplus
}
#endif
#endif // FIRMWARE_EFIAPI_BOOT_H
