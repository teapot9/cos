#include "entry_efi.h"

#include <errno.h>
#include <stddef.h>
#include <stdnoreturn.h>

#include <list.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <sched.h>
#include <task.h>
#include "../../../kernel/task.h"
#include <cpu.h>
#include <asm/asm.h>
#include <console.h>
#include <firmware/efistub.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/boot.h>
#include <firmware/efiapi/loaded_image.h>
#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/console.h>
#include <fonts.h>
#include <mm.h>
#include <mm/early.h>
#include <print.h>
#include <debug.h>
#include <string.h>
#include <unicode.h>
#include <video/fb.h>
#include <asm/io.h>
#include <platform_setup.h>
#include <setup.h>
#include <cmdline.h>
#include <panic.h>
#include <mm/memmap.h>
#include <setup.h>
#include <elf.h>

struct entry_efi_data boot_data;
extern uint8_t _binary_cos_elf_start[];
extern uint8_t _binary_cos_elf_end[];

extern noreturn void entry_efi_wrapper(
	efi_handle_t image_handle, efi_system_table_t * system_table
);

#ifdef CONFIG_DEBUG
static void debug_base_addr(efi_handle_t image_handle,
                            efi_system_table_t * system_table)
{
	efi_guid_t image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	efi_loaded_image_protocol_t * image;
	system_table->boot_services->handle_protocol(
		image_handle, &image_guid, (void *) &image
	);

	char buf[100];
	snprintf(buf, 100, "Kernel loaded at %p\n", image->image_base);

	uint16_t buff[100];
	utf_to_utf16_eol(buff, buf, 100, u"\r\n");
	system_table->con_out->output_string(
		system_table->con_out, buff
	);
}
#endif

noreturn static void trampoline(void (* fcn)(void))
{
	fcn();
	panic("second stage boot should not return");
}

noreturn void entry_efi_s1(efi_handle_t image_handle,
                           efi_system_table_t * system_table)
{
	int err = 0;

#ifdef CONFIG_DEBUG
	debug_base_addr(image_handle, system_table);
	bool tmp = false;
	while (!tmp);
#endif

	/* EFI stub */
	err = efistub_init(&boot_data.efistub, image_handle, system_table);
	if (err)
		panic("failed to initialize EFI stub, errno = %d", err);
	pr_info("Early init: EFI stub\n", 0);

	/* EFI console */
	err = efistub_console_init();
	if (err)
		pr_err("Early init: failed to initialize EFI console, "
		       "errno = %d\n", err);
	else
		pr_info("Early init: EFI console\n", 0);

	/* cmdline */
	err = efistub_cmdline(&boot_data.cmdline);
	kernel_cmdline = boot_data.cmdline;
	if (err)
		panic("failed to get cmdline, errno = %d", err);
	pr_info("Early init: cmdline: %s\n", kernel_cmdline);

	/* EFI GOP */
	gop_init(&boot_data.gop);
	pr_info("Early init: EFI GOP\n", 0);

	/* memmap & EFI exit */
	struct memmap map = memmap_new();
	if ((err = efistub_memmap_and_exit(&map))) {
		panic("Cannot get memory map or exit boot services"
		      ", errno = %d\n", err);
	}
	pr_info("Early init: exited boot services\n", 0);
#ifdef CONFIG_MM_DEBUG
	memmap_print(&map, "pmemmap");
#endif

#ifdef CONFIG_SERIAL_EARLY_DEBUG
	/* Serial debug */
	serial_init();
#endif

	/* pmm */
	pmm_init(&map);
	memmap_free(&map, false);

	/* vmm */
	vmm_init();

	void (* kentry)(void);
	elf_load(&kentry, _binary_cos_elf_start,
	         _binary_cos_elf_end - _binary_cos_elf_end);
	trampoline(kentry);
}
