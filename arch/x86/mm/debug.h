#ifndef MM_DEBUG_H
#define MM_DEBUG_H

#include "page.h"

void dump_vmm(union cr3 * cr3);
void dump_vmm_current(void);

#endif
