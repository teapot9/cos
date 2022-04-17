#ifndef PANIC_H
#define PANIC_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdnoreturn.h>

#include <cpp.h>

noreturn void panic(const char * fmt, ...) ISR_AVAILABLE;

#ifdef __cplusplus
}
#endif
#endif // PANIC_H
