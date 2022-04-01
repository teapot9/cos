#ifndef BOOT_ENTRY_EFI_H
#define BOOT_ENTRY_EFI_H

struct entry_efi_data {
	struct efistub_data * efistub;
	const char * cmdline;
	struct efigop_bdata * gop;
	struct vmemmap * vmemmap;
	struct memmap * pmemmap;
	struct memlist * bootmem;
};

#endif // BOOT_ENTRY_EFI_H
