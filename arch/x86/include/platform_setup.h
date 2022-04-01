#ifndef PLATFORM_SETUP_H
#define PLATFORM_SETUP_H

#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>

struct efistub_bdata;
#ifdef BOOTLOADER
int efistub_init(
	struct efistub_bdata ** data,
	efi_handle_t image_handle,
	const efi_system_table_t * system_table
);
#else
int efistub_init(
	struct efistub_bdata * data
);
#endif

struct efigop_bdata;
#ifdef BOOTLOADER
int gop_init(struct efigop_bdata ** data);
#else // BOOTLOADER
int gop_init(struct efigop_bdata * data);
#endif // !BOOTLOADER

int efistub_console_init(void);

int idt_init(void);

int vmm_init(void);

#endif // PLATFORM_SETUP_H
