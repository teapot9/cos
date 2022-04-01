#define pr_fmt(fmt) "sched: " fmt
#include <lock.h>

#include <stdbool.h>
#include <x86intrin.h>

void spinlock_lock(struct spinlock * s)
{
	bool expected = true;
	bool desired = false;
	_mm_monitor(&s->val, 0, 0);
	while (!atomic_compare_exchange_strong(&s->val, &expected, desired)) {
		expected = true;
		desired = false;
		_mm_mwait(0, 0);
	}
}

void spinlock_unlock(struct spinlock * s)
{
	s->val = true;
}
