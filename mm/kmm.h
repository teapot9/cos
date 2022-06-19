#ifndef _MM_KMM_H
#define _MM_KMM_H

#include <stddef.h>
#include <assert.h>
#include <stdint.h>

#define EARLY_MEMORY_BUFFER_SIZE (128 * 1024 * 10)
#define DEFAULT_MEMORY_BLOCK_SIZE (1024 * 1024)

struct memory_block {
	struct memory_block * next;
	struct free_memory_header * first_free;
	size_t size;
};

struct free_memory_header {
	struct free_memory_header * next;
	size_t size;
};

struct used_memory_header {
	size_t size_used;
	uint8_t size_left;
	uint8_t size_right;
};
static_assert((1 << 8) > sizeof(struct free_memory_header),
	      "free_memory_info is too big");
static_assert((1 << 8) > sizeof(struct used_memory_header),
              "used_memory_header is too big");

#endif // _MM_KMM_H
