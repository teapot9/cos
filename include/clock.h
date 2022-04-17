#ifndef CLOCK_H
#define CLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

int clock_new(size_t msec, void (*callback)(void), size_t nbcall);
int clock_del(void (*callback)(void));

#ifdef __cplusplus
}
#endif
#endif // CLOCK_H
