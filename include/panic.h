#ifndef PANIC_H
#define PANIC_H

#include <stdnoreturn.h>

#include <cpp.h>

noreturn void panic(const char * fmt, ...) ISR_AVAILABLE;

#endif // PANIC_H
