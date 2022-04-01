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

static struct cpu * get_cpu(const struct device * dev)
{
	if (dev != NULL
	    && !strcmp(dev->class, "cpu")
	    && !strcmp(dev->type, "cpu"))
		return dev->driver_data;
	return NULL;
}

#if 0
static bool is_current_cpu(const struct cpu * cpu)
{
	if (cpu == NULL)
		return false;
	struct gdtr gdtr = sgdt();
	return gdtr.offset == &cpu->gdt;
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

	cpu->running_proc = NULL;
	cpu->running_thread = NULL;

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
noreturn void cpu_start(size_t pid, size_t tid)
{
	struct process * p = process_get(pid);
	struct thread * t = thread_get(p, tid);
	struct interrupt_frame base_frame __attribute__((aligned(16)));
	task_switch(p, t, &base_frame);
	jmp_to_frame(&base_frame);
}

/* public: cpu.h */
const struct device * cpu_current(void)
{
#if 0
	struct device_iter iter = device_iter_init("cpu", "cpu");
	struct device * dev;
	while ((dev = device_iter_next(&iter)) != NULL
	       && !is_current_cpu(get_cpu(dev)));
	return dev;
#endif
	return (void *) read_msr(MSR_KGS_BASE);
}

/* public: cpu.h */
struct process * cpu_proc(const struct device * dev)
{
	struct cpu * cpu = get_cpu(dev);
	if (cpu == NULL)
		return NULL;
	return cpu->running_proc;
}

/* public: cpu.h */
struct thread * cpu_thread(const struct device * dev)
{
	struct cpu * cpu = get_cpu(dev);
	if (cpu == NULL)
		return NULL;
	return cpu->running_thread;
}

/* public: cpu.h */
void disable_nmi(void)
{
	outb(0x70, inb(0x70) | 0x80);
	inb(0x71);
}

/* public: cpu.h */
void enable_nmi(void)
{
	outb(0x70, inb(0x70) & 0x7F);
	inb(0x71);
}

static int interrupt_state = 0;

static inline void _disable_interrupts(void)
{
	asm volatile (intel("cli\n"));
}

static inline void _enable_interrupts(void)
{
	asm volatile (intel("sti\n"));
}

/* public: cpu.h */
void disable_interrupts(void)
{
	interrupt_state++;
	_disable_interrupts();
}

/* public: cpu.h */
void restore_interrupts(void)
{
	interrupt_state--;
	if (interrupt_state < 0)
		interrupt_state = 0;
	if (!interrupt_state)
		_enable_interrupts();
}

/* public: cpu.h */
void cpu_set_kstack(const struct device * cpu, void * kstack)
{
	struct cpu * cpudev = get_cpu(cpu);
	if (cpudev == NULL)
		return;
	tss_set_kstack(&cpudev->tss, kstack);
}

/* public: cpu,h */
void cpu_update_task(const struct device * cpu,
                     struct process * p, struct thread * t)
{
	struct cpu * cpudev = get_cpu(cpu);
	if (cpudev == NULL)
		return;
	cpudev->running_proc = p;
	cpudev->running_thread = t;
}
