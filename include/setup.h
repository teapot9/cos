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

int process_pid0(void);

#endif // SETUP_H
