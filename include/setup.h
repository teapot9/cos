#ifndef SETUP_H
#define SETUP_H

#include <stdnoreturn.h>

noreturn void kernel_main(void);

void kernel_initcalls_early(void);

void kernel_initcalls(void);

int pmm_init(void);

#ifdef CONFIG_SERIAL_EARLY_DEBUG
int serial_init(void);
#endif

#endif // SETUP_H
