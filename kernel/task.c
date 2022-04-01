#include <setup.h>
#include <task.h>
#include "task.h"

#include <lock.h>
#include <print.h>
#include <cpu.h>
#include <panic.h>
#include <mm.h>
#include <asm/cpu.h>
#include <gdt.h>
#include <errno.h>
#include <mm/helper.h>
#include <sched.h>

#define kstack_aligned(x) ((uint8_t *) aligned(x, KSTACK_ALIGN) + KSTACK_SIZE)

struct process processes[MAX_PROC_CNT];
uint8_t used[MAX_PROC_CNT] = {0};

static uint64_t kcr3 = 0;

static bool is_used(pid_t pid)
{
	if (pid > MAX_PROC_CNT)
		panic("PID is too high: %zu > %zu", pid, MAX_PROC_CNT);
	return used[pid / 8] & (1 << (7 - pid % 8));
}

static void set_used(pid_t pid, bool state)
{
	if (pid > MAX_PROC_CNT)
		panic("PID is too high: %zu > %zu", pid, MAX_PROC_CNT);
	uint8_t v = state ? 1 << (7 - pid % 8) : 0;
	used[pid / 8] = (used[pid / 8] & ~(1 << (7 - pid % 8))) | v;
}

static inline bool is_kernel_process(struct process * p)
{
	return p->pid == 0;
}

static pid_t new_pid(void)
{
	static pid_t last_pid = MAX_PID;
	pid_t cur = last_pid;
	do {
		if (++cur > MAX_PID)
			cur = 0;
	} while (is_used(cur));
	set_used(cur, true);
	last_pid = cur;
	return cur;
}

static bool is_tid_available(struct process * p, tid_t tid)
{
	if (tid == SIZE_MAX)
		return false;
	struct tlist * t = p->threads;
	while (t != NULL) {
		if (t->thread.tid == tid)
			return false;
		t = t->next;
	}
	return true;
}

static tid_t new_tid(struct process * p)
{
	tid_t cur = p->last_tid;
	while (!is_tid_available(p, ++cur));
	p->last_tid = cur;
	return cur;
}

noreturn static void kthread_wrapper(void (*start)(void))
{
	start();
	thread_kill(cpu_running(cpu_current()));
	sched_yield();
	hang();
}

static int thread_init(
	struct thread * thread, struct process * parent, void (* start)(void)
)
{
	if (thread == NULL || parent == NULL || start == NULL)
		return -EINVAL;

	thread->lock = mutex_init();
	thread->parent = parent;
	thread->tid = new_tid(parent);
	thread->state = TASK_READY;
	thread->running = NULL;
	thread->semaphores = NULL;

	void * stack;
	if (is_kernel_process(parent)) {
		struct page_perms perms = {.exec = false, .user = thread->parent->pid != 0, .write = true};
		thread->stack = valloc(thread->parent->pid, KSTACK_SIZE, KSTACK_ALIGN, KSTACK_ALIGN, perms);
		if (thread->stack == NULL)
			return -ENOMEM;
		stack = thread->stack;
	} else {
		// Should create user stack in user CR3
		return -ENOTSUP;
	}

	thread->task_state = (struct interrupt_frame) {0};
	uword_t data_seg = is_kernel_process(parent) ?
		gdt_segment(GDT_KERN_DS) : gdt_segment(GDT_USER_DS);

	thread->task_state.ds = data_seg;
	thread->task_state.es = data_seg;
	thread->task_state.fs = data_seg;
	thread->task_state.gs = data_seg;
	thread->task_state.ip = (uword_t) start;
	thread->task_state.cs = is_kernel_process(parent) ?
		gdt_segment(GDT_KERN_CS) : gdt_segment(GDT_USER_CS);
	thread->task_state.flags = read_rflags();
	thread->task_state.sp = (uword_t) stack;
	thread->task_state.ss = data_seg;

	return 0;
}

static int process_init(
	pid_t pid, struct process * parent
)
{
	struct process * proc = process_get(pid);
	if (proc == NULL)
		return -EINVAL;

	proc->pid = pid;
	if (parent == NULL && proc->pid != 0)
		return -EINVAL;
	proc->parent = parent;
	proc->threads = NULL;

	uword_t cr3 = 0;
	/*
	if (is_kernel_process(proc)) {
		cr3 = kcr3;
	} else {
		// Should form kernel CR3
		return -ENOTSUP;
	}
	*/
	proc->cr3 = cr3;
	proc->root_table_vaddr = NULL;
	proc->last_tid = SIZE_MAX;

	return 0;
}

struct process * process_next(pid_t pid)
{
	do {
		if (++pid > MAX_PID)
			pid = 0;
	} while (!is_used(pid));
	return &processes[pid];
}

