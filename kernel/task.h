#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdatomic.h>
#include <stdint.h>
#include <stddef.h>
#include <cpp.h>
#include <isr.h>
#include <alloc.h>
#include <task.h>
#include <lock.h>
#include <kconfig.h>

#define MAX_PROC_CNT MAX_PID + 1
#define USTACK_SIZE (1024*100)
#define USTACK_ALIGN 0x10

struct cpu;
struct semaphore_list;

struct tlist {
	struct thread _aligned_(THREAD_STRUCT_ALIGN) thread;
	struct tlist * next;
};

extern struct process processes[MAX_PROC_CNT];
struct process * process_next(size_t pid);
struct tlist * thread_lget(struct process * p, tid_t tid);

#endif // _KERNEL_TASK_H
