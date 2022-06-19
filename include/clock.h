/**
 * @file clock.h
 * @brief Clock devices and timers
 */

#ifndef __CLOCK_H
#define __CLOCK_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief Create a new timer
 * @param msec Number of milliseconds to wait
 * @param callback Function to call every `msec`
 * @param nbcall Number of times to call `callback` before deleting the timer,
 * 	0 for unlimited
 * @return errno
 */
int timer_new(size_t msec, void (*callback)(void), size_t nbcall);

/**
 * @brief Delete a timer
 * @param callback Callback of the timer to delete
 * @return errno
 */
int timer_del(void (*callback)(void));

#ifdef __cplusplus
}
#endif
#endif // __CLOCK_H
