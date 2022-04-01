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

struct efigop_bdata;
#ifdef BOOTLOADER
int gop_init(struct efigop_bdata ** data);
#else // BOOTLOADER
int gop_init(struct efigop_bdata * data);
#endif // !BOOTLOADER

int efistub_console_init(void);

int idt_init(void);

#ifdef BOOTLOADER
int vmm_init(void);
#else
struct vmemmap;
int vmm_init(struct vmemmap * map);
#endif

#endif // PLATFORM_SETUP_H
