/**
 * @file sched.h
 * @brief Task scheduler
 */

#ifndef SCHED_H
#define SCHED_H
#ifdef __cplusplus
extern "C" {
#endif

struct thread;

/**
 * @brief Task switch
 * @param next New thread
 * @param cur Current thread
 * @return errno
 */
int sched_next(struct thread ** next, struct thread * cur);

/**
 * @brief Enable scheduler
 * @return errno
 */
int sched_enable(void);

/**
 * @brief Disable scheduler
 */
void sched_disable(void);

/**
 * @brief Current task switch
 */
void sched_yield(void);

#ifdef __cplusplus
}
#endif
#endif // SCHED_H
