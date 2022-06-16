#ifndef ARCH_X86_KERNEL_CPU_H
#define ARCH_X86_KERNEL_CPU_H

#include <cpp.h>
#include "gdt.h"

#include <task.h>

#define MSR_FS_BASE 0xC0000100
#define MSR_GS_BASE 0xC0000101
#define MSR_KGS_BASE 0xC0000102

struct thread;

struct _aligned_(16) cpu {
	struct thread * running;
	struct interrupt_frame * state;
	struct gdt _aligned_(16) gdt;
	struct tss _aligned_(16) tss;
	int interrupt_state;
	bool is_in_interrupt;
};

#endif // ARCH_X86_KERNEL_CPU_H
