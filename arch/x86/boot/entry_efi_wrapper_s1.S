.intel_syntax noprefix
.globl entry_efi_wrapper_s1

.text

entry_efi_wrapper_s1:
	mov rdi, rcx
	mov rsi, rdx
	lea rdx, [rip + entry_efi_s1]
	jmp entry_efi_wrapper
