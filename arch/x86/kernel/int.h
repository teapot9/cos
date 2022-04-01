#ifndef KERNEL_INT_H
#define KERNEL_INT_H

#include <stdbool.h>

struct __attribute__((packed)) page_fault_code {
	bool present : 1;
	bool write : 1;
	bool user : 1;
	bool reserved_write : 1;
	bool instruction_fetch : 1;
	bool protection_key : 1;
	bool shadow_stack : 1;
	unsigned _reserved1 : 8;
	bool software_guard_extensions : 1;
	unsigned _reserved2 : 16;
};
void int_init(void);

#endif // KERNEL_INT_H
