#ifndef __CPU_H
#define __CPU_H

#include <stddef.h>
#include <stdnoreturn.h>
#include <stdbool.h>

struct device;
struct cpu;
struct thread;
struct interrupt_frame;

void disable_nmi(void);
void enable_nmi(void);
void disable_interrupts(void);
void restore_interrupts(void);
void interrupt_start(void);
void interrupt_end(void);
noreturn void hang(void);

int cpu_reg(const struct device ** dev);
noreturn void cpu_start(void);
void cpu_set_state(struct cpu * cpu, struct interrupt_frame * state);

struct cpu * cpu_current(void);
struct thread * cpu_running(struct cpu * cpu);
void cpu_load_state(struct cpu * cpu, struct interrupt_frame * state);
void cpu_save_state(struct cpu * cpu, struct interrupt_frame * state);
void cpu_load_kstack(struct cpu * cpu, void * kstack);
void cpu_set_task(struct cpu * cpu, struct thread * t);

#endif // __CPU_H
