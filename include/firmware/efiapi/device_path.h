#ifndef FIRMWARE_EFIAPI_DEVICE_PATH_H
#define FIRMWARE_EFIAPI_DEVICE_PATH_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
	{0x09576e91, 0x6d3f, 0x11d2, \
	 {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

typedef struct {
	uint8_t type;
	uint8_t sub_type;
	uint8_t length[2];
} efi_device_path_protocol_t;

#ifdef __cplusplus
}
#endif
#endif // FIRMWARE_EFIAPI_DEVICE_PATH_H
