#ifndef MM_BLOCK_H
#define MM_BLOCK_H

#include <stddef.h>
#include <stdbool.h>

// Sorted linked list of memory blocks
struct memblock_list {
	void * addr;
	void * mapping;
	size_t size;
	struct memblock_list * next;
};

struct memblock_list ** memblock_search_size(struct memblock_list ** memblock,
                                        size_t size, size_t align);

struct memblock_list ** memblock_search(struct memblock_list ** memblock,
                                   void * addr, size_t size, bool full);

bool memblock_exists(struct memblock_list ** memblock,
		     void * addr, size_t size, bool full);

int memblock_add(struct memblock_list ** memblock,
                 void * addr, size_t size, bool strict);

int memblock_rem(struct memblock_list ** memblock,
                 void * addr, size_t size, bool strict);

void memblock_free(struct memblock_list ** memblock);

#endif // MM_BLOCK_H
