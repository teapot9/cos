.intel_syntax noprefix

#include <kconfig.h>

.text

entry_efi_wrapper:
	lea rax, [rip + boot_kstack + CONFIG_KERNEL_FRAME_SIZE]
	mov rsp, rax
	mov rbp, 0
	push rbp
	push rbp
	jmp rdx

set_kstack:
	mov rsp, rdi
	jmp rsi

#ifdef BOOT
.globl entry_efi_wrapper_s1
entry_efi_wrapper_s1:
	mov rdi, rcx
	mov rsi, rdx
	lea rdx, [rip + entry_efi_s1]
	jmp entry_efi_wrapper
#else
.globl entry_efi_wrapper_s2
entry_efi_wrapper_s2:
	lea rdx, [rip + entry_efi_s2]
	jmp entry_efi_wrapper
#endif

.bss

.lcomm boot_kstack, CONFIG_KERNEL_FRAME_SIZE
