#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stdnoreturn.h>
#include <stdbool.h>

struct device;
struct thread;

void disable_nmi(void);
void enable_nmi(void);
void disable_interrupts(void);
void restore_interrupts(void);

int cpu_reg(const struct device ** dev);
noreturn void cpu_start(size_t pid, size_t tid);

const struct device * cpu_current(void);
struct process * cpu_proc(const struct device * dev);
struct thread * cpu_thread(const struct device * dev);
void cpu_set_kstack(const struct device * cpu, void * kstack);
void cpu_update_task(const struct device * cpu,
                     struct process * p, struct thread * t);

#endif // CPU_H
