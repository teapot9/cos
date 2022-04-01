#ifndef ARCH_X86_KERNEL_GDT_H
#define ARCH_X86_KERNEL_GDT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <cpp.h>
#include <asm/asm.h>

struct PACKED gdt_desc {
	unsigned limit_low : 16;
	unsigned base_low : 24;
	union desc_access {
		struct PACKED tss_desc_access {
			unsigned _one0 : 1;
			bool busy : 1;
			unsigned _zero0 : 1;
			unsigned _one1 : 1;
			unsigned _zero1 : 1;
			unsigned privl : 2;
			bool pr : 1;
		} tss;
		struct PACKED gdt_desc_access {
			bool ac : 1;
			bool rw : 1;
			unsigned dc : 1;
			bool ex : 1;
			bool s : 1;
			unsigned privl : 2;
			bool pr : 1;
		} gdt;
	} access;
	unsigned limit_high : 4;
	union desc_flags {
		struct PACKED tss_desc_flags {
			bool avl : 1;
			unsigned _zero0 : 2;
			bool gr : 1;
		} tss;
		struct PACKED gdt_desc_flags {
			unsigned _reserved0 : 1;
			bool l : 1;
			bool sz : 1;
			bool gr : 1;
		} gdt;
	} flags;
	unsigned base_high : 8;
};

struct PACKED gdtr {
	uint16_t size;
	struct gdt * offset;
};
static_assert(sizeof(struct gdtr) == 10, "gdtr should be 10 bytes");

struct PACKED tss {
	uint32_t _reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t _reserved1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t _reserved2;
	uint16_t _reserved3;
	uint16_t iopb_offset;
};

#define GDT_KERN_CS 1
#define GDT_KERN_DS 2
#define GDT_USER_CS 3
#define GDT_USER_DS 4
#define GDT_TSS 5

struct PACKED gdt {
	struct gdt_desc null;
	struct gdt_desc kernel_code;
	struct gdt_desc kernel_data;
	struct gdt_desc user_code;
	struct gdt_desc user_data;
	struct gdt_desc tss;
};

static inline void lgdt(struct gdtr gdtr)
{
	asm volatile (intel("lgdt [rax]\n") : : "a" (&gdtr));
}

static inline struct gdtr sgdt(void)
{
	struct gdtr gdtr;
	asm volatile (intel("sgdt [rax]\n") : "=a" (gdtr));
	return gdtr;
}

int gdt_create_load(struct gdt * gdt, struct tss * tss, void * kstack);

#endif // ARCH_X86_KERNEL_GDT_H
