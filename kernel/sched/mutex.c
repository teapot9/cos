#define pr_fmt(fmt) "sched: " fmt
#include <lock.h>

#include <stdatomic.h>

#if 0
void mutex_init(struct semaphore * s)
{
	semaphore_init(s, 1);
}
#endif

void mutex_lock(struct semaphore * s)
{
	semaphore_lock(s);
}

void mutex_unlock(struct semaphore * s)
{
	semaphore_unlock(s);
}

void mutex_unlock_all(struct thread * t)
{
	semaphore_unlock_all(t);
}
