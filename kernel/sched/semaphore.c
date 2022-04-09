#define pr_fmt(fmt) "sched: " fmt
#include <lock.h>
#include "semaphore.h"

#include <errno.h>
#include <stdatomic.h>

#include <print.h>
#include <sched.h>
#include <task.h>
#include <cpu.h>
#include <mm.h>
#include <panic.h>
#include <list.h>

static inline struct list_head * list(struct waiting_elt * elt) {
	return (struct list_head *) elt;
}

static inline struct waiting_elt * waiting(struct list_head * l) {
	return (struct waiting_elt *) l;
}

static int add_to_thread(struct thread * t, struct semaphore * s)
{
	if (t == NULL || s == NULL)
		return -EINVAL;

	struct semaphore_elt * elt = malloc(sizeof(*elt));
	if (elt == NULL)
		return -ENOMEM;
	elt->s = s;

	list_append(&t->semaphores, &elt->l);
	return 0;
}

static int del_from_thread(struct thread * t, struct semaphore * s)
{
	if (t == NULL || s == NULL)
		return -EINVAL;

	struct semaphore_elt * cur;
	list_foreach(cur, t->semaphores.first) {
		if (cur->s == s) {
			list_del(&t->semaphores, &cur->l, true);
			return 0;
		}
	}
	return -ENOENT;
}

static int add_to_waiting(struct semaphore * s, struct thread * t)
{
	if (s == NULL || t == NULL)
		return -EINVAL;

	struct waiting_elt * elt = malloc(sizeof(*elt));
	if (elt == NULL)
		return -ENOMEM;
	elt->t = t;

	list_append(&s->waiting, &elt->l);
	return 0;
}

static int del_from_waiting(struct semaphore * s, struct thread * t)
{
	if (s == NULL || t == NULL)
		return -EINVAL;

	struct waiting_elt * cur;
	list_foreach(cur, s->waiting.first) {
		if (cur->t == t) {
			list_del(&s->waiting, &cur->l, true);
			return 0;
		}
	}
	return -ENOENT;
}

static void do_sleep(struct cpu * cpu, struct thread * cur)
{
	struct thread * next;
	int err = sched_next(&next, cur);
	if (err)
		panic("no task to switch to, errno = %d", err);

	task_switch(cpu, next);
}

static void wake_next_waiting(struct semaphore * s)
{
	if (s == NULL)
		return;

	struct waiting_elt * cur;
	list_foreach(cur, s->waiting.first) {
		if (task_get_state(cur->t) == TASK_WAITING) {
			task_set_state(cur->t, TASK_READY);
			break;
		}
	}
}

void semaphore_lock(struct semaphore * s)
{
	struct thread * t = cpu_running(cpu_current());
	if (t == NULL) {
		return;
	}

	unsigned expected;
	unsigned desired;
	add_to_thread(t, s); // add to thread's semaphores
	add_to_waiting(s, t); // add thread to semaphore's waiting list
	do {
		if (s->val > 0) {
			expected = s->val;
			desired = s->val - 1;
		} else {
			expected = 1;
			desired = 0;
			sched_yield();
		}
	} while (!atomic_compare_exchange_strong(&s->val, &expected, desired));
	del_from_waiting(s, t); // remove from semaphore's waiting list
}

static void _semaphore_unlock(struct semaphore * s, struct thread * t)
{
	if (s == NULL || t == NULL)
		return;
	del_from_thread(t, s); // remove from thread's semaphores
	s->val++;
	wake_next_waiting(s);
}

void semaphore_unlock(struct semaphore * s)
{
	struct thread * t = cpu_running(cpu_current());
	if (t == NULL) {
		return;
	}
	_semaphore_unlock(s, t);
}

void semaphore_unlock_all(struct thread * t)
{
	if (t == NULL)
		return;
	while (t->semaphores.first != NULL) {
		struct semaphore_elt * cur = (void *) t->semaphores.first;
		_semaphore_unlock(cur->s, t);
	}
}
