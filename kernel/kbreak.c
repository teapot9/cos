#include <debug.h>

#include <stdbool.h>

/* public: debug.h */
void kbreak(void)
{
	bool cont = false;
	while (!cont);
}
