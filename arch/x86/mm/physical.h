#ifndef MM_PHYSICAL_H
#define MM_PHYSICAL_H

#include <mm/physical.h>

union pml4e;

extern void * const physical_map;
int physical_init(union pml4e * pml4);

#endif // MM_PHYSICAL_H
