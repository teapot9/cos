/**
 * @file spinlock.h
 * @brief Spinlock synchronization primitive
 */

#ifndef __SPINLOCK_H
#define __SPINLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>

/**
 * @brief Lock waiting by looping until available
 */
struct spinlock {
	atomic_bool val;
};

/**
 * @brief Create a new lock
 * @return struct spinlock
 */
#define spinlock_init() (struct spinlock) {.val = true}

/**
 * @brief Acquire the lock
 * @param s Spinlock
 */
void spinlock_lock(struct spinlock * s);

/**
 * @brief Release the lock
 * @param s Spinlock
 */
void spinlock_unlock(struct spinlock * s);

#ifdef __cplusplus
}
#endif
#endif // __SPINLOCK_H
