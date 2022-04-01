#ifndef ARCH_X86_KERNEL_CPU_H
#define ARCH_X86_KERNEL_CPU_H

#include "gdt.h"

struct cpu {
	struct gdt gdt;
	struct tss tss;
	uint8_t kstack[CONFIG_KERNEL_FRAME_SIZE];
};

#endif // ARCH_X86_KERNEL_CPU_H
