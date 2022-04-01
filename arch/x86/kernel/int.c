#include <cpu.h>
#include "cpu.h"

#include <asm/io.h>

static int boot_interrupt_state = 0;

static inline void _disable_interrupts(void)
{
	asm volatile (intel("cli\n"));
}

static inline void _enable_interrupts(void)
{
	asm volatile (intel("sti\n"));
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

#ifdef BOOTLOADER
# define cpu_current() NULL
#endif
/* public: cpu.h */
void disable_interrupts(void)
{
	struct cpu * cpu = cpu_current();
	int * interrupt_state = cpu == NULL
		? &boot_interrupt_state : &cpu->interrupt_state;

	(*interrupt_state)++;
	if (cpu != NULL && cpu->is_in_interrupt)
		return;
	_disable_interrupts();
}

/* public: cpu.h */
void restore_interrupts(void)
{
	struct cpu * cpu = cpu_current();
	int * interrupt_state = cpu == NULL
		? &boot_interrupt_state : &cpu->interrupt_state;

	(*interrupt_state)--;
	if (*interrupt_state < 0)
		*interrupt_state = 0;
	if (cpu != NULL && cpu->is_in_interrupt)
		return;
	if (!*interrupt_state)
		_enable_interrupts();
}

/* public: cpu.h */
void interrupt_start(void)
{
	struct cpu * cpu = cpu_current();
	if (cpu == NULL)
		return;
	cpu->is_in_interrupt = true;
}

/* public: cpu.h */
void interrupt_end(void)
{
	struct cpu * cpu = cpu_current();
	if (cpu == NULL)
		return;
	cpu->is_in_interrupt = false;
}

/* public: cpu.h */
noreturn void hang(void)
{
	while (1)
		asm volatile (intel("hlt\n"));
}
