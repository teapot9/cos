#ifndef MM_KMM_H
#define MM_KMM_H

#include <stddef.h>

struct memory_block {
	struct memory_block * next;
	struct free_memory_info * first_free;
	size_t size;
};

struct free_memory_info {
	struct free_memory_info * next;
	size_t size;
};

struct used_memory_header {
	void * start;
	size_t size;
};

#endif // MM_KMM_H
