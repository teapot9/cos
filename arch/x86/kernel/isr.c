#include "isr.h"

#include <stdbool.h>
#include <errno.h>

#include <asm/cpu.h>
#include <mm.h>
#include <panic.h>
#include <print.h>
#include <debug.h>
#include <asm/asm.h>
#include <irq.h>
#include <cpp.h>
#include <gdt.h>
#include <cpu.h>

static struct isr * isrs[ISR_MAX + 1] = {NULL};

static inline void dobreak(void)
{
	bool cont = false;
	while (!cont);
}

static const char * str_int(uword_t i)
{
	switch (i) {
	case EXC_DIV_ZERO:
		return "div zero exception";
	case EXC_DEBUG:
		return "debug exception";
	case EXC_NMI:
		return "nmi exception";
	case EXC_BREAKPOINT:
		return "breakpoint exception";
	case EXC_OVERFLOW:
		return "overflow exception";
	case EXC_BOUND_RANGE_EXCEEDED:
		return "bound range exceeded exception";
	case EXC_INVALID_OPCODE:
		return "invalid opcode exception";
	case EXC_DEV_NOT_AVAILABLE:
		return "dev not available exception";
	case EXC_DOUBLE_FAULT:
		return "double fault exception";
	case EXC_INVALID_TSS:
		return "invalid tss exception";
	case EXC_SEGMENT_NOT_PRESENT:
		return "segment not present exception";
	case EXC_STACK_SEGMENT_FAULT:
		return "stack segment fault exception";
	case EXC_GPF:
		return "gpf exception";
	case EXC_PF:
		return "pf exception";
	case EXC_FPE:
		return "fpe exception";
	case EXC_ALIGNMENT_CHECK:
		return "alignment check exception";
	case EXC_MACHINE_CHECK:
		return "machine check exception";
	case EXC_SIMD_FPE:
		return "simd fpe exception";
	case EXC_VIRTUALIZATION_EXCEPTION:
		return "virtualization exception exception";
	case EXC_CONTROL_PROTECTION_EXCEPTION:
		return "control protection exception exception";
	case EXC_HYPERVISOR_INJECTION_EXCEPTION:
		return "hypervisor injection exception exception";
	case EXC_VMM_COMMUNICATION_EXCEPTION:
		return "vmm communication exception exception";
	case EXC_SECURITY_EXCEPTION:
		return "security exception exception";
	case IRQ_PIT:
		return "pit IRQ";
	case IRQ_KBD:
		return "kbd IRQ";
	case IRQ_COM2:
		return "com2 IRQ";
	case IRQ_COM1:
		return "com1 IRQ";
	case IRQ_LPT2:
		return "lpt2 IRQ";
	case IRQ_FLOPPY:
		return "floppy IRQ";
	case IRQ_LPT1:
		return "lpt1 IRQ";
	case IRQ_CMOS:
		return "cmos IRQ";
	case IRQ_PS2:
		return "ps2 IRQ";
	case IRQ_FPU:
		return "fpu IRQ";
	case IRQ_ATA1:
		return "ata1 IRQ";
	case IRQ_ATA2:
		return "ata2 IRQ";
	default:
		return "unknown interrupt";
	}
}

void isr_handler(struct interrupt_frame * frame)
{
	static int depth = 0;
	depth++;
	cpu_set_state(cpu_current(), frame);
	//interrupt_start(); // TODO: remove this and support nested int
			   // PIT callback should not do callback if callback
			   // already in progress
	disable_interrupts();

	if (frame->interrupt > ISR_MAX) {
		pr_err("unexpected interrupt number: %u\n", frame->interrupt);
		return;
	}

	switch (frame->interrupt) {
	case EXC_DEBUG:
		pr_debug("Debug interrupt\n", 0);
		dobreak();
		break;
	case EXC_BREAKPOINT:
		pr_debug("Breakpoint\n", 0);
		dobreak();
		break;
	default:
		if (is_exc(frame->interrupt))
			panic("unhandled exception: %s",
			      str_int(frame->interrupt));
	}

	if (!is_exc(frame->interrupt))
		restore_interrupts();

	struct isr * cur = isrs[frame->interrupt];
	while (cur != NULL) {
		if (cur->isr == frame->interrupt)
			cur->callback();
		cur = cur->next;
	}

	if (is_irq(frame->interrupt))
		irq_eoi(isr_to_irq(frame->interrupt));

	if (is_exc(frame->interrupt))
		restore_interrupts();
	//interrupt_end();
	depth--;
}

int isr_reg(unsigned isr, void (* callback)(void))
{
	if (isr > ISR_MAX || callback == NULL)
		return -EINVAL;

	struct isr * new = kmalloc(sizeof(*new));
	if (new == NULL)
		return -ENOMEM;

	new->isr = isr;
	new->callback = callback;
	new->next = isrs[isr];
	isrs[isr] = new;
	return 0;
}

int isr_unreg(unsigned isr, void (* callback)(void))
{
	if (isr > ISR_MAX || callback == NULL)
		return -EINVAL;

	struct isr ** cur = &isrs[isr];
	while (*cur != NULL && (*cur)->isr != isr
	       && (*cur)->callback != callback)
		cur = &(*cur)->next;
	if (*cur == NULL)
		return -ENOENT;

	struct isr * tmp = *cur;
	*cur = tmp->next;
	kfree(tmp);
	return 0;
}

void maybe_swap_gs(struct interrupt_frame * frame)
{
	if (frame->cs != gdt_segment(GDT_KERN_CS))
		swapgs();
}
