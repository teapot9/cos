#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdatomic.h>
#include <stdint.h>
#include <stddef.h>
#include <cpp.h>
#include <isr.h>
#include <mm.h>
#include <task.h>
#include <lock.h>

#define MAX_PID 1024
#define MAX_PROC_CNT MAX_PID + 1
#define KSTACK_SIZE CONFIG_KERNEL_FRAME_SIZE
#define KSTACK_ALIGN 0x10
#define USTACK_SIZE (1024*100)
#define USTACK_ALIGN 0x10

struct cpu;
struct semaphore_list;

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
	struct memblock stack;
	struct cpu * running;
	struct semaphore_list * semaphores;
};

struct tlist {
	struct thread thread;
	struct tlist * next;
};

struct process * process_next(size_t pid);
struct tlist * thread_lget(struct process * p, tid_t tid);

#endif // KERNEL_TASK_H
