#ifndef KERNEL_SCHED_SEMAPHORE_H
#define KERNEL_SCHED_SEMAPHORE_H

#include <lock.h>

struct waiting_elt {
	struct list_head l;
	struct thread * t;
};

struct semaphore_elt {
	struct list_head l;
	struct semaphore * s;
};

#endif // KERNEL_SCHED_SEMAPHORE_H
