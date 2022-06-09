#ifndef ARCH_X86_KERNEL_GDT_H
#define ARCH_X86_KERNEL_GDT_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <gdt.h>
#include <cpp.h>
#include <asm/asm.h>
#include "isr.h"
#include <alloc.h>

struct PACKED gdt_desc {
	unsigned limit_low : 16;
	unsigned base_low : 24;
	bool ac : 1;
	bool rw_busy : 1;
	bool dc : 1;
	bool ex : 1;
	bool s : 1;
	unsigned privl : 2;
	bool pr : 1;
	unsigned limit_high : 4;
	bool avl: 1;
	bool l : 1;
	bool sz : 1;
	bool gr : 1;
	unsigned base_high : 8;
};
static_assert(sizeof(struct gdt_desc) == 8, "gdt_desc should be 8 bytes");

struct PACKED tss_desc {
	struct gdt_desc gdt;
	unsigned base_higher : 32;
	unsigned _zero0 : 32;
};
static_assert(sizeof(struct tss_desc) == 16, "tss_desc should be 16 bytes");

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

struct PACKED gdt {
	struct gdt_desc null;
	struct gdt_desc kernel_code;
	struct gdt_desc kernel_data;
	struct gdt_desc user_code;
	struct gdt_desc user_data;
	struct tss_desc tss;
};

static inline void lgdt(struct gdtr gdtr)
{
	asm volatile (intel("lgdt [rax]\n") : : "a" (&gdtr));
}

static inline struct gdtr sgdt(void)
{
	struct gdtr gdtr;
	struct gdtr * pgdtr = &gdtr;
	asm volatile (intel("sgdt [rax]\n") : "=a" (pgdtr));
	return gdtr;
}

static inline uint16_t get_cs(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, cs\n") : "=a" (v));
	return v;
}

static inline uint16_t get_ds(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, ds\n") : "=a" (v));
	return v;
}

static inline uint16_t get_ss(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, ss\n") : "=a" (v));
	return v;
}

static inline uint16_t get_es(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, es\n") : "=a" (v));
	return v;
}

static inline uint16_t get_fs(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, fs\n") : "=a" (v));
	return v;
}

static inline uint16_t get_gs(void)
{
	uint16_t v;
	asm volatile (intel("mov ax, gs\n") : "=a" (v));
	return v;
}

static inline void set_cs(uint16_t v)
{
	asm volatile (intel(
		"lea rbx, [rip + .set_cs_end]\n"
		"push rax\n"
		"push rbx\n"
		"retfq\n"
		".set_cs_end:\n"
	) : : "a" (v) : "rbx");
}

static inline void set_ds(uint16_t v)
{
	asm volatile (intel("mov ds, ax\n") : : "a" (v));
}

static inline void set_ss(uint16_t v)
{
	asm volatile (intel("mov ss, ax\n") : : "a" (v));
}

static inline void set_es(uint16_t v)
{
	asm volatile (intel("mov es, ax\n") : : "a" (v));
}

static inline void set_fs(uint16_t v)
{
	asm volatile (intel("mov fs, ax\n") : : "a" (v));
}

static inline void set_gs(uint16_t v)
{
	asm volatile (intel("mov gs, ax\n") : : "a" (v));
}

static inline uint16_t str(void)
{
	uint16_t v;
	asm volatile (intel("str ax\n") : "=a" (v));
	return v;
}

static inline void ltr(uint16_t v)
{
	asm volatile (intel("ltr ax\n") : : "a" (v));
}

int gdt_create_load(struct gdt * gdt, struct tss * tss, void * kstack);

#endif // ARCH_X86_KERNEL_GDT_H
