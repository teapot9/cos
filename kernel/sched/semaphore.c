#define pr_fmt(fmt) "sched: " fmt
#include <lock.h>
#include "semaphore.h"

#include <errno.h>
#include <stdatomic.h>

#include <sched.h>
#include <task.h>
#include <cpu.h>
#include <mm.h>
#include <panic.h>

static void add_to_thread(struct thread * t, struct semaphore * s)
{
	struct semaphore_list ** l = task_semaphores(t);
	if (l == NULL)
		return;

	struct semaphore_list * new = kmalloc(sizeof(**l));
	if (new == NULL)
		panic("cannot add to list of semaphore, maybe kill proc");

	new->s = s;
	new->next = *l;
	*l = new;
}

static int del_from_thread(struct thread * t, struct semaphore * s)
{
	struct semaphore_list ** l = task_semaphores(t);
	if (l == NULL)
		return -EINVAL;

	while (*l != NULL && (*l)->s != s)
		l = &(*l)->next;
	if (*l == NULL)
		return -ENOENT;

	struct semaphore_list * tmp = *l;
	*l = tmp->next;
	kfree(tmp);
	return 0;
}

static void add_thread(struct semaphore * s, struct thread * t)
{
	struct thread_list ** l = &s->threads;

	while (*l != NULL)
		l = &(*l)->next;
	struct thread_list * new = kmalloc(sizeof(**l));
	if (new == NULL)
		panic("cannot add to waiting list, maybe kill proc");

	new->t = t;
	new->next = *l;
	*l = new;
}

static int del_thread(struct semaphore * s, struct thread * t)
{
	struct thread_list ** l = &s->threads;

	if (l == NULL)
		return -EINVAL;

	while (*l != NULL && (*l)->t != t)
		l = &(*l)->next;
	if (*l == NULL)
		return -ENOENT;

	struct thread_list * tmp = *l;
	*l = tmp->next;
	kfree(tmp);
	return 0;
}

static void do_sleep(struct cpu * cpu, struct thread * cur)
{
	struct thread * next;
	int err = sched_next(&next, cur);
	if (err)
		panic("no task to switch to, errno = %d", err);

	task_switch(cpu, next);
}

#if 0
void semaphore_init(struct semaphore * s, unsigned n)
{
	s->val = n;
	s->threads = NULL;
}
#endif

static void wake_next_thread(struct thread_list * l)
{
	while (l != NULL && task_get_state(l->t) != TASK_WAITING)
		l = l->next;
	if (l != NULL)
		task_set_state(l->t, TASK_READY);
}

void semaphore_lock(struct semaphore * s)
{
	struct cpu * cpu = cpu_current();
	struct thread * t = cpu_running(cpu);

	unsigned expected;
	unsigned desired;
	add_to_thread(t, s); // add to thread's semaphores
	add_thread(s, t); // add thread to semaphore's waiting list
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
	del_thread(s, t); // remove from semaphore's waiting list
}

void semaphore_unlock(struct semaphore * s)
{
	struct thread * t = cpu_running(cpu_current());
	del_from_thread(t, s); // remove from thread's semaphores
	s->val++;
	wake_next_thread(s->threads);
}

void semaphore_unlock_all(struct semaphore_list * l)
{
	struct thread * t = cpu_running(cpu_current());
	while (l != NULL) {
		int waiting = 0;
		int owned = 0;
		while (del_thread(l->s, t) != -ENOENT)
			waiting++;
		while (del_from_thread(t, l->s) != -ENOENT)
			owned++;
		l->s->val += owned - waiting;
		l = l->next;
	}
}
