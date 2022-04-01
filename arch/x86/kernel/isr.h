#ifndef ARCH_X86_KERNEL_ISR_H
#define ARCH_X86_KERNEL_ISR_H

#include "isr_wrapper.h"

#include <isr.h>

struct PACKED cpu_interrupt_frame {
	uword_t ip;
	uword_t cs;
	uword_t flags;
	uword_t sp;
	uword_t ss;
};

struct isr {
	void (* callback)(struct interrupt_frame * frame);
	struct isr * next;
};

void isr_handler(struct interrupt_frame * frame);
void maybe_swap_gs(struct interrupt_frame * frame);

#endif // ARCH_X86_KERNEL_ISR_H
