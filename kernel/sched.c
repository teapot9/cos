#define pr_fmt(fmt) "sched: " fmt

#include <sched.h>
#include "sched.h"

#include <errno.h>
#include <stdbool.h>

#include <panic.h>
#include <print.h>
#include <cpu.h>
#include <task.h>
#include "task.h"

static bool is_enabled = false;

static struct thread * task_next(struct thread * thread)
{
	pid_t pid = thread->parent->pid;
	struct tlist * t = thread_lget(thread->parent, thread->tid);
	if (t == NULL)
		return NULL;
	do {
		while (t != NULL && t->thread.state != TASK_READY)
			t = t->next;
		if (t == NULL) {
			t = process_next(pid)->threads;
			pid = t->thread.parent->pid;
		}
	} while (t == NULL);
	return &t->thread;
}

/* public: sched.h */
int sched_next(void)
{
	if (!is_enabled)
		return -EBUSY;

	struct process * cproc = cpu_proc(cpu_current());
	struct thread * cthread = cpu_thread(cpu_current());
	struct thread * t = task_next(cthread);

	if (t == NULL) {
		pr_err("no task to run after [%zu:%zu]\n",
		       cproc->pid, cthread->tid);
		return -ENOENT;
	}

	//*pid = t->parent->pid;
	//*tid = t->tid;
	pr_debug("task switch: [%zu:%zu] -> [%zu:%zu]\n",
	         cproc->pid, cthread->tid, t->parent->pid, t->tid);
	return 0;
}

/* public: sched.h */
int sched_enable(void)
{
	is_enabled = true;
	return 0;
}

/* public: sched.h */
void sched_disable(void)
{
	is_enabled = false;
}
