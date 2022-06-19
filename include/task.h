/**
 * @file task.h
 * @brief Process and threads management
 */

#ifndef __TASK_H
#define __TASK_H
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
#define THREAD_STRUCT_ALIGN KSTACK_ALIGN

/* Data structures */

struct cpu;
struct process;
struct thread;

/**
 * @brief Thread state
 */
enum tstate {
	TASK_READY,
	TASK_RUNNING,
	TASK_WAITING,
	TASK_TERMINATED,
};

/**
 * @brief Process
 */
struct process {
	/// Parent process
	struct process * parent;
	/// Process ID
	size_t pid;
	/// List of threads
	struct tlist * threads;
	/// CR3 register: memory space
	uword_t cr3;
	/// Last created thread ID
	size_t last_tid;
};

/**
 * @brief Thread
 */
struct thread {
	/// Internal lock
	struct semaphore lock;
	/// Parent process
	struct process * parent;
	/// Thread ID
	size_t tid;
	/// Current state
	_Atomic enum tstate state;
	/// Thread kernel stack
	uint8_t _aligned_(KSTACK_ALIGN) kstack[KSTACK_SIZE];
	/// CPU state (used for interrupts and syscalls)
	struct interrupt_frame task_state;
	/// Thread standard stack pointer
	void * stack;
	/// CPU running the thread
	struct cpu * running;
	/// List of semaphores acquired by the thread
	struct list semaphores;
#if IS_ENABLED(CONFIG_UBSAN)
	/// Recursion counter for ubsan
	int ubsan;
#endif
};

/* Current state information */

/**
 * @brief Get current thread
 * @return Running thread
 */
struct thread * thread_current(void);

/**
 * @brief Get current process
 * @return Running process
 */
struct process * process_current(void);

/* Task creation and destruction */

/**
 * @brief Create a new thread
 * @param proc Parent process
 * @param start Entry point
 * @return errno
 */
int thread_new(
	struct process * proc, void (* start)(void)
);

/**
 * @brief Create a new kernel thread
 * @param start Entry point
 * @return errno
 */
int kthread_new(void (*start)(void));

/**
 * @brief Create a new process
 * @param parent Parent process
 * @param start Entry point of the first thread
 * @return errno
 */
int process_new(struct process * parent, void (* start)(void));

/**
 * @brief Kill a thread
 * @param t Thread
 */
void thread_kill(struct thread * t);

/**
 * @brief Free a thread
 * @param t Thread
 */
void thread_delete(struct thread * t);

/* Task informations */

/**
 * @brief Get kernel stack pointer
 * @param thread Thread
 * @return SP register
 */
void * thread_kstack_ptr(struct thread * thread);

/**
 * @brief Get process by ID
 * @param pid Process ID
 * @return Process structure
 */
struct process * process_get(size_t pid);

/**
 * @brief Get thread by ID
 * @param p Process
 * @param tid Thread ID
 * @return Thread structure
 */
struct thread * thread_get(struct process * p, tid_t tid);

/**
 * @brief Get process CR3 register (memory space)
 * @param proc Process
 * @return CR3 register structure
 */
union cr3 * process_cr3(struct process * proc);

/* Task management */

/**
 * @brief Switch CPU to another task
 * @param cpu CPU to switch task
 * @param new New task
 */
void task_switch(struct cpu * cpu, struct thread * new);

/**
 * @brief Set task state
 * @param t Thread
 * @param state Task state
 */
void task_set_state(struct thread * t, enum tstate state);

/**
 * @brief Get task state
 * @param t Thread
 * @return Task state
 */
enum tstate task_get_state(struct thread * t);

#ifdef __cplusplus
}
#endif
#endif // __TASK_H
