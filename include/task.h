#ifndef TASK_H
#define TASK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <isr.h>
#include <lock.h>
#include <list.h>
#include <types.h>

#define MAX_PID 1024
#define KSTACK_SIZE CONFIG_KERNEL_FRAME_SIZE
#define KSTACK_ALIGN 0x10

struct cpu;
struct process;
struct thread;

enum tstate {
	TASK_READY,
	TASK_RUNNING,
	TASK_WAITING,
	TASK_TERMINATED,
};

struct process {
	struct process * parent;
	size_t pid;
	struct tlist * threads;
	uword_t cr3;
	size_t last_tid;
};

struct thread {
	struct semaphore lock;
	struct process * parent;
	size_t tid;
	_Atomic enum tstate state;
	uint8_t kstack[KSTACK_SIZE + KSTACK_ALIGN];
	struct interrupt_frame task_state;
	void * stack;
	struct cpu * running;
	struct list semaphores;
#if IS_ENABLED(CONFIG_UBSAN)
	int ubsan;
#endif
};

struct thread * thread_current(void);
struct process * process_current(void);

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

union cr3 * process_cr3(struct process * proc);

#ifdef __cplusplus
}
#endif
#endif // TASK_H
