#ifndef FIRMWARE_EFIAPI_LOADED_IMAGE_H
#define FIRMWARE_EFIAPI_LOADED_IMAGE_H

#include <firmware/efiapi/device_path.h>
#include <firmware/efiapi/system_table.h>

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
	{0x5B1B31A1, 0x9562, 0x11d2, \
	 {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}

#define EFI_LOADED_IMAGE_PROTOCOL_REVISION 0x1000

typedef struct {
	uint32_t revision;
	efi_handle_t parent_handle;
	efi_system_table_t * system_table;
	efi_handle_t device_handle;
	efi_device_path_protocol_t * file_path;
	void * reserved;
	uint32_t load_options_size;
	uint16_t * load_options;
	void * image_base;
	uint64_t image_size;
	efi_memory_type_t image_code_type;
	efi_memory_type_t image_data_type;
	efi_image_unload_t unload;
} efi_loaded_image_protocol_t;

#endif // FIRMWARE_EFIAPI_LOADED_IMAGE_H
