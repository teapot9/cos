#define pr_fmt(fmt) "kmm: " fmt

#include <mm.h>
#include "kmm.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <cpu.h>
#include <lock.h>
#include <print.h>
#include <mm.h>
#include <cpp.h>
#include <mm/helper.h>

/*
 * used memory block:
 * +--------+--------+------------------------+--------+
 * |        |        |                        |        |
 * | unused | header |  actual used memory    | unused |
 * |        |        |                        |        |
 * +--------+--------+------------------------+--------+
 *  <---------------> <----------------------> <------>
 *      size_left              size_used      size_right
 * total block size = size_left + size_used + size_right
 *
 * Note on unused blocks:
 *  - Unused space on right: caused by allocatted size being smaller than
 *    the size of the free memory block.
 *  - Unused space on left: caused by alignment requirement leaving space on
 *    the left.
 * If those unused blocks are large enough, they are converted into free memory
 * blocks, otherwise they are stored in the used memory block.
 *
 * free memory block:
 * +--------+------------------------+
 * |        |                        |
 * | header |  actual free memory    |
 * |        |                        |
 * +--------+------------------------+
 *  <------------------------------->
 *                size
 * total block size = size
 */

static uint8_t early_memory_buffer[EARLY_MEMORY_BUFFER_SIZE];
static struct memory_block * first_block = NULL;
static struct spinlock kmm_lock = spinlock_init();
#define lock() do { \
	disable_interrupts(); \
	spinlock_lock(&kmm_lock); \
} while (0)
#define unlock() do { \
	spinlock_unlock(&kmm_lock); \
	restore_interrupts(); \
} while (0)
#define maybe_kmm_init() do { \
	if (unlikely(!kmm_is_init())) \
		kmm_early_init(); \
} while (0)

/* Check if memory manager has been initialized */
static inline bool kmm_is_init(void)
{
	return first_block != NULL;
}

/* Initialize a new memory block */
static void init_block(struct memory_block * block, size_t size)
{
	memset(block, 'A', size);
	block->next = NULL;
	block->size = size;
	block->first_free = (void *) ((uint8_t *) block + sizeof(*block));
	block->first_free->next = NULL;
	block->first_free->size =
		block->size - sizeof(*block);
}

/* Initialize kernel memory manager */
static void kmm_early_init(void)
{
	if (kmm_is_init())
		return;
	first_block = (void *) early_memory_buffer;
	init_block(first_block, sizeof(early_memory_buffer));
}

/* Find the first free memory block located at or after *address* */
static struct free_memory_header * find_last_free_block_before(
	struct free_memory_header * start, void * address
)
{
	if (start == NULL)
		return NULL; // invalid

	struct free_memory_header * prev = NULL;
	struct free_memory_header * cur = start;

	while (cur != NULL) {
		if ((void *) cur >= address)
			return prev;
		prev = cur;
		cur = cur->next;
	}
	return NULL;
}

/* Find memory block owner of an address */
static struct memory_block * find_memory_block_owner(void * address)
{
	for (struct memory_block * cur = first_block;
	     cur != NULL; cur = cur->next) {
		if (is_inside(cur, cur->size, address, 0))
			return cur;
	}
	return NULL;
}

/* Test if the *block* can contain *size* bytes aligned to *align* */
static bool can_contain(
	struct free_memory_header * block, size_t size, size_t align
)
{
	uint8_t * block_start = (void *) block;
	uintptr_t effective_start = (uintptr_t) aligned_up(
		block_start + sizeof(struct free_memory_header), align
	);
	uintptr_t effective_end = (uintptr_t) (block_start + block->size);
	return effective_end >= effective_start
		&& effective_end - effective_start >= size;
}

/* Find a free block that can contain *size* bytes aligned to *align* */
static struct free_memory_header ** find_free_block_size(
	struct free_memory_header ** start, size_t size, size_t align
)
{
	if (start == NULL)
		return NULL; // invalid

	struct free_memory_header ** prev = start;
	struct free_memory_header * cur = *prev;

	while (cur != NULL) {
		if (can_contain(cur, size, align))
			return prev;
		prev = &cur->next;
		cur = cur->next;
	}
	return prev;
}

