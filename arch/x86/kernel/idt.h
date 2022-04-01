#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include <asm/asm.h>
#include <cpp.h>

#ifdef CONFIG_X86_64
typedef unsigned long long int uword_t;
# define UWORD_PRINT "ll"
#elif CONFIG_X86_32
typedef unsigned int uword_t;
# define UWORD_PRINT ""
#else
# error Unknown architecture type
#endif

enum idt_gate_type {
	IDT_TASK_GATE = 0x5,
	IDT_16INT_GATE = 0x6,
	INT_16TRAP_GATE = 0x7,
	INT_INT_GATE = 0xE,
	INT_TRAP_GATE = 0xF,
};

struct PACKED idt_reg {
	uint16_t limit;
	uint64_t base;
};

struct PACKED idt_desc {
	long unsigned offset_low : 16;
	uint16_t segment;
	unsigned ist : 3;
	unsigned zero1 : 5;
	unsigned gate_type : 5;
	unsigned privilege_level : 2;
	unsigned present : 1;
	long unsigned offset_high : 48;
	uint32_t zero2;
};
static_assert(sizeof(struct idt_desc) == 16, "idt_desc must be 16 bytes");

struct interrupt_frame {
	uword_t ip;
	uword_t cs;
	uword_t flags;
	uword_t sp;
	uword_t ss;
};

void set_idt(size_t index, void * callback);

#endif // KERNEL_IDT_H
