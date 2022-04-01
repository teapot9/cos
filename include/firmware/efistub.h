#ifndef FIRMWARE_EFI_H
#define FIRMWARE_EFI_H

#include <stdbool.h>

#include <driver.h>
#include <firmware/efiapi/system_table.h>
#include <mm.h>
#include <string.h>

static inline bool efi_guid_t_eq(efi_guid_t a, efi_guid_t b)
{
	return a.data1 == b.data1 && a.data2 == b.data2 && a.data3 == b.data3
		&& memcmp(a.data4, b.data4, sizeof(a.data4)) == 0;
}

/**
 * @var Boolean to check if the driver is initialized
 */
extern bool efistub_is_init;

/**
 * @var Boolean to check if the console is initialized
 */
extern bool efistub_console_is_init;

extern bool efistub_boot_services;

/**
 * @var EFI image handle pointer
 */
extern efi_handle_t efistub_image_handle;

/**
 * @var EFI system table pointer
 */
extern efi_system_table_t * efistub_system_table;

/**
 * @brief Driver entry point
 */
void efistub_init(void);

/**
 * @brief Initialize the EFI console
 * @param driver EFI driver structure
 * @return 0 on success, non-zero on failure
 */
int efistub_console_init(struct driver * driver);

/**
 * @brief Disable the EFI console
 * @return 0 on success, error code if error
 */
int efistub_console_disable(void);

/**
 * @brief Clear the EFI console
 * @return 0 on success, error code if error
 */
int efistub_console_clear(void);

/**
 * @brief Reset the EFI console
 * @param id Reserved
 */
void efistub_console_reset(__attribute__((unused)) void * id);

/**
 * @brief Update the EFI console
 * @param id Reserved
 */
void efistub_console_update(__attribute__((unused)) void * id);

/// TODO: doc
int efistub_memmap_and_exit(struct memmap * map);

#endif // FIRMWARE_EFI_H
