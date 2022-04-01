#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#include <isr.h>

struct cpu;
struct process;
struct thread;

typedef size_t pid_t;
typedef size_t tid_t;

enum tstate {
	TASK_READY,
	TASK_RUNNING,
	TASK_WAITING,
	TASK_TERMINATED,
};

int thread_new(
	struct process * proc, void (* start)(void)
);
int kthread_new(void (*start)(void));
int process_new(struct process * parent, void (* start)(void));
void thread_kill(struct thread * t);
void thread_delete(struct thread * t);

void * thread_kstack_ptr(struct thread * thread);
struct process * process_get(size_t pid);
struct thread * thread_get(struct process * p, tid_t tid);
void task_switch(struct cpu * cpu, struct thread * new);
void task_set_state(struct thread * t, enum tstate state);
enum tstate task_get_state(struct thread * t);
struct semaphore_list ** task_semaphores(struct thread * t);

union cr3 * process_cr3(struct process * proc);
struct table_vaddr ** process_vrtable(struct process * proc);

#endif // TASK_H
