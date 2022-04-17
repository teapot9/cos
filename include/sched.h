#ifndef SCHED_H
#define SCHED_H
#ifdef __cplusplus
extern "C" {
#endif

struct thread;

int sched_next(struct thread ** next, struct thread * cur);
int sched_enable(void);
void sched_disable(void);
void sched_yield(void);

#ifdef __cplusplus
}
#endif
#endif // SCHED_H
