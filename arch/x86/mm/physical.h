#ifndef _X86_MM_PHYSICAL_H
#define _X86_MM_PHYSICAL_H

#include <mm/physical.h>

union pml4e;

extern void * const physical_map;
int physical_init(union pml4e * pml4);

#endif // _X86_MM_PHYSICAL_H
