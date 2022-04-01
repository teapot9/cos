#include <mm.h>
#include "kmm.h"

#include <string.h>
#include <stdint.h>

#include <print.h>
#include "vmm.h"

#ifndef EARLY_MEMORY_BUFFER_SIZE
#define EARLY_MEMORY_BUFFER_SIZE (128 * 1024 * 10)
#endif

static uint8_t early_memory_buffer[EARLY_MEMORY_BUFFER_SIZE];
static struct memory_block * first_block = NULL;

static void init_block(struct memory_block * block)
{
	block->next = NULL;
	block->first_free = (void *)
		((uint8_t *) block + sizeof(*block));
	block->size = sizeof(early_memory_buffer);
	block->first_free->next = NULL;
	block->first_free->size =
		block->size - sizeof(*block);
}

int kmm_early_init(void)
{
	first_block = (void *) early_memory_buffer;
	init_block(first_block);
	return 0;
}

static struct free_memory_info * find_free_mem_list(
	size_t size, struct memory_block * block
)
{
	struct free_memory_info * cur = block->first_free;
	struct free_memory_info ** pprev = &block->first_free;

	while (cur != NULL) {
		if (cur->size < size) {
			pprev = &cur->next;
			cur = cur->next;
			continue;
		}

		if (cur->size - size >= sizeof(struct free_memory_info)) {
			// Move start of free region to the right
			struct free_memory_info * new =
				(void *) ((uint8_t *) cur + size);

			new->next = cur->next;
			new->size = cur->size - size;
			*pprev = new;

			return cur;
		} else {
			// Remove free region
			*pprev = cur->next;
			return cur;
		}
	}

	return NULL;
}

static struct free_memory_info * find_free_mem_block(size_t size)
{
	struct memory_block * cur = first_block;
	struct memory_block ** pprec = &first_block;

	while (cur != NULL) {
		struct free_memory_info * free = find_free_mem_list(size, cur);
		if (free != NULL)
			return free;
		pprec = &cur->next;
		cur = cur->next;
	}

	struct mmap alloc = vmalloc(size);
	if (alloc.ptr == NULL)
		return NULL;

	struct memory_block * new = alloc.ptr;
	init_block(new);
	*pprec = new;
	return find_free_mem_list(size, new);
}

/* public: mm.h */
void * kmalloc(size_t size)
{
	struct free_memory_info * free_mem;

	size += sizeof(struct used_memory_header);
	if (size < sizeof(struct free_memory_info))
		size = sizeof(struct free_memory_info);

	if ((free_mem = find_free_mem_block(size)) == NULL) {
		pr_alert("Out of memory: Could not allocate %zd bytes\n", size);
		return NULL;
	}

	struct used_memory_header * used_mem = (void *) free_mem;

	free_mem = NULL;
	used_mem->start = (uint8_t *) used_mem + sizeof(*used_mem);
	used_mem->size = size;
	return used_mem->start;
}

static void merge_entry_list(struct free_memory_info * ptr, int count)
{
	for (int i = 0; i < count; i++) {
		if (ptr->next == (void *) ((uint8_t *) ptr + ptr->size)) {
			ptr->size += ptr->next->size;
			ptr->next = ptr->next->next;
		} else {
			ptr = ptr->next;
		}
	}
}

static void create_entry_list(
	struct used_memory_header * ptr, struct memory_block * block
)
{
	struct free_memory_info * cur = block->first_free;
	struct free_memory_info * prev = NULL;
	struct free_memory_info ** pprev = &block->first_free;
	struct free_memory_info * new = (void *) ptr;
	size_t size = ptr->size;

	while (cur != NULL && (void *) cur < (void *) ptr) {
		prev = cur;
		pprev = &cur->next;
		cur = cur->next;
	}

	*pprev = new;
	new->next = cur;
	new->size = size;

	merge_entry_list(prev, 2);
}

static void create_entry_block(
	struct used_memory_header * ptr
)
{
	struct memory_block * cur = first_block;

	while (cur != NULL && !((void *) cur <= (void *) ptr
	       && (uint8_t *) cur + cur->size > (uint8_t *) ptr))
		cur = cur->next;

	create_entry_list(ptr, cur);
}

/* public: mm.h */
void kfree(const void * ptr)
{
	if (ptr == NULL)
		return;

	struct used_memory_header * eptr =
		(void *) ((uint8_t *) ptr - sizeof(*eptr));
	create_entry_block(eptr);
}

/* public: mm.h */
void * krealloc(void * oldptr, size_t newsize)
{
	if (oldptr == NULL)
		return kmalloc(newsize);

	struct used_memory_header * oldeptr =
		(void *) ((uint8_t *) oldptr - sizeof(*oldeptr));
	void * newptr = kmalloc(newsize);

	if (newptr == NULL) {
		pr_alert("Out of memory: cannot realloc %zs to %zs bytes\n",
			 oldeptr->size, newsize);
		return NULL;
	}

	memcpy(newptr, oldptr, oldeptr->size);
	kfree(oldeptr->start);
	return newptr;
}
