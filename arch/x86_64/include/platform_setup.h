#ifndef __PLATFORM_SETUP_H
#define __PLATFORM_SETUP_H

#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>

struct efistub_data;
#ifdef BOOT
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
#ifdef BOOT
int gop_init(struct gop ** data);
#else // BOOT
int gop_init(struct gop * gop);
struct gop * gop_save(struct gop * boot_gop);
#endif // !BOOT

int efistub_console_init(void);

int idt_init(void);

int vmm_init(void);

#endif // __PLATFORM_SETUP_H
