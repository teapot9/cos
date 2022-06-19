#ifndef _X86_KERNEL_ISR_H
#define _X86_KERNEL_ISR_H

#include "isr_wrapper.h"

#include <isr.h>

struct _packed_ cpu_interrupt_frame {
	uword_t ip;
	uword_t cs;
	uword_t flags;
	uword_t sp;
	uword_t ss;
};

struct isr {
	unsigned isr;
	void (* callback)(void);
	struct isr * next;
};

void isr_handler(struct interrupt_frame * frame);
void maybe_swap_gs(struct interrupt_frame * frame);

#endif // _X86_KERNEL_ISR_H
