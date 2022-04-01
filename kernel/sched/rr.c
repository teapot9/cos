#define pr_fmt(fmt) "sched: " fmt

#include "rr.h"
#include <sched.h>
#include "sched.h"

#include <errno.h>
#include <stdbool.h>

#include <panic.h>
#include <print.h>
#include <cpu.h>
#include <task.h>
#include "../task.h"

#define task_ready(t) (t != NULL && t->thread.state == TASK_READY)
#define loop_task(t) (!task_ready(t) && t != NULL && &t->thread != thread)

static struct thread * task_next(struct thread * thread)
{
	mutex_lock(&thread->lock);

	pid_t pid = thread->parent->pid;
	struct tlist * t = thread_lget(thread->parent, thread->tid);
	if (t == NULL)
		return NULL;

	t = t->next;
	do {
		while (loop_task(t))
			t = t->next;
		if (t == NULL) {
			t = process_next(pid)->threads;
			pid = t->thread.parent->pid;
		}
	} while (loop_task(t));

	mutex_unlock(&thread->lock);
	if (!task_ready(t))
		return NULL;
	return &t->thread;
}

static struct thread * task_first(void)
{
	return &process_next(0)->threads->thread;
}

/* public: sched.h */
int sched_next(struct thread ** next, struct thread * cur)
{
	if (next == NULL)
		return -EINVAL;
	if (!sched_enabled())
		return -EBUSY;
	if (cur == NULL)
		cur = task_first();

	struct thread * t = task_next(cur);

	if (t == NULL) {
		pr_err("no task to run after [%zu:%zu]\n",
		       cur->parent->pid, cur->tid);
		return -ENOENT;
	}

	/*
	pr_debug("task switch: [%zu:%zu] -> [%zu:%zu]\n",
	         cur->parent->pid, cur->tid, t->parent->pid, t->tid);
	*/
	*next = t;
	return 0;
}
