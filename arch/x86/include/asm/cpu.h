#ifndef ASM_CPU_H
#define ASM_CPU_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <asm/asm.h>

static inline uint64_t read_cr0(void)
{
	uint64_t val;
	asm volatile (intel("mov rax, cr0\n") : "=a" (val));
	return val;
}

static inline void write_cr0(uint64_t val)
{
	asm volatile (intel("mov cr0, rax\n") : : "a" (val));
}

static inline uint64_t read_cr3(void)
{
	uint64_t val;
	asm volatile (intel("mov rax, cr3\n") : "=a" (val));
	return val;
}

static inline void write_cr3(uint64_t val)
{
	asm volatile (intel("mov cr3, rax\n") : : "a" (val));
}

static inline uint64_t read_cr4(void)
{
	uint64_t val;
	asm volatile (intel("mov rax, cr4\n") : "=a" (val));
	return val;
}

static inline void write_cr4(uint64_t val)
{
	asm volatile (intel("mov cr4, rax\n") : : "a" (val));
}

static inline uint64_t read_msr(uint32_t msr)
{
	uint32_t val_low, val_high;
	asm volatile (
		intel("rdmsr\n")
		: "=d" (val_high),
		"=a" (val_low)
		: "c" (msr)
	);
	return val_low | ((uint64_t) val_high << 32);
}

static inline void write_msr(uint32_t msr, uint64_t val)
{
	asm volatile (
		intel("wrmsr\n")
		:
		: "a" (val >> 0),
		"d" (val >> 32),
		"c" (msr)
	);
}

struct __attribute__((packed)) cr4 {
	bool vme : 1;
	bool pvi : 1;
	bool tsd : 1;
	bool de : 1;
	bool pse : 1;
	bool pae : 1;
	bool mce : 1;
	bool pge : 1;
	bool pce : 1;
	bool osfxsr : 1;
	bool osxmmexcpt : 1;
	bool umip : 1;
	unsigned _reserved1 : 1;
	bool vmxe : 1;
	bool smxe : 1;
	bool _reserved2 : 1;
	bool fsgsbase : 1;
	bool pcide : 1;
	bool osxsave : 1;
	unsigned _reserved3 : 1;
	bool smep : 1;
	bool smap : 1;
	bool pke : 1;
	bool cet : 1;
	bool pks : 1;
	unsigned long _reserved4 : 39;
};
static_assert(sizeof(struct cr4) == 8, "CR4 must be 64 bits");

static inline uint64_t read_rflags(void)
{
	uint64_t v;
	asm volatile (intel(
		"pushfq\n"
		"pop rax\n"
	) : "=a" (v));
	return v;
}

static inline void swapgs(void)
{
	asm volatile (intel("swapgs\n"));
}

#endif // ASM_CPU_H
