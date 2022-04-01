#ifndef SETUP_H
#define SETUP_H

#include <stdnoreturn.h>

struct memlist;
struct memmap;

noreturn void kernel_main(void);

void kernel_initcalls_early(void);

void kernel_initcalls(void);

int pmm_init(struct memmap * newmap);

#ifdef CONFIG_SERIAL_EARLY_DEBUG
int serial_init(void);
#endif

#ifdef BOOTLOADER
extern struct vmemmap kvmemmap;
#endif

#endif // SETUP_H
