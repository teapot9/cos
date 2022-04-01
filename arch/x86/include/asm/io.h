#ifndef ASM_CPU_H
#define ASM_CPU_H

#include <stdint.h>

#include <asm/asm.h>

static inline void outb(uint16_t port, uint8_t data)
{
	asm volatile (intel("out dx, al\n") : : "a" (data), "d" (port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t data;
	asm volatile (intel("in al, dx\n") : "=a" (data) : "d" (port));
	return data;
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

#endif // ASM_CPU_H
