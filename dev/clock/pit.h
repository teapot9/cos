#ifndef DEV_CLOCK_PIT_H
#define DEV_CLOCK_PIT_H

#include <stdbool.h>
#include <assert.h>
#include <stddef.h>

#include <cpp.h>

#define PIT_FREQUENCY 1193182UL
#define PIT_DEFAULT_DIV 1000

#define PIT_IO_CHAN0 0x40
#define PIT_IO_CHAN1 0x41
#define PIT_IO_CHAN2 0x42
#define PIT_IO_CMD 0x43

struct PACKED pit_cmd {
	bool bcd_mode : 1;
	enum op_mode {
		OP_INTERRUPT_TERMINAL = 0,
		OP_RE_TRIGGER_ONE_SHOT = 1,
		OP_RATE_GENERATOR = 2,
		OP_SQUARE_WAVE = 3,
		OP_SOFTWARE_STROBE = 4,
		OP_HARDWARE_STROBE = 5,
	} op : 3;
	enum access_mode {
		ACCESS_LATCH_COUNT_VALUE_CMD = 0,
		ACCESS_LOBYTE_ONLY = 1,
		ACCESS_HIBYTE_ONLY = 2,
		ACCESS_LOBYTE_HIBYTE = 3,
	} access : 2;
	unsigned channel : 2;
};
static_assert(sizeof(struct pit_cmd) == 1, "pit_cmd must be 1 byte");

struct timer_list {
	size_t last;
	size_t msec;
	void (*callback)(void);
	size_t nb_call;
	struct timer_list * next;
	struct timer_list * prec;
};

#endif // DEV_CLOCK_PIT_H
