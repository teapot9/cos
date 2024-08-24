#include "cxx.h"

#include <stdatomic.h>

#include <cpp.h>
#include <panic.h>


/* Virtual methods */

noreturn void __cxa_pure_virtual()
{
	panic("tried to call pure virtual function");
}

noreturn void __cxa_deleted_virtual()
{
	panic("tried to call deleted virtual function");
}


/* Static objects */

/* guard object:
 *  - byte 1: init done (bool)
 *  - byte 2: locked (bool)
 */

typedef _Atomic int64_t guard_t;

int __cxa_guard_acquire(int64_t * guard_object)
{
	guard_t * guard = (void *) guard_object;
	int64_t expected;
	int64_t desired;
	do {
		expected = 0x0000;
		desired = 0x0100;
		if (*guard == 1)
			return 0;
	} while (!atomic_compare_exchange_weak(guard, &expected, desired));
	return 1;
}

void __cxa_guard_release(int64_t * guard_object)
{
	guard_t * guard = (void *) guard_object;
	*guard = 0x0001;
}

void __cxa_guard_abort(int64_t * guard_object)
{
	guard_t * guard = (void *) guard_object;
	*guard = 0x0000;
}


/* Global variables */

void * __dso_handle = (void *) &__dso_handle;

int __cxa_atexit(_unused_ void (* f)(void *), _unused_ void * p, _unused_ void * d)
{
	return 0; // atexit functions are never called
}

void __cxa_finalize(_unused_ void * d)
{
	// noop
}
