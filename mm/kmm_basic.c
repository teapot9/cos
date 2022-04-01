#include <mm.h>
#include "kmm.h"

#include <string.h>
#include <stdint.h>

#include <print.h>

#ifndef EARLY_MEMORY_BUFFER_SIZE
#define EARLY_MEMORY_BUFFER_SIZE (128 * 1024 * 10)
#endif

struct free_memory_info {
	struct free_memory_info * prev;
	struct free_memory_info * next;
	size_t size;
};

struct used_memory_header {
	size_t size;
	void * start;
};

static uint8_t early_memory_buffer[EARLY_MEMORY_BUFFER_SIZE];
#if 0
static uint8_t * mem_base = NULL;
static uint8_t * mem_free_start = NULL;
static uint8_t * mem_end = NULL;
#endif
static struct free_memory_info * mem_free_list = NULL;

int kmm_early_init(void)
{
#if 0
	mem_base = early_memory_buffer;
	mem_free_start = mem_base;
	mem_end = mem_base + sizeof(early_memory_buffer);
	mem_free_list = NULL;
#endif
	mem_free_list = (void *) early_memory_buffer;
	mem_free_list->prev = NULL;
	mem_free_list->next = NULL;
	mem_free_list->size = sizeof(early_memory_buffer);
	return 0;
}

struct free_memory_info * find_free_mem_list(size_t size)
{
	struct free_memory_info * cur = mem_free_list;

	while (cur != NULL) {
		if (cur->size < size) {
			cur = cur->next;
			continue;
		}

		if (cur->size - size >= sizeof(struct free_memory_info)) {
			// Move start of free region to the right
			struct free_memory_info * new =
				(void *) ((uint8_t *) cur + size);

			new->prev = cur->prev;
			new->next = cur->next;
			new->size = cur->size - size;
			if (cur->prev != NULL)
				new->prev->next = new;
			else
				mem_free_list = new;
			if (cur->next != NULL)
				new->next->prev = new;

			return cur;
		} else {
			// Remove free region
			if (cur->prev != NULL)
				cur->prev->next = cur->next;
			else
				mem_free_list = cur->next;
			if (cur->next != NULL)
				cur->next->prev = cur->prev;
			return cur;
		}
	}

	return NULL;
}

#if 0
struct free_memory_info * find_free_mem_contig(size_t size)
{
	if (mem_end < mem_free_start + size)
		return NULL;

	void * ret = mem_free_start;

	mem_free_start += size;
	return ret;
}
#endif

void * kmalloc(size_t size)
{
	struct free_memory_info * free_mem;

	size += sizeof(struct used_memory_header);
	if (size < sizeof(struct free_memory_info))
		size = sizeof(struct free_memory_info);

	if ((free_mem = find_free_mem_list(size)) != NULL) {
#if 0
		if ((free_mem = find_free_mem_contig(size)) == NULL) {
			pr_alert("Out of memory: Could not allocate %zd bytes",
			         size);
			return NULL;
		}
#endif
		struct used_memory_header * used_mem = (void *) free_mem;

		free_mem = NULL;
		used_mem->size = size;
		used_mem->start = (uint8_t *) used_mem + sizeof(*used_mem);
		return used_mem->start;
	}

	pr_alert("Out of memory: Could not allocate %zd bytes", size);
	return NULL;
}

struct free_memory_info * create_list_entry(struct used_memory_header * ptr)
{
	struct free_memory_info * cur = mem_free_list;
	struct free_memory_info * prev = NULL;
	struct free_memory_info * new = (void *) ptr;
	size_t size = ptr->size;

	while (cur != NULL && (void *) cur < (void *) ptr) {
		prev = cur;
		cur = cur->next;
	}

	if (cur != NULL)
		cur->prev = new;
	if (prev != NULL)
		prev->next = new;
	else
		mem_free_list = new;
	new->prev = prev;
	new->next = cur;
	new->size = size;

	return new;
}

void merge_list_entry(struct free_memory_info * ptr)
{
#if 0
	if (ptr->next == NULL
	    && (uint8_t *) ptr + ptr->size >= mem_free_start) {
		if (ptr->prev != NULL)
			ptr->prev->next = NULL;
		else
			mem_free_list = NULL;
		mem_free_start = (uint8_t *) ptr;
		return;
	}
#endif

	if (ptr->next == (void *) ((uint8_t *) ptr + ptr->size)) {
		ptr->size += ptr->next->size;
		ptr->next = ptr->next->next;
		ptr->next->prev = ptr;
	}
	if ((uint8_t *) ptr->prev + ptr->prev->size == (uint8_t *) ptr) {
		ptr->size += ptr->prev->size;
		ptr->prev = ptr->prev->prev;
		ptr->prev->next = ptr;
	}
}

void kfree(const void * ptr)
{
	if (ptr == NULL)
		return;

	struct used_memory_header * eptr =
		(void *) ((uint8_t *) ptr - sizeof(*eptr));
	struct free_memory_info * fptr = create_list_entry(eptr);

	merge_list_entry(fptr);
}

void * krealloc(void * oldptr, size_t newsize)
{
	struct used_memory_header * oldeptr =
		(void *) ((uint8_t *) oldptr - sizeof(*oldeptr));
	void * newptr = kmalloc(newsize);

	if (newptr == NULL) {
		pr_alert("Out of memory: cannot realloc %zs to %zs bytes",
			 oldeptr->size, newsize);
		return NULL;
	}

	memcpy(newptr, oldptr, oldeptr->size);
	kfree(oldeptr->start);
	return newptr;
}
