/**
 * @file power.h
 * @brief System power management
 */

#ifndef __POWER_H
#define __POWER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdnoreturn.h>

/**
 * @brief Shutdown the CPU
 *
 * This function never returns.
 */
// TODO: this should stop scheduler and interrupts
noreturn void halt(void);

#ifdef __cplusplus
}
#endif
#endif // __POWER_H
