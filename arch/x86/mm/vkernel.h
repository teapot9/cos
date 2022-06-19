#ifndef _X86_MM_VKERNEL_H
#define _X86_MM_VKERNEL_H

#include "paging.h"

int vkernel_map_kmem(union pml4e * pml4);
int vkernel_init(void);
int vkernel_identity_paging(void);

#endif // _X86_MM_VKERNEL_H
