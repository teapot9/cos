#include <debug.h>

#include <stdbool.h>

void kbreak(void)
{
	bool cont = false;

	while (!cont)
		cont = false;
}
