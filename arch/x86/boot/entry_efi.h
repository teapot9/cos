#ifndef BOOT_ENTRY_EFI_H
#define BOOT_ENTRY_EFI_H

#include <stddef.h>

struct entry_efi_data {
	struct efistub_data * efistub;
	const char * cmdline;
	struct efigop_bdata * gop;
	struct memmap * pmemmap;
	//struct memlist * bootmem;
	void * kernel_elf;
	size_t kernel_elf_size;
};

#endif // BOOT_ENTRY_EFI_H
