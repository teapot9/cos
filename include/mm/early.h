#ifndef MM_EARLY_H
#define MM_EARLY_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include <mm.h>

extern uint64_t kernel_cr3;
int vmm_init(void);
void vmm_enable_paging(void);

#ifdef __cplusplus
}
#endif
#endif // MM_EARLY_H
