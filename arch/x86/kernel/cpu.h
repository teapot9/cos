#ifndef ARCH_X86_KERNEL_CPU_H
#define ARCH_X86_KERNEL_CPU_H

#include <cpp.h>
#include "gdt.h"

#include <task.h>

#define MSR_FS_BASE 0xC0000100
#define MSR_GS_BASE 0xC0000101
#define MSR_KGS_BASE 0xC0000102

struct thread;

struct __attribute__((aligned(16))) cpu {
	void * alloc_ptr;
	struct thread * running;
	struct interrupt_frame * state;
	struct gdt __attribute__((aligned(16))) gdt;
	struct tss __attribute__((aligned(16))) tss;
};

#endif // ARCH_X86_KERNEL_CPU_H
