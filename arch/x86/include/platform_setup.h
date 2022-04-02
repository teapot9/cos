#ifndef PLATFORM_SETUP_H
#define PLATFORM_SETUP_H

#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>

struct efistub_data;
#ifdef BOOTLOADER
int efistub_init(
	struct efistub_data ** data,
	efi_handle_t image_handle,
	const efi_system_table_t * system_table
);
#else
int efistub_init(
	struct efistub_data * data
);
#endif

struct gop;
#ifdef BOOTLOADER
int gop_init(struct gop ** data);
#else // BOOTLOADER
int gop_init(struct gop * gop);
struct gop * gop_save(struct gop * boot_gop);
#endif // !BOOTLOADER

int efistub_console_init(void);

int idt_init(void);

int vmm_init(void);

#endif // PLATFORM_SETUP_H
