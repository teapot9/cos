#ifndef _X86_BOOT_ENTRY_EFI_H
#define _X86_BOOT_ENTRY_EFI_H

#include <stddef.h>

struct entry_efi_data {
	struct efistub_data * efistub;
	const char * cmdline;
	struct gop * gop;
	struct memmap * pmemmap;
	//struct memlist * bootmem;
	void * kernel_elf;
	size_t kernel_elf_size;
};

#endif // _X86_BOOT_ENTRY_EFI_H
