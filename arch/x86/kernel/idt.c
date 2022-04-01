#include <asm/traps.h>
#include "idt.h"

#include <stddef.h>

#include <print.h>
#include "int_wrapper.h"

#define IDT_SIZE 14

static struct idt_desc idt[IDT_SIZE];
static struct idt_reg idtr;

__attribute__((interrupt)) void int_handler(void* i)
{
	pr_notice("Got interrupt\n", 0);
}

void lidt(void * idt)
{
	asm volatile(intel("lidt [rax]\n") : : "a" (idt));
}

void set_idt_desc(struct idt_desc * desc, void * callback)
{
}

void idt_init(void)
{
	uint64_t tmp = 0;
	asm volatile (intel("sidt [rax]\n") : "=a" (tmp));
	struct idt_reg * cidtr = (void *) tmp;
	struct idt_desc * cidt = (void *) (cidtr->base);
	uint16_t cs;
	asm volatile (intel("mov ax, cs\n") : "=a" (cs));
	struct idt_desc std_idt = {
		.offset_low = (uint64_t) int_handler & 0xFFFF,
		.segment = cs,
		.ist = 0,
		.zero1 = 0,
		.gate_type = INT_INT_GATE,
		.privilege_level = 0,
		.present = 1,
		.offset_high = (uint64_t) int_handler >> 16,
		.zero2 = 0,
	};
	for (size_t i = 0; i < IDT_SIZE; i++)
		idt[i] = std_idt;
	idtr.limit = IDT_SIZE * sizeof(*idt);
	idtr.base = (uint64_t) idt;
	lidt(&idtr);
	int a = 15;
	int b = 0;
	tmp = a/b;
	//asm volatile (intel("lidt [rax]\n") : : "a" (&idtr));
}

