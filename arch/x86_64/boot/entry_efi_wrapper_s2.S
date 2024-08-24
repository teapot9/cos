.intel_syntax noprefix
.globl entry_efi_wrapper_s2

.text

entry_efi_wrapper_s2:
	lea rdx, [rip + entry_efi_s2]
	jmp entry_efi_wrapper
