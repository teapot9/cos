#ifndef FIRMWARE_EFI_H
#define FIRMWARE_EFI_H

#include <stdbool.h>

#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/loaded_image.h>
#include <mm.h>
#include <string.h>

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
int efistub_memmap_and_exit(void);

/**
 * @brief Get kernel cmdline
 */
char * efistub_cmdline(void);

#endif // FIRMWARE_EFI_H
