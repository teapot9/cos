/**
 * @file lock.h
 * @brief Synchronization primitives
 */

#ifndef __LOCK_H
#define __LOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <spinlock.h>
#include <list.h>

struct thread;

/**
 * @brief Lock allowing one or more simultaneous owner
 */
struct semaphore {
	/// Number of available users
	atomic_uint val;
	/// Threads waiting list
	struct list waiting;
};

/**
 * @brief Create a new semaphore
 * @param n Number of allowed simultaneous owner
 * @return struct semaphore
 */
#define semaphore_init(n) (struct semaphore) \
	{.val = (n), .waiting = list_new()}

/**
 * @brief Lock a semaphore (wait if necessary)
 * @param s Semaphore
 */
void semaphore_lock(struct semaphore * s);

/**
 * @brief Unlock a semaphore
 * @param s Semaphore
 */
void semaphore_unlock(struct semaphore * s);

/**
 * @brief Unlock all semaphore locked by a thread
 * @param t Thread
 */
void semaphore_unlock_all(struct thread * t);

/**
 * @brief Create a new mutex
 * @return struct semaphore
 */
#define mutex_init() semaphore_init(1)

/**
 * @brief Lock a mutex (wait if necessary)
 * @param s Mutex
 */
void mutex_lock(struct semaphore * s);

/**
 * @brief Unlock a mutex
 * @param s Mutex
 */
void mutex_unlock(struct semaphore * s);

/**
 * @brief Unlock all mutexes locked by a thread
 * @param t Thread
 */
void mutex_unlock_all(struct thread * t);

/**
 * @brief Create a non-blocking lock
 * @return struct spinlock
 */
#define nblock_init() spinlock_init()

/**
 * @brief Lock a non-blocking lock
 * @param s Lock
 * @return True if locking was successful, false otherwise
 */
bool nblock_lock(struct spinlock * s);

/**
 * @brief Unlock a non-blocking lock
 * @param s Lock
 */
void nblock_unlock(struct spinlock * s);

#ifdef BOOT
/* Bootloader run in a single thread */
#define semaphore_lock(x)
#define semaphore_unlock(x)
#define semaphore_unlock_all(x)
#define mutex_lock(x)
#define mutex_unlock(x)
#define mutex_unlock_all(x)
#endif

#ifdef __cplusplus
}
#endif
#endif // __LOCK_H
