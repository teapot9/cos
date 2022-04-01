#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>

int sched_next(void);
int sched_enable(void);
void sched_disable(void);

#endif // SCHED_H
