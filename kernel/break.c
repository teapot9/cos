#include <debug.h>

#include <stdbool.h>

#include <asm/asm.h>

void _kbreak(void) {
	volatile bool cont = false;
	while (!cont);
}
