.intel_syntax noprefix

.section .multiboot

multiboot2:
	.long 0xe85250d6 # Magic
	.long 0 # 32 bit arch

.text

.global entry_grub
entry_grub:
	mov ax, 0xB00B
	jmp .deadloop

.deadloop:
	hlt
	jmp .deadloop
