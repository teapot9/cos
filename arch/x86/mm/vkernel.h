#ifndef MM_VKERNEL_H
#define MM_VKERNEL_H

#include "paging.h"

int vkernel_map_kmem(union pml4e * pml4);
int vkernel_init(void);
int vkernel_identity_paging(void);

#endif // MM_VKERNEL_H
