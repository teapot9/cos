#define pr_fmt(fmt) "sched: " fmt
#include <lock.h>

#include <stdbool.h>
#include <x86intrin.h>

bool nblock_lock(struct spinlock * s)
{
	bool expected = true;
	bool desired = false;
	return atomic_compare_exchange_strong(&s->val, &expected, desired);
}

void nblock_unlock(struct spinlock * s)
{
	s->val = true;
}
