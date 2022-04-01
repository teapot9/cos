#ifndef MEMMAP_H
#define MEMMAP_H

#include <stddef.h>

#include <mm/block.h>

struct memmap_list;

int memmap_add(struct memmap_list ** map, void * addr1, void * addr2, size_t size);

void * memmap_translate(struct memmap_list ** map, void * addr);

#endif // MEMMAP_H