/* Find free memory in any memory blocks of the kmm */
static struct free_memory_header ** find_free_block_size_all(
	size_t size, size_t align
)
{
	for (struct memory_block * cur = first_block;
	     cur != NULL; cur = cur->next) {
		struct free_memory_header ** found =
			find_free_block_size(&cur->first_free, size, align);
		if (found != NULL && *found != NULL)
			return found;
	}
	return NULL;
}

/* Value of used.size_left (taking align into account) */
static size_t calc_size_left(
	struct free_memory_header * block, size_t align
)
{
	uint8_t * block_start = (void *) block;
	uint8_t * effective_start = aligned_up(
		block_start + sizeof(struct free_memory_header), align
	);
	return effective_start - block_start;
}

/* Value of used.size_right */
static size_t calc_size_right(
	struct free_memory_header * block, size_t size, size_t size_left
)
{
	return block->size - size - size_left;
}

/* Merge with next free block if possible */
bool maybe_merge_free(struct free_memory_header * block)
{
	uintptr_t cur_start = (uintptr_t) block;
	uintptr_t next_start = (uintptr_t) block->next;
	if (cur_start + block->size == next_start) {
		block->size += block->next->size;
		block->next = block->next->next;
		return true;
	}
	return false;
}

/* Allocate memory within a free memory block pointed by *block_ptr* */
static int alloc_free_block(
	struct free_memory_header ** block_ptr, size_t size, size_t align,
	struct used_memory_header ** dest
)
{
	if (block_ptr == NULL || *block_ptr == NULL || dest == NULL)
		return -EINVAL;
	if (!can_contain(*block_ptr, size, align))
		return -ENOMEM;
	struct free_memory_header free_ = **block_ptr; // copy
	// pointer to the free block we are working on
	struct free_memory_header ** cur_free = block_ptr;
	// free block next to the current one
	struct free_memory_header * next_free = free_.next;
	uintptr_t block_start = (uintptr_t) *cur_free; // start of free memory

	/* Poison free block */
	memset((void *) block_start, 'A', free_.size);

	*cur_free = free_.next; // make this memory block unusable (temporary)

	size_t size_left = calc_size_left(&free_, align);
	size_t size_right = calc_size_right(&free_, size, size_left);
	struct used_memory_header * used =
		(void *) (block_start + size_left - sizeof(*used));
	if (free_.size != size_left + size_right + size)
		return -EINVAL;

	/* If large enough, try to create free memory block on left */
	if (size_left >= sizeof(struct free_memory_header)
	    + sizeof(struct used_memory_header)) {
		struct free_memory_header * new_free = (void *) block_start;
		new_free->next = next_free;
		new_free->size = size_left;
		*cur_free = new_free;
		cur_free = &new_free->next;
		used->size_left = 0;
	} else {
		used->size_left = size_left;
	}

	/* If large enough, try to create free memory block on right */
	size_t align_diff_right = align_diff_up(
		(void *) (block_start + size_left + size),
		DEFAULT_MEMORY_ALLOC_ALIGN
	); // we want the new free block to be aligned in memory
	if (size_right
	    >= sizeof(struct free_memory_header) + align_diff_right) {
		struct free_memory_header * new_free = (void *)
			(block_start + size_left + size + align_diff_right);
		new_free->next = next_free;
		new_free->size = size_right - align_diff_right;
		*cur_free = new_free;
		cur_free = &new_free->next;
		used->size_right = align_diff_right;
		maybe_merge_free(new_free);
	} else {
		used->size_right = size_right;
	}

	used->size_used = size;
	*dest = used;
	return 0;
}

/* Free a used memory block */
static int free_used_block(
	struct used_memory_header * block
)
{
	if (block == NULL)
		return -EINVAL;

	uintptr_t block_start = (uintptr_t) block;
	struct used_memory_header used = *block; // copy

	/* Poison block */
	memset((void *) block_start, 'A',
	       used.size_left + used.size_used + used.size_right);

	struct free_memory_header * new_free =
		(void *) (block_start + sizeof(used) - used.size_left);

	/* Set size */
	new_free->size = used.size_left + used.size_used + used.size_right;

	struct memory_block * memblock =
		find_memory_block_owner((void *) block_start);
	if (memblock == NULL)
		return -EINVAL;
	struct free_memory_header * prev_free = find_last_free_block_before(
		memblock->first_free, (void *) block_start
	);

	/* Add to linked list */
	if (prev_free == NULL) {
		new_free->next = memblock->first_free;
		memblock->first_free = new_free;
	} else {
		new_free->next = prev_free->next;
		prev_free->next = new_free;
	}

	/* Merge free blocks */
	if (prev_free != NULL && maybe_merge_free(prev_free))
		maybe_merge_free(prev_free);
	else
		maybe_merge_free(new_free);

	return 0;
}

