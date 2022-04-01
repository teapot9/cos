#ifndef BOOT_ENTRY_EFI_H
#define BOOT_ENTRY_EFI_H

struct entry_efi_data {
	struct efistub_bdata * efistub;
	const char * cmdline;
	struct efigop_bdata * gop;
};

#endif // BOOT_ENTRY_EFI_H
