#ifndef LIB_MEMMAP_H
#define LIB_MEMMAP_H

#include <stddef.h>

struct memmap_list {
	void * addr1;
	void * addr2;
	size_t size;
	struct memmap_list * next;
};

#endif // LIB_MEMMAP_H
