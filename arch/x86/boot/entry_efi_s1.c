#include "entry_efi.h"

#include <errno.h>
#include <stddef.h>
#include <stdnoreturn.h>

#include <asm/cpu.h>
#include <mm/vmemmap.h>
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
#include <memlist.h>

void * malloc(size_t s) { return kmalloc(s); }

struct entry_efi_data boot_data;
extern uint8_t _binary_cos_elf_start[];
extern uint8_t _binary_cos_elf_end[];
static struct memlist bootloader_mem;

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

noreturn static void trampoline(void (* fcn)(struct entry_efi_data *))
{
	fcn(&boot_data);
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
	uint64_t cr4 = read_cr4();
	cr4 &= ~(1 << 7);
	write_cr4(cr4);
	cr4 |= 1 << 7;
	write_cr4(cr4);

	/* pmm */
	err = pmm_init(&map);
	if (err)
		panic("failed to initialize pmm, errno = %d", err);
	memmap_free(&map, false);

	/* vmm */
	err = vmm_init();
	if (err)
		panic("failed to initialzie vmm, errno = %d", err);

	/* Load kernel ELF */
	void (* kentry)(void);
	err = elf_load(&kentry, _binary_cos_elf_start,
	               _binary_cos_elf_end - _binary_cos_elf_end);
	if (err)
		panic("failed to load kernel ELF executable, errno = %d", err);

	/* Save virtual memory info */
	boot_data.vmemmap = &kvmemmap;

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
	memmap_print(boot_data.pmemmap, "new pmemmap");
#endif

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

	//int testi = *(int *) kentry;
	//asm volatile (intel("jmp rax\n") : : "a" (kentry));
	trampoline(kentry);
}
