/**
 * @file firmware/efistub.h
 * @brief EFI stub library
 */

#ifndef __FIRMWARE_EFISTUB_H
#define __FIRMWARE_EFISTUB_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/loaded_image.h>
#include <alloc.h>
#include <string.h>

struct memmap;
struct memlist;

/**
 * @brief Kernel EFI image handle
 */
efi_handle_t efistub_image_handle(void);

/**
 * @brief EFI system table pointer
 */
const efi_system_table_t * efistub_system_table(void);

/**
 * @brief Get EFI loaded image protocol
 */
const efi_loaded_image_protocol_t * efistub_image_proto(void);

/**
 * @brief Get memory map and exit boot services
 */
int efistub_memmap_and_exit(struct memmap * map);

/**
 * @brief Get EFI cmdline
 *
 * Return cmdline in a dynamically allocated buffer
 */
const char * efistub_firmware_cmdline(void);

#ifdef __cplusplus
}
#endif
#endif // __FIRMWARE_EFISTUB_H
