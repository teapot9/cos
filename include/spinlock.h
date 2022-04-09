#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdatomic.h>

struct spinlock {
	atomic_bool val;
};

#define spinlock_init() (struct spinlock) {.val = true}
void spinlock_lock(struct spinlock * s);
void spinlock_unlock(struct spinlock * s);

#endif // SPINLOCK_H
