#ifndef LOCK_H
#define LOCK_H

#include <stdatomic.h>
#include <stdbool.h>

struct thread_list;
struct semaphore_list;

struct semaphore {
	atomic_uint val;
	struct thread_list * threads;
};

struct spinlock {
	atomic_bool val;
};

#define semaphore_init(n) (struct semaphore) {.val = (n), .threads = NULL}
//void semaphore_init(struct semaphore * s, unsigned n);
void semaphore_lock(struct semaphore * s);
void semaphore_unlock(struct semaphore * s);
void semaphore_unlock_all(struct semaphore_list * l);

#define mutex_init() semaphore_init(1)
//#define mutex_init() (struct semaphore) {.val = 1, .threads = NULL}
//void mutex_init(struct semaphore * s);
void mutex_lock(struct semaphore * s);
void mutex_unlock(struct semaphore * s);
void mutex_unlock_all(struct semaphore_list * l);

#define spinlock_init() (struct spinlock) {.val = true}
void spinlock_lock(struct spinlock * s);
void spinlock_unlock(struct spinlock * s);

#define nblock_init() spinlock_init()
bool nblock_lock(struct spinlock * s);
void nblock_unlock(struct spinlock * s);

#ifdef BOOTLOADER
#define semaphore_lock(x)
#define semaphore_unlock(x)
#define semaphore_unlock_all(x)
#define mutex_lock(x)
#define mutex_unlock(x)
#define mutex_unlock_all(x)
#define spinlock_lock(x)
#define spinlock_unlock(x)
#define nblock_lock(x)
#define nblock_unlock(x)
#endif

#endif // LOCK_H
