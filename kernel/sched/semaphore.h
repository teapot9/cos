#ifndef KERNEL_SCHED_SEMAPHORE_H
#define KERNEL_SCHED_SEMAPHORE_H

struct semaphore_list {
	struct semaphore * s;
	struct semaphore_list * next;
};

struct thread_list {
	struct thread * t;
	struct thread_list * next;
};

#endif // KERNEL_SCHED_SEMAPHORE_H
