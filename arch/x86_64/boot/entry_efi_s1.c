#include "entry_efi.h"

#include <errno.h>
#include <stddef.h>
#include <stdnoreturn.h>

#include <asm/cpu.h>
#include <list.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <mm/pmm.h>
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
#include <alloc.h>
#include <mm/vmm.h>
#include <printk.h>
#include <printf.h>
#include <debug.h>
#include <string.h>
#include <unicode.h>
#include <video/fb.h>
#include <asm/io.h>
#include <platform_setup.h>
#include <setup.h>
#include <cmdline.h>
#include <panic.h>
#include <setup.h>
#include <elf.h>
#include <memlist.h>
#include <kconfig.h>

#define __kernel_symbol(suffix, k) _binary_ ## k ## _elf ## suffix
#define _kernel_symbol(suffix, k) __kernel_symbol(suffix, k)
#define kernel_symbol(suffix) _kernel_symbol(suffix, KERNEL_NAME)

struct entry_efi_data boot_data;
extern uint8_t kernel_symbol(_start)[];
extern uint8_t kernel_symbol(_end)[];
static struct memlist bootloader_mem;

extern noreturn void entry_efi_wrapper(
	efi_handle_t image_handle, efi_system_table_t * system_table
);

#if IS_ENABLED(CONFIG_DEBUG)
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

noreturn static void trampoline(void (* fcn)(struct entry_efi_data *))
{
	fcn(&boot_data);
	panic("second stage boot should not return");
}

noreturn void entry_efi_s1(efi_handle_t image_handle,
                           efi_system_table_t * system_table)
{
	int err = 0;
	kmm_init();

#if IS_ENABLED(CONFIG_DEBUG)
	debug_base_addr(image_handle, system_table);
#endif
#if IS_ENABLED(BOOT_BREAKPOINT_BOOTLOADER)
	kbreak();
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
	err = cmdline_init(efistub_firmware_cmdline);
	boot_data.cmdline = kernel_cmdline;
	if (err)
		panic("failed to get cmdline, errno = %d", err);
	pr_info("Early init: cmdline: %s\n", kernel_cmdline);

#if IS_BUILTIN(CONFIG_FB_EFIGOP)
	/* EFI GOP */
	gop_init(&boot_data.gop);
	pr_info("Early init: EFI GOP\n", 0);
#endif

	/* memmap & EFI exit */
	struct memmap map = memmap_new();
	if ((err = efistub_memmap_and_exit(&map))) {
		panic("Cannot get memory map or exit boot services"
		      ", errno = %d\n", err);
	}
	pr_info("Early init: exited boot services\n", 0);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	memmap_dump(&map, "pmemmap");
#endif

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
	/* Serial debug */
	serial_init();
#endif

	/* pmm */
	err = pmm_init(&map);
	if (err)
		panic("failed to initialize pmm, errno = %d", err);
	memmap_free(&map, false);

	/* vmm */
	err = vmm_init();
	if (err)
		panic("failed to initialzie vmm, errno = %d", err);
	vmm_enable_paging();

	/* Load kernel ELF */
	boot_data.kernel_elf = kernel_symbol(_start);
	boot_data.kernel_elf_size = kernel_symbol(_end) - kernel_symbol(_start);
	void (* kentry)(void);
	err = elf64_load(0, &kentry, boot_data.kernel_elf,
	                 boot_data.kernel_elf_size);
	if (err)
		panic("failed to load kernel ELF executable, errno = %d", err);

	/* Save physical memory info */
	boot_data.pmemmap = &memmap;

#if 0
	kpmemmap = memmap_new();
	struct memmap_elt * pcur;
	list_foreach(pcur, memmap.l.l.first) {
		enum memory_type type = pcur->type == MEMORY_TYPE_USED
			? MEMORY_TYPE_FREE : pcur->type;
		err = memmap_update(&kpmemmap, pcur->l.addr,
		                    pcur->l.size, type, 0);
		if (err)
			panic("cannot save physical memory map, errno = %d",
			      err);
	}
	struct vmemmap_elt * vcur;
	list_foreach(vcur, kvmemmap.l.l.first) {
		err = memmap_update(&kpmemmap, vcur->map,
		                    vcur->l.size, MEMORY_TYPE_USED, 0);
		if (err)
			panic("cannot save physical memory map, errno = %d",
			      err);
	}
	boot_data.pmemmap = &kpmemmap;
	memmap_dump(boot_data.pmemmap, "new pmemmap");

	/* Save bootloader memory */
	bootloader_mem = memlist_new_default();
	struct memmap_elt * mcur;
	list_foreach (mcur, memmap.l.l.first) {
		if (mcur->type == MEMORY_TYPE_USED) {
			err = memlist_add(&bootloader_mem, mcur->l.addr,
			                  mcur->l.size, false);
			if (err)
				panic("cannot register bootloader mem at "
				      "%p (%zu bytes), errno = %d",
				      mcur->l.addr, mcur->l.size, err);
		}
	}
	struct vmemmap_elt * vcur;
	list_foreach (vcur, kvmemmap.l.l.first) {
		err = memlist_del(&bootloader_mem, vcur->map,
		                  vcur->l.size, false);
		if (err)
			panic("cannot mark kernel memory as needed after boot"
			      "at %p (%zu bytes), errno = %d",
			      vcur->map, vcur->l.size, err);
	}
	boot_data.bootmem = &bootloader_mem;
#endif

	trampoline((void (*)(struct entry_efi_data *)) kentry);
}
