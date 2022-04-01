#include <stddef.h>
#include <stdnoreturn.h>

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
#include <print.h>
#include <debug.h>
#include <string.h>
#include <unicode.h>
#include <video/fb.h>
#include <asm/io.h>
#include <init.h>
#include <int.h>

#include "../../../mm/pmm.h"
#include "../mm/vmm.h"

noreturn EFIABI void entry_efi(efi_handle_t image_handle,
               efi_system_table_t * system_table);

noreturn static void halt(void);

static void debug_base_addr(efi_handle_t image_handle, efi_system_table_t * system_table)
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

EFIABI void entry_efi(efi_handle_t image_handle,
                      efi_system_table_t * system_table)
{
	int ret;

	debug_base_addr(image_handle, system_table);
	kbreak();

	efistub_early_setup(image_handle, system_table);
	pr_info("Early init: EFI stub\n", 0);

	kernel_initcalls_early();
	pr_info("Early init: modules\n", 0);

	if ((ret = efistub_memmap_and_exit())) {
		pr_emerg("Cannot get memory map or exit boot services"
			 ", errno = %d\n", ret);
		halt();
	}
	pr_info("Early init: exited boot services\n", 0);

	idt_init();
	pmm_init();
	vmm_init();
	struct vmalloc alloc = vmalloc(6000);

	halt();

	efistub_early_setup(image_handle, system_table);
	//pr_info("Kernel booted from EFI\n", 0);

	debug_base_addr(image_handle, system_table);
	kbreak();

	/* mm */
	pr_debug("Kernel loaded at %p\n", efistub_image_proto()->image_base);
	pr_info("Initialized memory manager\n", 0);

	/* efistub */
	//efistub_image_handle = image_handle;
	//efistub_system_table = system_table;
	efistub_early_setup(image_handle, system_table);
	pr_info("Initialized efistub driver\n", 0);
	//efistub_init();

	/* cmdline */
	const efi_loaded_image_protocol_t * image = efistub_image_proto();
	if (image == NULL) {
		pr_crit("Failed to get loaded image handle\n", 0);
		pr_crit("Cannot get cmdline\n", 0);
	} else {
		pr_debug("Kernel loaded at %p\n", image->image_base);
		//const char * cmdline = get_cmdline(image);
		const char * cmdline = efistub_cmdline();
		if (cmdline == NULL)
			pr_crit("Failed to get cmdline\n", 0);
		else
			pr_debug("Booted with cmdline: %s\n", cmdline);
	}

	/* fb */
	//fb_init();
	pr_info("Initialized framebuffer\n", 0);

	/* efistub: exit boot services */
	//struct memmap map;
	if ((ret = efistub_memmap_and_exit())) {
		pr_emerg("Cannot get memory map or exit boot services "
			 "(error %d)\n", ret);
		halt();
	}
	pr_info("Exited boot services\n", 0);

	/* kernel: load IDT */
	//idt_init();

	/* mm: load memmap */
	//print_memmap(map);
	//mm_init(map);
	// TODO
	pr_err("causing div zero:\n", 0);
	int a = 12;
	int b = 0;
	int c = a/b;
	pr_err("this is here %d\n", c);

	halt();
}

noreturn static void halt(void)
{
	pr_emerg("Kernel halting\n", 0);
	while (1) {
		asm(intel(
			"cli\n"
			"hlt\n"
		));
	}
}
