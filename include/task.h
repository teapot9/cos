#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#include <isr.h>

struct process;
struct thread;

typedef size_t pid_t;
typedef size_t tid_t;
struct tid {
	pid_t pid;
	tid_t tid;
};

void * thread_kstack_ptr(struct thread * thread);
int thread_new(
	struct process * proc, void (* start)(void)
);
int process_new(struct process * parent, void (* start)(void));
struct process * process_get(size_t pid);
struct thread * thread_get(struct process * p, tid_t tid);
void task_switch(struct process * new_proc, struct thread * new_thread,
                 struct interrupt_frame * state);

#endif // TASK_H
