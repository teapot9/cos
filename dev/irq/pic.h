#ifndef DEV_IRQ_PIC_H
#define DEV_IRQ_PIC_H

#include <stdbool.h>
#include <assert.h>

#include <cpp.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

struct _packed_ icw1 {
	bool icw4 : 1;
	bool single : 1;
	bool interval4 : 1;
	bool level : 1;
	bool init : 1;
	bool eoi : 1;
	unsigned _reserved0 : 2;
};
static_assert(sizeof(struct icw1) == 1, "icw1 must be 1 byte");

struct _packed_ icw4 {
	bool m8086 : 1;
	bool auto_eoi : 1;
	bool buf_master : 1; // need buf_slave = true
	bool buf_slave : 1;
	bool sfnm : 1;
	unsigned _reserved0 : 3;
};
static_assert(sizeof(struct icw4) == 1, "icw4 must be 1 byte");

#endif // DEV_IRQ_PIC_H
