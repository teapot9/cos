#ifndef CLOCK_H
#define CLOCK_H

#include <stddef.h>

int clock_new(size_t msec, void (*callback)(void), size_t nbcall);
int clock_del(void (*callback)(void));

#endif // CLOCK_H
