#ifndef ARCH_X86_KERNEL_ISR_WRAPPER_H
#define ARCH_X86_KERNEL_ISR_WRAPPER_H

#define ISR_MAX 0x30

#define EXC_DIV_ZERO 0x0
#define EXC_DEBUG 0x1
#define EXC_NMI 0x2
#define EXC_BREAKPOINT 0x3
#define EXC_OVERFLOW 0x4
#define EXC_BOUND_RANGE_EXCEEDED 0x5
#define EXC_INVALID_OPCODE 0x6
#define EXC_DEV_NOT_AVAILABLE 0x7
#define EXC_DOUBLE_FAULT 0x8
#define EXC_INVALID_TSS 0xA
#define EXC_SEGMENT_NOT_PRESENT 0xB
#define EXC_STACK_SEGMENT_FAULT 0xC
#define EXC_GPF 0xD
#define EXC_PF 0xE
#define EXC_FPE 0x10
#define EXC_ALIGNMENT_CHECK 0x11
#define EXC_MACHINE_CHECK 0x12
#define EXC_SIMD_FPE 0x13
#define EXC_VIRTUALIZATION_EXCEPTION 0x14
#define EXC_CONTROL_PROTECTION_EXCEPTION 0x15
#define EXC_HYPERVISOR_INJECTION_EXCEPTION 0x1C
#define EXC_VMM_COMMUNICATION_EXCEPTION 0x1D
#define EXC_SECURITY_EXCEPTION 0x1E

#define IRQ_PIT 0x20
#define IRQ_KBD 0x21
#define IRQ_COM2 0x23
#define IRQ_COM1 0x24
#define IRQ_LPT2 0x25
#define IRQ_FLOPPY 0x26
#define IRQ_LPT1 0x27
#define IRQ_CMOS 0x28
#define IRQ_PS2 0x2C
#define IRQ_FPU 0x2D
#define IRQ_ATA1 0x2E
#define IRQ_ATA2 0x2F

#define ISR_YIELD 0x30

#define is_exc(i) ((i) < 0x20)
#define is_pic1(i) ((i) >= 0x20 && (i) < 0x28)
#define is_pic2(i) ((i) >= 0x28 && (i) < 0x30)
#define is_irq(i) (is_pic1(i) || is_pic2(i))
#define isr_to_irq(i) ((i) - 0x20)
#define irq_to_isr(i) ((i) + 0x20)

#endif // ARCH_X86_KERNEL_ISR_WRAPPER_H
