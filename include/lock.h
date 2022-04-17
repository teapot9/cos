#ifndef LOCK_H
#define LOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <spinlock.h>
#include <list.h>

struct thread;

struct semaphore {
	atomic_uint val;
	struct list waiting;
};

#define semaphore_init(n) (struct semaphore) \
	{.val = (n), .waiting = list_new()}
//void semaphore_init(struct semaphore * s, unsigned n);
void semaphore_lock(struct semaphore * s);
void semaphore_unlock(struct semaphore * s);
void semaphore_unlock_all(struct thread * t);

#define mutex_init() semaphore_init(1)
//#define mutex_init() (struct semaphore) {.val = 1, .threads = NULL}
//void mutex_init(struct semaphore * s);
void mutex_lock(struct semaphore * s);
void mutex_unlock(struct semaphore * s);
void mutex_unlock_all(struct thread * t);

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
#endif

#ifdef __cplusplus
}
#endif
#endif // LOCK_H
