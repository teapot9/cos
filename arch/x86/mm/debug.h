#ifndef _X86_MM_DEBUG_H
#define _X86_MM_DEBUG_H

#include "paging.h"

void dump_vmm(union cr3 * cr3);
void dump_vmm_current(void);

#endif
