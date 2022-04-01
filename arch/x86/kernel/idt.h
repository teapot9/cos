#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>
#include <assert.h>

#include <asm.h>

enum idt_gate_type {
	IDT_TASK_GATE = 0x5,
	IDT_16INT_GATE = 0x6,
	INT_16TRAP_GATE = 0x7,
	INT_INT_GATE = 0xE,
	INT_TRAP_GATE = 0xF,
};

struct idt_reg {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));
static_assert(sizeof(struct idt_reg) == 10, "idt_reg must be 10 bytes");

struct idt_desc {
	long unsigned offset_low : 16;
	uint16_t segment;
	unsigned ist : 3;
	unsigned zero1 : 5;
	unsigned gate_type : 5;
	unsigned privilege_level : 2;
	unsigned present : 1;
	long unsigned offset_high : 48;
	uint32_t zero2;
} __attribute__((packed));
static_assert(sizeof(struct idt_desc) == 16, "idt_desc must be 16 bytes");

#endif // KERNEL_IDT_H
