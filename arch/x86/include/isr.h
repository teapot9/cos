#ifndef ISR_H
#define ISR_H

#include <stdnoreturn.h>

#include "../kernel/isr_wrapper.h"
#include <cpp.h>
#include <kconfig.h>

#if IS_ENABLED(CONFIG_X86_64)
typedef unsigned long long int uword_t;
# define UWORD_PRINT "ll"
#elif IS_ENABLED(CONFIG_X86_32)
typedef unsigned int uword_t;
# define UWORD_PRINT ""
#else
# error Unknown architecture type
#endif

struct PACKED interrupt_frame {
	uword_t ds;
	uword_t es;
	uword_t fs;
	uword_t gs;
	uword_t r15;
	uword_t r14;
	uword_t r13;
	uword_t r12;
	uword_t r11;
	uword_t r10;
	uword_t r9;
	uword_t r8;
	uword_t bp;
	uword_t di;
	uword_t si;
	uword_t dx;
	uword_t cx;
	uword_t bx;
	uword_t ax;
	uword_t interrupt;
	uword_t error;
	uword_t ip;
	uword_t cs;
	uword_t flags;
	uword_t sp;
	uword_t ss;
};

noreturn void jmp_to_frame(struct interrupt_frame * frame);
int isr_reg(unsigned isr, void (* callback)(void));
int isr_unreg(unsigned isr, void (* callback)(void));

#endif // ISR_H
