#ifndef CONSOLE_H
#define CONSOLE_H

struct device;

// Use a struct kmsg {int lvl; int categ; char * msg};

// Update console at driver initialization

int console_reg(
	const struct device * dev,
	void (*update)(const struct device *),
	void (*clear)(const struct device *),
	int (*enable)(const struct device *),
	void (*disable)(const struct device *)
);

void console_update(void);

void console_clear(void);

void console_reset(void);

#endif // CONSOLE_H
