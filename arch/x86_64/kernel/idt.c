#include "idt.h"
#include <platform_setup.h>

#include <stddef.h>

#include "gdt.h"
#include <printk.h>
#include <asm/asm.h>
#include <string.h>

#define IDT_SIZE ISR_MAX + 1

static struct idt_desc idt[IDT_SIZE];

void isr0(void);
void isr1(void);
void isr2(void);
void isr3(void);
void isr4(void);
void isr5(void);
void isr6(void);
void isr7(void);
void isr8(void);
void isr10(void);
void isr11(void);
void isr12(void);
void isr13(void);
void isr14(void);
void isr16(void);
void isr17(void);
void isr18(void);
void isr19(void);
void isr20(void);
void isr21(void);
void isr28(void);
void isr29(void);
void isr30(void);
void isr32(void);
void isr33(void);
void isr35(void);
void isr36(void);
void isr37(void);
void isr38(void);
void isr39(void);
void isr40(void);
void isr44(void);
void isr45(void);
void isr46(void);
void isr47(void);
void isr48(void);

#if 0
static inline uint16_t read_cs(void)
{
	uint16_t ret;
	asm volatile(intel("mov ax, cs\n") : "=a" (ret));
	return ret;
}
#endif

static void set_idt(size_t index, void * callback)
{
	if (index >= IDT_SIZE) {
		pr_alert("Index %zu too big for IDT size %zu", index, IDT_SIZE);
		return;
	}

	// uint16_t cs = read_cs();
	struct idt_desc desc = {
		.offset_low = (uint64_t) callback & 0xFFFF,
		.segment = gdt_segment(GDT_KERN_CS),
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

static int idt_create(void)
{
	static bool is_init = false;
	if (is_init)
		return 0;

	set_idt(0, (void *) isr0);
	set_idt(1, (void *) isr1);
	set_idt(2, (void *) isr2);
	set_idt(3, (void *) isr3);
	set_idt(4, (void *) isr4);
	set_idt(5, (void *) isr5);
	set_idt(6, (void *) isr6);
	set_idt(7, (void *) isr7);
	set_idt(8, (void *) isr8);
	set_idt(9, NULL);
	set_idt(10, (void *) isr10);
	set_idt(11, (void *) isr11);
	set_idt(12, (void *) isr12);
	set_idt(13, (void *) isr13);
	set_idt(14, (void *) isr14);
	set_idt(15, NULL);
	set_idt(16, (void *) isr16);
	set_idt(17, (void *) isr17);
	set_idt(18, (void *) isr18);
	set_idt(19, (void *) isr19);
	set_idt(20, (void *) isr20);
	set_idt(21, (void *) isr21);
	set_idt(22, NULL);
	set_idt(23, NULL);
	set_idt(24, NULL);
	set_idt(25, NULL);
	set_idt(26, NULL);
	set_idt(27, NULL);
	set_idt(28, (void *) isr28);
	set_idt(29, (void *) isr29);
	set_idt(30, (void *) isr30);
	set_idt(31, NULL);
	set_idt(32, (void *) isr32);
	set_idt(33, (void *) isr33);
	set_idt(34, NULL);
	set_idt(35, (void *) isr35);
	set_idt(36, (void *) isr36);
	set_idt(37, (void *) isr37);
	set_idt(38, (void *) isr38);
	set_idt(39, (void *) isr39);
	set_idt(40, (void *) isr40);
	set_idt(41, NULL);
	set_idt(42, NULL);
	set_idt(43, NULL);
	set_idt(44, (void *) isr44);
	set_idt(45, (void *) isr45);
	set_idt(46, (void *) isr46);
	set_idt(47, (void *) isr47);
	set_idt(48, (void *) isr48);

	is_init = true;
	return 0;
}

int idt_create_load(void)
{
	int err = idt_create();
	if (err)
		return err;

	struct idt_desc * ptr = idt;
	struct idtr idtr = {
		.idt = ptr,
		.limit = sizeof(idt),
	};
	lidt(&idtr);
	return 0;
}
