#define pr_fmt(fmt) "mm: " fmt

#include <mm/block.h>

#include <errno.h>

#include <mm.h>
#include <mm/helper.h>

static int insert(struct memblock_list ** prev, void * addr, size_t size)
{
	if (prev == NULL)
		return -EINVAL;

	struct memblock_list * new = kmalloc(sizeof(*new));
	new->addr = addr;
	new->size = size;
	new->next = *prev;
	*prev = new;

	return 0;
}

static int _quick_merge_next(struct memblock_list * cur)
{
	struct memblock_list * next = cur->next;

	if (cur->addr > next->addr) {
		cur->addr = next->addr;
		cur->size += next->size;
	}
	cur->size += next->size;
	cur->next = next->next;

	kfree(next);
	return 0;
}

static int _merge_new(struct memblock_list * cur, void * addr, size_t size)
{
	if (cur == NULL || (!is_overlap(cur->addr, cur->size, addr, size)
	                    && !is_next(cur->addr, cur->size, addr, size)))
		return -EINVAL;

	if (is_next(cur->addr, cur->size, addr, size)) {
		int err = insert(&cur->next, addr, size);
		if (err)
			return err;
		return _quick_merge_next(cur);
	}

	uint8_t * dst_start = cur->addr;
	uint8_t * dst_end = dst_start + cur->size;
	uint8_t * src_start = addr;
	uint8_t * src_end = src_start + size;

	if (src_start < dst_start) {
		// Append dst block on left
		size_t diff = dst_start - src_start;
		dst_start = src_start;
		cur->addr = addr;
		cur->size += diff;
	}
	if (src_end > dst_end) {
		// Append dst block on right
		size_t diff = src_end - dst_end;
		dst_end = src_end;
		cur->size += diff;
	}

	return 0;
}

static int _merge_current(struct memblock_list * cur)
{
	int err;
	if (cur == NULL)
		return -EINVAL;

	if (is_next(cur->addr, cur->size, cur->next->addr, cur->next->size))
		return _quick_merge_next(cur);

	if (!is_overlap(cur->addr, cur->size, cur->next->addr, cur->next->size))
		return 0;

	err = _merge_new(cur, cur->next->addr, cur->next->size);
	if (err)
		return err;

	struct memblock_list * next = cur->next;
	cur->next = cur->next->next;
	kfree(next);

	return 0;
}

static int merge(struct memblock_list ** prev, void * addr, size_t size)
{
	int err;
	if (prev == NULL)
		return -EINVAL;

	struct memblock_list * cur = *prev;
	if (cur == NULL)
		return -EINVAL;

	err = _merge_new(cur, addr, size);
	if (err)
		return err;

	err = _merge_current(cur);
	if (err)
		return err;

	return 0;
}

static int _unmerge(struct memblock_list * cur, void * addr, size_t size)
{
	if (cur == NULL)
		return -EINVAL;

	uint8_t * dst_start = cur->addr;
	uint8_t * dst_end = dst_start + cur->size;
	uint8_t * src_start = addr;
	uint8_t * src_end = src_start + size;

	if (src_start > dst_start) {
		// Keep left part of dst block
		size_t diff = src_start - dst_start;
		cur->size = diff;
	}
	if (src_end < dst_end) {
		// Keep right part of dst block
		size_t diff = dst_end - src_end;
		if (src_start > dst_start)
			return insert(&cur->next, src_end, diff);
		cur->addr = src_end;
		cur->size = diff;
	}

	return 0;
}

static int unmerge(struct memblock_list ** prev, void * addr, size_t size)
{
	if (prev == NULL)
		return -EINVAL;
	if (*prev== NULL)
		return -ENOENT;
	struct memblock_list * cur = *prev;

	if (cur->addr == addr && cur->size == size) {
		*prev = cur->next;
		kfree(cur);
		return 0;
	} else {
		return _unmerge(cur, addr, size);
	}
}

struct memblock_list ** memblock_search(struct memblock_list ** memblock,
                                        void * addr, size_t size, bool full)
{
	if (memblock == NULL)
		return NULL;

	struct memblock_list ** prev = memblock;
	struct memblock_list * cur = *memblock;

	while (cur != NULL && cur->addr <= addr) {
		if (full && is_inside(cur->addr, cur->size, addr, size))
			return prev;
		if (!full && is_overlap(cur->addr, cur->size, addr, size))
			return prev;
		if (!full && is_next(cur->addr, cur->size, addr, size))
			return prev;
		prev = &(*prev)->next;
		cur = cur->next;
	}
	return prev;
}

bool memblock_exists(struct memblock_list ** memblock,
		     void * addr, size_t size, bool full)
{
	struct memblock_list ** found =
		memblock_search(memblock, addr, size, full);
	if (found == NULL || *found == NULL)
		return false;
	struct memblock_list * cur = *found;

	return (full && is_inside(cur->addr, cur->size, addr, size))
		|| (!full && is_overlap(cur->addr, cur->size, addr, size));
}

struct memblock_list ** memblock_search_size(struct memblock_list ** memblock,
                                        size_t size, size_t align)
{
	if (memblock == NULL)
		return NULL;

	struct memblock_list ** prev = memblock;
	struct memblock_list * cur = *memblock;

	while (cur != NULL) {
		if (cur->size - align_diff(cur->addr, align) >= size)
			return prev;
		prev = &(*prev)->next;
		cur = cur->next;
	}
	return prev;
}

int memblock_add(struct memblock_list ** memblock,
                 void * addr, size_t size, bool strict)
{
	if (memblock == NULL)
		return -EINVAL;

	struct memblock_list ** prev = memblock_search(memblock, addr, size, false);
	struct memblock_list * cur = *prev;

	if (cur != NULL) {
		if (is_overlap(cur->addr, cur->size, addr, size)) {
			if (strict)
				return -EBUSY;
			return merge(prev, addr, size);
		}
		if (is_next(cur->addr, cur->size, addr, size))
			return merge(prev, addr, size);
	}
	return insert(prev, addr, size);
}

int memblock_rem(struct memblock_list ** memblock,
		 void * addr, size_t size, bool strict)
{
	if (memblock == NULL)
		return -EINVAL;

	struct memblock_list ** prev = memblock_search(memblock, addr, size, false);
	struct memblock_list * cur = *prev;

	if (cur == NULL)
		return -ENOENT;

	if (!is_overlap(cur->addr, cur->size, addr, size))
		return -ENOENT;

	if (!is_inside(cur->addr, cur->size, addr, size) && strict)
		return -ENOENT;

	return unmerge(prev, addr, size);
}

void memblock_free(struct memblock_list ** memblock)
{
	if (memblock == NULL)
		return;

	struct memblock_list * cur = *memblock;
	*memblock = NULL;

	while (cur != NULL) {
		struct memblock_list * tmp = cur;
		cur = cur->next;
		kfree(tmp);
	}
}