static int alloc_memory_block(size_t size)
{
	struct memory_block * new_block =
		valloc(0, &size, 0, 0, true, false, false);
	if (new_block == NULL)
		return -ENOMEM;
	init_block(new_block, size);
	new_block->next = first_block;
	first_block = new_block;
	return 0;
}

static void * _kmalloc(size_t size, size_t align)
{
	int err;
	struct free_memory_header ** free_mem =
		find_free_block_size_all(size, align);
	if (free_mem == NULL || *free_mem == NULL) {
		err = alloc_memory_block(DEFAULT_MEMORY_BLOCK_SIZE);
		if (err) {
			pr_alert("coult not create a new memory block, "
			         "errno = %d\n", err);
			return NULL;
		}
		free_mem = find_free_block_size_all(size, align);
		if (free_mem == NULL || *free_mem == NULL) {
			pr_alert("could not find %zu bytes [%zu aligned] "
			         "of free memory\n", size, align);
			return NULL;
		}
	}

	struct used_memory_header * alloc;
	err = alloc_free_block(free_mem, size, align, &alloc);
	if (err) {
		pr_alert("could not allocate %zu bytes of free memory at %p, "
		         "errno = %d", size, *free_mem, err);
		return NULL;
	}
	return (uint8_t *) alloc + sizeof(struct used_memory_header);
}

/* public: mm.h */
void * kmalloc(size_t size, size_t align)
{
	maybe_kmm_init();
	lock();
	void * ret = _kmalloc(size, align);
	unlock();
	return ret;
}

/* public: mm.h */
void * malloc(size_t size)
{
	return kmalloc(size, DEFAULT_MEMORY_ALLOC_ALIGN);
}

static void _kfree(void * ptr)
{
	if (ptr == NULL)
		return;
	if (find_memory_block_owner(ptr) == NULL) {
		pr_alert("tried to free memory unmanaged by kmm at %p\n", ptr);
		return;
	}

	struct used_memory_header * used = (void *)
		((uintptr_t) ptr - sizeof(struct used_memory_header));
	int err = free_used_block(used);
	if (err) {
		pr_alert("failed to free memory at %p [%zu bytes], "
		         "errno = %d\n", ptr, used->size_used, err);
		return;
	}
}

/* public: mm.h */
void kfree(const void * ptr)
{
	maybe_kmm_init();
	lock();
	_kfree((void *) ptr);
	unlock();
}

/* public: mm.h */
void free(void * ptr)
{
	kfree(ptr);
}

static void * _krealloc(void * oldptr, size_t newsize, size_t align)
{
	if (oldptr == NULL)
		return _kmalloc(newsize, align);
	if (find_memory_block_owner(oldptr) == NULL) {
		pr_alert("tried to free memory unmanaged by kmm at %p\n",
		         oldptr);
		return NULL;
	}

	struct used_memory_header * oldhdr = (void *)
		((uintptr_t) oldptr - sizeof(struct used_memory_header));
	void * newptr = _kmalloc(newsize, align);

	if (newptr == NULL) {
		pr_alert("out of memory: cannot realloc %p [%zu -> %zu bytes]"
		         "\n", oldptr, oldhdr->size_used, newsize);
		return NULL;
	}

	memcpy(newptr, oldptr, oldhdr->size_used);
	_kfree(oldptr);
	return newptr;
}

/* public: mm.h */
void * krealloc(void * oldptr, size_t newsize, size_t align)
{
	maybe_kmm_init();
	lock();
	void * ret = _krealloc(oldptr, newsize, align);
	unlock();
	return ret;
}

/* public: mm.h */
void * realloc(void * oldptr, size_t newsize)
{
	return krealloc(oldptr, newsize, DEFAULT_MEMORY_ALLOC_ALIGN);
}
