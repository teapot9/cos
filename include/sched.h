#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>
#include <task.h>

struct thread;

int sched_next(struct thread ** next, struct thread * cur);
int sched_enable(void);
void sched_disable(void);
void sched_yield(void);

#endif // SCHED_H
