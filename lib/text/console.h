#ifndef KERNEL_TEXT_CONSOLE_H
#define KERNEL_TEXT_CONSOLE_H

#include <device.h>

struct console {
	void (*update)(const struct device *);
	void (*clear)(const struct device *);
	int (*enable)(const struct device *);
	void (*disable)(const struct device *);
};

#endif // KERNEL_TEXT_CONSOLE_H
