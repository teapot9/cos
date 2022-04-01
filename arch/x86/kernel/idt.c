#include <int.h>
#include "idt.h"

#include <stddef.h>

#include "int.h"
#include <print.h>
#include <asm/asm.h>

#define IDT_SIZE 16

static struct idt_desc idt[IDT_SIZE];
static struct idt_reg idtr;

static __attribute__((interrupt)) void int_handler(
	struct interrupt_frame * frame
)
{
	pr_notice("Got interrupt\n", 0);
}

static inline void lidt(void * idt)
{
	asm volatile(intel("lidt [rax]\n") : : "a" (idt));
}

static inline uint64_t sidt(void)
{
	uint64_t ret;
	asm volatile(intel("sidt [rax]\n") : "=a" (ret));
	return ret;
}

static inline uint16_t read_cs(void)
{
	uint16_t ret;
	asm volatile(intel("mov ax, cs\n") : "=a" (ret));
	return ret;
}

void set_idt(size_t index, void * callback)
{
	if (index >= IDT_SIZE) {
		pr_alert("Index %zu too big for IDT size %zu", index, IDT_SIZE);
		return;
	}

	uint16_t cs = read_cs();
	struct idt_desc desc = {
		.offset_low = (uint64_t) callback & 0xFFFF,
		.segment = cs,
		.ist = 0,
		.zero1 = 0,
		.gate_type = INT_INT_GATE,
		.privilege_level = 0,
		.present = callback != NULL ? 1 : 0,
		.offset_high = (uint64_t) callback >> 16,
		.zero2 = 0,
	};
	idt[index] = desc;
}

int idt_init(void)
{
	uint64_t old_idt = sidt();

	for (size_t i = 0; i < IDT_SIZE; i++)
		set_idt(i, NULL);
	idtr.limit = IDT_SIZE * sizeof(*idt);
	idtr.base = (uint64_t) idt;
	lidt(&idtr);

	int_init();

	return 0;
}

