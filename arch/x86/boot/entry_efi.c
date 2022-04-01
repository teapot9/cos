#include <errno.h>
#include <stddef.h>
#include <stdnoreturn.h>

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

noreturn void entry_efi(efi_handle_t image_handle,
                        efi_system_table_t * system_table);

extern noreturn void entry_efi_wrapper(
	efi_handle_t image_handle, efi_system_table_t * system_table
);
extern noreturn void set_kstack(void * kstack, void (* fcn)(void));

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

noreturn void entry_efi(efi_handle_t image_handle,
                        efi_system_table_t * system_table)
{
	int ret = 0;

#ifdef CONFIG_DEBUG
	debug_base_addr(image_handle, system_table);
	//bool tmp = false;
	//while (!tmp);
#endif

	efistub_init(image_handle, system_table);
	pr_info("Early init: EFI stub\n", 0);

	efistub_console_init();
	pr_info("Early init: EFI console\n", 0);

	kernel_cmdline = efistub_cmdline();
	if (kernel_cmdline == NULL)
		pr_crit("Failed to get cmdline\n", 0);
	pr_info("Early init: cmdline: %s\n", kernel_cmdline);

	gop_init();
	pr_info("Early init: EFI GOP\n", 0);

	if ((ret = efistub_memmap_and_exit())) {
		panic("Cannot get memory map or exit boot services"
		      ", errno = %d\n", ret);
	}
	pr_info("Early init: exited boot services\n", 0);

#ifdef CONFIG_SERIAL_EARLY_DEBUG
	serial_init();
#endif

	pmm_init();
	pr_info("Early init: PMM\n", 0);

	vmm_init();
	pr_info("Early init: VMM\n", 0);

	int main_pid = process_new(NULL, kernel_main);
	if (main_pid > 0)
		main_pid = -EINVAL;
	if (main_pid)
		panic("Failed to create kernel process, errno = %d",
		      main_pid);

	ret = cpu_reg(NULL);
	if (ret)
		panic("Failed CPU0 initialization, errno = %d", ret);
	pr_info("Early init: CPU0\n", 0);

	cpu_start(0, 0);
}
