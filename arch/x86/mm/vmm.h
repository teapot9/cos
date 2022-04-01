#ifndef MM_VMM_H
#define MM_VMM_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mm.h>

#define USER_SPACE_START (void *) 0x0
#define USER_SPACE_END (void *) 0x00007fffffffffff
#define KERNEL_SPACE_START (void *) 0xffff800000000000
#define KERNEL_SPACE_END (void *) 0xffffffffffffffff
#define KERNEL_IMAGE_BASE (void *) 0xffffffff80000000

void vmm_flush_tlb(void);
void vmm_full_flush_tlb(void);
extern bool vmm_paging_not_identity;

#endif // MM_VMM_H
