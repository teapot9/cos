#ifndef DEV_CLOCK_CLOCK_H
#define DEV_CLOCK_CLOCK_H

#include <stddef.h>

struct device;

struct clock {
	int (*new)(size_t msec, void (*callback)(void), size_t nbcall);
	int (*del)(void (*callback)(void));
};

int clock_reg(
	const struct device ** dev,
	const struct device * parent,
	int (*new)(size_t msec, void (*callback)(void), size_t nbcall),
	int (*del)(void (*callback)(void))
);

#endif // DEV_CLOCK_CLOCK_H
