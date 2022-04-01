.intel_syntax noprefix
.globl entry_efi_wrapper
.globl set_kstack

.text

entry_efi_wrapper:
	mov rdi, rcx
	mov rsi, rdx
	lea rax, [rip + boot_kstack + CONFIG_KERNEL_FRAME_SIZE]
	mov rsp, rax
	jmp entry_efi

set_kstack:
	mov rsp, rdi
	jmp rsi

.bss

.lcomm boot_kstack, CONFIG_KERNEL_FRAME_SIZE