struct tlist * thread_lget(struct process * p, tid_t tid)
{
	if (p == NULL)
		return NULL;

	struct tlist * cur = p->threads;
	while (cur != NULL && cur->thread.tid != tid)
		cur = cur->next;
	return cur;
}

/* public: task.h */
int thread_new(
	struct process * proc, void (* start)(void)
)
{
	struct tlist * new = kmalloc(sizeof(*new));
	if (new == NULL)
		return -ENOMEM;

	int err = thread_init(&new->thread, proc, start);
	if (err) {
		kfree(new);
		return err;
	}

	new->next = proc->threads;
	proc->threads = new;
	return new->thread.tid;
}

/* public: task.h */
int kthread_new(void (*start)(void))
{
	if (!is_used(0))
		return -EAGAIN;
	tid_t tid = thread_new(&processes[0], start);

	struct thread * t = thread_get(&processes[0], tid);
	if (t == NULL)
		panic("cannot find newly created thread [0:%zu]", tid);
	mutex_lock(&t->lock);
	t->task_state.ip = (uword_t) kthread_wrapper;
	t->task_state.di = (uword_t) start;
	mutex_unlock(&t->lock);

	return tid;
}

/* public: task.h */
int process_new(struct process * parent, void (* start)(void))
{
	int err;

	pid_t pid = new_pid();
	if (start == NULL && pid != 0)
		return -EINVAL;

	struct process * proc = process_get(pid);
	if (proc == NULL)
		return -EAGAIN;

	err = process_init(pid, parent);
	if (err)
		return err;

	if (start != NULL) {
		err = thread_new(proc, start);
		if (err < 0)
			return err;
		if (err > 0)
			return -EINVAL;
	}

	return proc->pid;
}

/* public: task.h */
void thread_delete(struct thread * t)
{
	struct tlist ** cur = &t->parent->threads;
	while (*cur != NULL && (*cur)->thread.tid != t->tid)
		cur = &(*cur)->next;
	if (*cur) {
		struct tlist * tmp = *cur;
		*cur = (*cur)->next;
		kfree(tmp);
	}
}

/* public: task.h */
void thread_kill(struct thread * t)
{
	mutex_lock(&t->lock);
	t->state = TASK_TERMINATED;
	semaphore_unlock_all(t->semaphores);
	mutex_unlock(&t->lock);
}

/* public: task.h */
struct process * process_get(pid_t pid)
{
	if (pid <= MAX_PID && is_used(pid))
		return &processes[pid];
	return NULL;
}

/* public: task.h */
struct thread * thread_get(struct process * p, tid_t tid)
{
	struct tlist * t = thread_lget(p, tid);
	return t == NULL ? NULL : &t->thread;
}

/* public: task.h */
void * thread_kstack_ptr(struct thread * thread)
{
	return kstack_aligned(thread->kstack);
}

/* public: task.h */
void task_switch(struct cpu * cpu, struct thread * new)
{
	struct thread * old = cpu_running(cpu);
	if (new != NULL)
		mutex_lock(&new->lock);
	if (old != NULL && old != new)
		mutex_lock(&old->lock);

	if (old != NULL) {
		if (old->state == TASK_RUNNING)
			panic("currnt task still running");
		cpu_save_state(cpu, &old->task_state);
	}

	if (new == NULL || new->state != TASK_READY)
		panic("no task to switch from [%zu:%zu]",
		      old->parent->pid, old->tid);

	if (old == NULL
	    || old->parent->pid != new->parent->pid
	    || old->tid != new->tid) {
		if (old == NULL || old->parent->pid != new->parent->pid)
			write_cr3(new->parent->cr3);
		cpu_set_task(cpu, new);
		cpu_load_state(cpu, &new->task_state);
		cpu_load_kstack(cpu, thread_kstack_ptr(new));
	}
	new->state = TASK_RUNNING;

	if (new != NULL)
		mutex_unlock(&new->lock);
	if (old != NULL && old != new)
		mutex_unlock(&old->lock);
}

/* public: task.h */
void task_set_state(struct thread * t, enum tstate state)
{
	mutex_lock(&t->lock);
	if (t->state != TASK_TERMINATED)
		t->state = state;
	mutex_unlock(&t->lock);
}

/* public: task.h */
enum tstate task_get_state(struct thread * t)
{
	return t->state;
}

/* public: task.h */
struct semaphore_list ** task_semaphores(struct thread * t)
{
	return &t->semaphores;
}

/* public: task.h */
union cr3 * process_cr3(struct process * proc)
{
	return (union cr3 *) &proc->cr3;
}

/* public: task.h */
struct table_vaddr ** process_vrtable(struct process * proc)
{
	return &proc->root_table_vaddr;
}
