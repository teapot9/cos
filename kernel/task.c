#include <setup.h>
#include <task.h>
#include "task.h"

#include <print.h>
#include <cpu.h>
#include <panic.h>
#include <mm.h>
#include <asm/cpu.h>
#include <gdt.h>
#include <errno.h>
#include <mm/helper.h>

#define kstack_aligned(x) ((uint8_t *) aligned(x, KSTACK_ALIGN) + KSTACK_SIZE)

struct process processes[MAX_PROC_CNT];
uint8_t used[MAX_PROC_CNT] = {0};

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

static inline void save_state(struct thread * t, struct interrupt_frame * state)
{
	pr_debug("Saving [%zu:%zu] state\n", t->parent->pid, t->tid);
	t->task_state = *state;
}

static inline void load_state(struct thread * t, struct interrupt_frame * state)
{
	pr_debug("Saving [%zu:%zu] state\n", t->parent->pid, t->tid);
	*state = t->task_state;
}

int thread_init(
	struct thread * thread, struct process * parent, void (* start)(void)
)
{
	if (thread == NULL || parent == NULL || start == NULL)
		return -EINVAL;

	thread->parent = parent;
	thread->tid = new_tid(parent);
	thread->state = TASK_READY;

	void * stack;
	if (is_kernel_process(parent)) {
		thread->stack = vmalloc(KSTACK_SIZE + KSTACK_ALIGN);
		if (thread->stack.addr == NULL)
			return -ENOMEM;
		stack = kstack_aligned(thread->stack.addr);
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

int process_init(
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

	uword_t cr3;
	if (is_kernel_process(proc)) {
		cr3 = kcr3();
	} else {
		// Should form kernel CR3
		return -ENOTSUP;
	}
	proc->cr3 = cr3;
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
int process_new(struct process * parent, void (* start)(void))
{
	int err;

	pid_t pid = new_pid();
	struct process * proc = process_get(pid);
	if (proc == NULL)
		return -EAGAIN;

	err = process_init(pid, parent);
	if (err)
		return err;

	err = thread_new(proc, start);
	if (err < 0)
		return err;
	if (err > 0)
		return -EINVAL;

	return proc->pid;
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
void task_switch(struct process * new_proc, struct thread * new_thread,
                 struct interrupt_frame * state)
{
	const struct device * cpu = cpu_current();
	struct process * old_proc = cpu_proc(cpu);
	struct thread * old_thread = cpu_thread(cpu);

	if (old_proc != NULL && old_thread != NULL) {
		save_state(old_thread, state);
		if (old_thread->state == TASK_RUNNING)
			panic("Switching from still running task\n");
	}

	if (new_proc == NULL || new_thread == NULL
	    || new_thread->state != TASK_READY)
		panic("no task to switch from [%zu:%zu]",
		      old_proc->pid, old_thread->tid);

	cpu_update_task(cpu, new_proc, new_thread);
	if (old_proc == NULL
	    || old_proc->pid != new_proc->pid
	    || old_thread->tid != new_thread->tid) {
		if (old_proc == NULL || old_proc->pid != new_proc->pid)
			write_cr3(new_proc->cr3);
		load_state(new_thread, state);
		cpu_set_kstack(cpu, thread_kstack_ptr(new_thread));
		new_thread->state = TASK_RUNNING;
	}
}
