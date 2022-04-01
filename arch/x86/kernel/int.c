#include <int.h>
#include "int.h"

#include <stdbool.h>

#include <panic.h>
#include "idt.h"
#include <print.h>
#include <debug.h>

static __attribute__((interrupt)) void
exc_divide_by_zero(struct interrupt_frame * frame)
{
	panic("Exception: divide-by-zero error\n", 0);
}

static __attribute__((interrupt)) void
exc_debug(struct interrupt_frame * frame)
{
	pr_debug("Exception: debug\n", 0);
	kbreak();
}

static __attribute__((interrupt)) void
exc_nmi(struct interrupt_frame * frame)
{
	pr_info("Exception: non-maskable interrupt\n", 0);
}

static __attribute__((interrupt)) void
exc_breakpoint(struct interrupt_frame * frame)
{
	pr_debug("Exception: breakpoint\n", 0);
	kbreak();
}

static __attribute__((interrupt)) void
exc_invalid_opcode(struct interrupt_frame * frame)
{
	panic("Exception: invalid opcode\n", 0);
}

static __attribute__((interrupt)) void
exc_double_fault(struct interrupt_frame * frame, uword_t error_code)
{
	panic("Exception: double fault: " UWORD_PRINT "\n", error_code);
}

#define get_bit(data, bit) ((data) & ((1 << (bit))) >> (bit))

static inline uint64_t read_cr2(void)
{
	uint64_t ret;
	asm volatile(intel("mov rax, cr2\n") : "=a" (ret));
	return ret;
}

static __attribute__((interrupt)) void
exc_page_fault(struct interrupt_frame * frame, uword_t error_code)
{
	struct page_fault_code * err = (void *) &error_code;
	uint64_t cr2 = read_cr2();
#if 0
	pr_emerg("Exception: page fault:"
		 "p=%d w=%d u=%d r=%d i=%d pk=%d ss=%d sgx=%d\n",
		 err->present, err->write, err->user, err->reserved_write,
		 err->instruction_fetch, err->protection_key,
		 err->shadow_stack, err->software_guard_extensions);
	panic("Page fault caused by %p\n", cr2);
#endif
}


void int_init(void)
{
	set_idt(0, exc_divide_by_zero);
	set_idt(1, exc_debug);
	set_idt(2, exc_nmi);
	set_idt(3, exc_breakpoint);
	set_idt(6, exc_invalid_opcode);
	set_idt(8, exc_double_fault);
	set_idt(14, exc_page_fault);
}
