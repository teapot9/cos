#include "isr_wrapper.h"
#define isr(name, id) \
	.globl name ;\
	name: ;\
		push 0 ;\
		push id ;\
		jmp isr_wrapper

#define isr_err(name, id) \
	.globl name ;\
	name: ;\
		push id ;\
		jmp isr_wrapper

.intel_syntax noprefix
.globl isr_handler
.globl jmp_to_frame

.text

isr_wrapper:
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	xor rdi, rdi
	mov di, gs
	push rdi
	mov di, fs
	push rdi
	mov di, es
	push rdi
	mov di, ds
	push rdi

	// pushed 26 * 8 bytes: 16-byte aligned
	mov rdi, rsp
	call isr_handler

do_iret:
	pop rdi
	mov ds, di
	pop rdi
	mov es, di
	pop rdi
	mov fs, di
	pop rdi
	mov gs, di
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	add rsp, 16 // interrupt number and error code
	iretq

jmp_to_frame:
	//sub rdi, 8 // next pop will get first interrupt_frame item
	mov rsp, rdi
	jmp do_iret

isr(isr0, EXC_DIV_ZERO)
isr(isr1, EXC_DEBUG)
isr(isr2, EXC_NMI)
isr(isr3, EXC_BREAKPOINT)
isr(isr4, EXC_OVERFLOW)
isr(isr5, EXC_BOUND_RANGE_EXCEEDED)
isr(isr6, EXC_INVALID_OPCODE)
isr(isr7, EXC_DEV_NOT_AVAILABLE)
isr_err(isr8, EXC_DOUBLE_FAULT)
isr_err(isr10, EXC_INVALID_TSS)
isr_err(isr11, EXC_SEGMENT_NOT_PRESENT)
isr_err(isr12, EXC_STACK_SEGMENT_FAULT)
isr_err(isr13, EXC_GPF)
isr_err(isr14, EXC_PF)
isr(isr16, EXC_FPE)
isr_err(isr17, EXC_ALIGNMENT_CHECK)
isr(isr18, EXC_MACHINE_CHECK)
isr(isr19, EXC_SIMD_FPE)
isr(isr20, EXC_VIRTUALIZATION_EXCEPTION)
isr_err(isr21, EXC_CONTROL_PROTECTION_EXCEPTION)
isr(isr28, EXC_HYPERVISOR_INJECTION_EXCEPTION)
isr_err(isr29, EXC_VMM_COMMUNICATION_EXCEPTION)
isr_err(isr30, EXC_SECURITY_EXCEPTION)
isr(isr32, IRQ_PIT)
isr(isr33, IRQ_KBD)
isr(isr35, IRQ_COM2)
isr(isr36, IRQ_COM1)
isr(isr37, IRQ_LPT2)
isr(isr38, IRQ_FLOPPY)
isr(isr39, IRQ_LPT1)
isr(isr40, IRQ_CMOS)
isr(isr44, IRQ_PS2)
isr(isr45, IRQ_FPU)
isr(isr46, IRQ_ATA1)
isr(isr47, IRQ_ATA2)
