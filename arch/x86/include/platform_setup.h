#ifndef PLATFORM_SETUP_H
#define PLATFORM_SETUP_H

#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>

int efistub_init(efi_handle_t image_handle,
                 const efi_system_table_t * system_table);

int gop_init(void);

int efistub_console_init(void);

int idt_init(void);

int vmm_init(void);

#endif // PLATFORM_SETUP_H
