#ifndef MM_BLOCK_H
#define MM_BLOCK_H

#include <stddef.h>
#include <stdbool.h>

// Sorted linked list of memory blocks
struct memblock {
	void * addr;
	size_t size;
	struct memblock * next;
};

struct memblock ** memblock_search_size(struct memblock ** memblock,
                                        size_t size, size_t align);

struct memblock ** memblock_search(struct memblock ** memblock,
                                   void * addr, size_t size, bool full);

int memblock_add(struct memblock ** memblock,
                 void * addr, size_t size, bool strict);

int memblock_rem(struct memblock ** memblock,
                 void * addr, size_t size, bool strict);

#endif // MM_BLOCK_H
