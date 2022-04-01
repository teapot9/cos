#include <cpu.h>
#include "cpu.h"

#include <errno.h>
#include <stdbool.h>

#include <sched.h>
#include <panic.h>
#include <task.h>
#include <string.h>
#include <mm.h>
#include <mm/helper.h>
#include <print.h>
#include <device.h>
#include "gdt.h"
#include "idt.h"
#include <asm/io.h>
#include <asm/cpu.h>

#define CPU_ALIGN 16

static int dev_reg(const struct device * dev)
{
	//write_msr(MSR_FS_BASE, (uint64_t) NULL);
	//write_msr(MSR_GS_BASE, (uint64_t) dev);
	write_msr(MSR_KGS_BASE, (uint64_t) dev);
	pr_info("%s: initialized CPU\n", dev->name);
	return 0;
}

static void dev_unreg(const struct device * dev)
{
	kfree(dev->driver_data);
}

#if 0
static struct cpu * get_cpu(const struct device * dev)
{
	if (dev != NULL
	    && !strcmp(dev->class, "cpu")
	    && !strcmp(dev->type, "cpu"))
		return dev->driver_data;
	return NULL;
}
#endif

/* public: cpu.h */
int cpu_reg(const struct device ** dev)
{
	int err;
	size_t nproc = 0;
	struct cpu * cpu = NULL;

	void * cpu_ptr = kmalloc(sizeof(*cpu) + CPU_ALIGN);
	if (cpu_ptr == NULL)
		return -ENOMEM;
	cpu = aligned(cpu_ptr, CPU_ALIGN);

	cpu->alloc_ptr = cpu_ptr;
	cpu->running = NULL;
	cpu->state = NULL;

	err = idt_create_load();
	if (err) {
		pr_err("cpu: failed to load IDT, errno = %d\n", err);
		return err;
	}

	err = gdt_create_load(&cpu->gdt, &cpu->tss, NULL);
	if (err) {
		pr_err("cpu: failed to load GDT, errno = %d\n", err);
		return err;
	}

	err = device_create(dev, core_module(), NULL, "cpu", "cpu",
	                    dev_reg, dev_unreg, cpu, "cpu%zu", nproc++);
	if (err) {
		pr_err("cpu: failed to create device, errno = %d\n", err);
		return err;
	}

	return 0;
}

/* public: cpu.h */
noreturn void cpu_start(void)
{
	struct cpu * cpu = cpu_current();

	struct interrupt_frame base_frame __attribute__((aligned(16)));
	cpu->state = &base_frame;

	struct thread * next;
	int err = sched_next(&next, NULL);
	if (err)
		panic("no process to run on this cpu, errno = %d", err);

	task_switch(cpu, next);

	jmp_to_frame(cpu->state);
}

/* public: cpu.h */
void cpu_set_state(struct cpu * cpu, struct interrupt_frame * state)
{
	if (cpu != NULL && state != NULL)
		cpu->state = state;
}

/* public: cpu.h */
struct cpu * cpu_current(void)
{
	return ((struct device *) read_msr(MSR_KGS_BASE))->driver_data;
}

/* public: cpu.h */
struct thread * cpu_running(struct cpu * cpu)
{
	return cpu == NULL ? NULL : cpu->running;
}

/* public: cpu.h */
void cpu_load_state(struct cpu * cpu, struct interrupt_frame * state)
{
	if (cpu != NULL && state != NULL)
		*cpu->state = *state;
}

/* public: cpu.h */
void cpu_save_state(struct cpu * cpu, struct interrupt_frame * state)
{
	if (cpu != NULL && state != NULL)
		*state = *cpu->state;
}

/* public: cpu.h */
void cpu_load_kstack(struct cpu * cpu, void * kstack)
{
	if (cpu != NULL && kstack != NULL)
		tss_set_kstack(&cpu->tss, kstack);
}

/* public: cpu.h */
void cpu_set_task(struct cpu * cpu, struct thread * t)
{
	if (cpu != NULL && t != NULL)
		cpu->running = t;
}
