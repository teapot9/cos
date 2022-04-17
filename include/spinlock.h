#ifndef SPINLOCK_H
#define SPINLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>

struct spinlock {
	atomic_bool val;
};

#define spinlock_init() (struct spinlock) {.val = true}
void spinlock_lock(struct spinlock * s);
void spinlock_unlock(struct spinlock * s);

#ifdef __cplusplus
}
#endif
#endif // SPINLOCK_H
