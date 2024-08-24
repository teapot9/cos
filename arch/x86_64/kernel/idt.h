#ifndef _X86_KERNEL_IDT_H
#define _X86_KERNEL_IDT_H

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#include <asm/asm.h>
#include <cpp.h>

enum idt_gate_type {
	IDT_TASK_GATE = 0x5,
	IDT_16INT_GATE = 0x6,
	INT_16TRAP_GATE = 0x7,
	INT_INT_GATE = 0xE,
	INT_TRAP_GATE = 0xF,
};

struct _packed_ idt_desc {
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

struct _packed_ idtr {
	uint16_t limit;
	struct idt_desc * idt;
};
static_assert(sizeof(struct idtr) == 10, "idtr must be 10 bytes");

static inline void lidt(struct idtr * idt)
{
	asm volatile(intel("lidt [rax]\n") : : "a" (idt));
}

static inline struct idtr sidt(void)
{
	struct idtr idtr;
	asm volatile(intel("sidt [rax]\n") : "=a" (idtr));
	return idtr;
}

int idt_create_load(void);

#endif // _X86_KERNEL_IDT_H
