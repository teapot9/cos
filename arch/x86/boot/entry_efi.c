#include <stddef.h>
#include <stdnoreturn.h>

#include <asm.h>
#include <console.h>
#include <firmware/efistub.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/boot.h>
#include <firmware/efiapi/loaded_image.h>
#include <firmware/efiapi/system_table.h>
#include <fonts.h>
#include <mm.h>
#include <print.h>
#include <debug.h>
#include <string.h>
#include <unicode.h>
#include <video/fb.h>
#include <asm/io.h>
#include <asm/traps.h>

#include "../../../mm/pmm.h"

noreturn EFIABI void entry_efi(efi_handle_t image_handle,
               efi_system_table_t * system_table);

noreturn static void halt(void);

#ifdef CONFIG_EXTRA_CMDLINE
const char * const extra_cmdline = CONFIG_EXTRA_CMDLINE;
#endif

efi_loaded_image_protocol_t * get_image(efi_handle_t handle)
{
	efi_status_t status;
	efi_guid_t image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	efi_loaded_image_protocol_t * image = NULL;

	status = efistub_system_table->boot_services->handle_protocol(
		handle, &image_guid, (void *) &image
	);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to get EFI loaded image protocol handle "
		       "(error %d)\n", status);
		return NULL;
	}
	return image;
}

/* Return EFI cmdline in a dynamically allocated buffer */
char * get_efi_cmdline(efi_loaded_image_protocol_t * image)
{
	char * cmdline = NULL;
	size_t size;

	if (image->load_options_size == 0 || image->load_options == NULL)
		return NULL;

	size = image->load_options_size * 2;
	cmdline = kmalloc(size);
	if (cmdline == NULL) {
		pr_err("Failed to allocate %zu bytes for the EFI "
		       "cmdline buffer", size);
		kfree(cmdline);
		return NULL;
	}

	if (utf16_to_utf(cmdline, image->load_options, size) == 0) {
		pr_err("Failed to convert UTF-16 to UTF-8 UEFI cmdline\n", 0);
		kfree(cmdline);
		return NULL;
	}

	return cmdline;
}

/* Return cmdline in a dynamically allocated buffer */
char * get_cmdline(efi_handle_t handle)
{
	size_t size = 0;
	size_t start = 0;
	char * efi_cmdline = get_efi_cmdline(handle);

	if (efi_cmdline != NULL)
		size += strlen(efi_cmdline);
#ifdef CONFIG_EXTRA_CMDLINE
	if (size)
		size += sizeof(char); // space
	size += strlen(efi_cmdline);
#endif

	char * cmdline = kmalloc(size);
	if (cmdline == NULL) {
		pr_err("Failed to allocate %zu bytes for cmdline\n", 0);
		kfree(efi_cmdline);
		return NULL;
	}

#ifdef CONFIG_EXTRA_CMDLINE
#ifndef CONFIG_EXTRA_CMDLINE_OVERRIDE
	if (*extra_cmdline) {
		strncpy(cmdline + start, extra_cmdline, size - start);
		start = strlen(cmdline);
		cmdline[start] = ' ';
		cmdline[++start] = 0;
	}
#endif
#endif
	if (efi_cmdline != NULL) {
		strncpy(cmdline + start, efi_cmdline, size - start);
		start = strlen(cmdline);
	}
#ifdef CONFIG_EXTRA_CMDLINE
#ifdef CONFIG_EXTRA_CMDLINE_OVERRIDE
	if (efi_cmdline != NULL && *efi_cmdline) {
		cmdline[start] = ' ';
		cmdline[++start] = 0;
	}
	strncpy(cmdline + start, extra_cmdline, size - start);
	start = strlen(cmdline);
#endif
#endif

	kfree(efi_cmdline);
	return cmdline;
}

typedef int initcall_entry_t;
typedef int (*initcall_t)(void);
//typedef int initcall_entry_t;
//typedef void (*initcall_t)(void);
extern initcall_entry_t __initcalltest[];
static inline void *offset_to_ptr(const int *off)
{
	return (void *)((unsigned long)off + *off);
}
static inline initcall_t initcall_from_entry(initcall_entry_t *entry)
{
	return offset_to_ptr(entry);
}
/*
static inline void *offset_to_ptr(const int *off)
{ return (void *)((unsigned long)off + *off); }
static __attribute__((__section__(".init.text"))) void foo(void)
{
}
*/
int myfb(void);
static void hang(void){while(1);}

EFIABI void entry_efi(efi_handle_t image_handle,
                      efi_system_table_t * system_table)
{
	int ret;

	pr_info("Kernel booted from EFI\n", 0);

	/* mm */
	mm_init_early();
	pr_info("Initialized memory manager\n", 0);

	/* efistub */
	efistub_image_handle = image_handle;
	efistub_system_table = system_table;
	efistub_init();
	pr_info("Initialized efistub driver\n", 0);

	/* cmdline */
	efi_loaded_image_protocol_t * image = get_image(image_handle);
	if (image == NULL) {
		pr_crit("Failed to get loaded image handle\n", 0);
		pr_crit("Cannot get cmdline\n", 0);
	} else {
		pr_debug("Kernel loaded at %p\n", image->image_base);
		const char * cmdline = get_cmdline(image);
		if (cmdline == NULL)
			pr_crit("Failed to get cmdline\n", 0);
		else
			pr_debug("Booted with cmdline: %s\n", cmdline);
	}

	hang();
	//initcall_t my_local_call = initcall_from_entry(__initcalltest[0]);
	//initcall_t my_local_call = 0;
	initcall_t my_local_call2 = initcall_from_entry(__initcalltest);
	hang();
	pr_debug("testing: %p, %p, %p, %p\n", __initcalltest, *__initcalltest, my_local_call2, myfb);
	my_local_call2();
	hang();
	//pr_debug("testing: %p, %p == %p\n", *__initcalltest, my_local_call2, myfb);

	/* fb */
	fb_init();
	pr_info("Initialized framebuffer\n", 0);

	/* efistub: exit boot services */
	struct memmap map;
	if ((ret = efistub_memmap_and_exit(&map))) {
		pr_emerg("Cannot get memory map or exit boot services "
			 "(error %d)\n", ret);
		halt();
	}
	pr_info("Exited boot services\n", 0);

	/* kernel: load IDT */
	idt_init();

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

/*
void mystub(void);
void mystub(void) { foo(); }
asm(".section \".initcalltest\", \"a\"\n"
    "myname:\n"
    ".long mystub - .\n"
    ".previous\n");
    */
//static initcall_t myname __attribute__((__used__)) __attribute__((__section__(".initcalltest"))) = (void *)&foo;
