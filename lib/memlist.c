#define pr_fmt(fmt) "memlist: " fmt
#include <memlist.h>

#include <errno.h>

#include <mm/block.h>
#include <mm/align.h>
#include <alloc.h>
#include <string.h>

static void inline copy_user_data(
	struct memlist * l, struct memlist_elt * dst, struct memlist_elt * src
)
{
	memcpy((uint8_t *) dst + sizeof(struct memlist_elt),
	       (uint8_t *) src + sizeof(struct memlist_elt),
	       l->elt_size - sizeof(struct memlist_elt));
}

static inline bool is_compat(
	struct memlist * l, struct memlist_elt * a, struct memlist_elt * b
)
{
	return l->compat == NULL || l->compat(a, b);
}

static inline bool mlist_is_overlap(
	struct memlist_elt * a, struct memlist_elt * b
)
{
	return is_overlap(a->addr, a->size, b->addr, b->size);
}

static inline bool mlist_is_inside(
	struct memlist_elt * a, struct memlist_elt * b
)
{
	return is_inside(a->addr, a->size, b->addr, b->size);
}

static inline bool mlist_is_next(
	struct memlist_elt * a, struct memlist_elt * b
)
{
	return is_next(a->addr, a->size, b->addr, b->size);
}

static inline bool mlist_is_ordered(
	struct memlist_elt * is_before, struct memlist_elt * is_after
)
{
	uint8_t * is_before_end = is_before->addr == 0
		? is_before->addr
		: (uint8_t *) is_before->addr + is_before->size;
	return is_before_end <= (uint8_t *) is_after->addr;
}

static inline bool mlist_is_mergeable(
	struct memlist * l, struct memlist_elt * a, struct memlist_elt * b
)
{
	return (mlist_is_overlap(a, b) || mlist_is_next(a, b))
		&& is_compat(l, a, b);
}

static int merge_into(
	struct memlist * l, struct memlist_elt * dst, struct memlist_elt * src
)
{
	if (l == NULL || dst == NULL || src == NULL)
		return -EINVAL;

	if (mlist_is_overlap(dst, src) && is_compat(l, dst, src)) {
		if (mlist_is_inside(dst, src))
			return 0;
		if (mlist_is_inside(src, dst)) {
			dst->addr = src->addr;
			dst->size = src->size;
			return 0;
		}
		if (dst->addr > src->addr) {
			size_t diff =
				(uint8_t *) dst->addr - (uint8_t *) src->addr;
			dst->addr = src->addr;
			dst->size += diff;
			return 0;
		}
		uint8_t * dst_end = (uint8_t *) dst->addr + dst->size;
		if (dst_end > (uint8_t *) src->addr) {
			size_t diff = dst_end - (uint8_t *) src->addr;
			dst->size += diff;
			return 0;
		}
		return -EINVAL;
	} else if (mlist_is_next(dst, src) && is_compat(l, dst, src)) {
		if (dst->addr > src->addr)
			dst->addr = src->addr;
		dst->size += src->size;
		return 0;
	} else {
		return -ENOTSUP;
	}
}

static struct memlist_elt * first_overlap_or_next(
	struct memlist * l, struct memlist_elt * elt
)
{
	struct memlist_elt * cur;
	list_foreach(cur, l->l.first) {
		if (mlist_is_overlap(cur, elt) || mlist_is_next(cur, elt))
			return cur;
		if (!mlist_is_ordered(cur, elt))
			return cur;
	}
	return NULL;
}

/* public: memlist.h */
struct memlist_elt * memlist_search(
	struct memlist * l, size_t size, size_t align
)
{
	struct memlist_elt * cur;
	list_foreach(cur, l->l.first) {
		if (cur->size - align_diff_up(cur->addr, align) >= size)
			return cur;
	}
	return NULL;
}

/* public: memlist.h */
struct memlist_elt * memlist_get(
	struct memlist * l, void * addr, size_t size, bool full
)
{
	struct memlist_elt elt;
	elt.addr = addr;
	elt.size = size;
	struct memlist_elt * found = first_overlap_or_next(l, &elt);
	if (found != NULL) {
		if (mlist_is_next(found, &elt))
			found = (struct memlist_elt *) found->l.next;
		if (full && !mlist_is_inside(found, &elt))
			found = NULL;
		else if (!full && !mlist_is_overlap(found, &elt))
			found = NULL;
	}
	return found;
}

/* public: memlist.h */
struct memlist_elt * memlist_get_ptr(struct memlist * l, void * addr)
{
	return memlist_get(l, addr, 1, true);
}

/* public: memlist.h */
int memlist_add_elt(struct memlist * l, struct memlist_elt * elt, bool strict)
{
	if (l == NULL || elt == NULL)
		return -EINVAL;

	struct memlist_elt * found = first_overlap_or_next(l, elt);

	if (found == NULL) {
		list_append(&l->l, &elt->l);
		return 0;
	}

	bool overlap = mlist_is_overlap(found, elt);
	if (strict && overlap)
		return -EBUSY;

	int err = merge_into(l, found, elt);

	switch (err) {
	case 0:
		kfree(elt);
		return 0;
	case -ENOTSUP:
		if (overlap)
			return -EBUSY;
		if (found->addr > elt->addr)
			list_add_before(&l->l, &found->l, &elt->l);
		else
			list_add_after(&l->l, &found->l, &elt->l);
		return 0;
	default:
		return err;
	}
}

/* public: memlist.h */
int memlist_add(struct memlist * l, void * addr, size_t size, bool strict)
{
	struct memlist_elt * elt = malloc(sizeof(*elt));
	elt->addr = addr;
	elt->size = size;
	int err = memlist_add_elt(l, elt, strict);
	if (err)
		kfree(elt);
	return err;
}

static int unmerge(
	struct memlist * l, struct memlist_elt * dst, struct memlist_elt * src
)
{
	if (l == NULL || dst == NULL || src == NULL)
		return -EINVAL;

	if (mlist_is_overlap(dst, src)) {
		uint8_t * dst_start = dst->addr;
		uint8_t * dst_end = dst_start + dst->size;
		uint8_t * src_start = src->addr;
		uint8_t * src_end = src_start + src->size;
		if (src_start <= dst_start && src_end >= dst_end) {
			// src covers dst entirely
			list_del(&l->l, &dst->l, true);
			return 0;
		}
		if (src_start <= dst_start) {
			// src cover left part of dst
			size_t diff = src_end - dst_start;
			dst->addr = src_end;
			dst->size -= diff;
			return 0;
		}
		if (src_end >= dst_end) {
			// src cover right part of dst
			size_t diff = dst_end - src_start;
			dst->size -= diff;
			return 0;
		}
		// src is in the middle of dst
		struct memlist_elt * left = malloc(l->elt_size);
		copy_user_data(l, left, dst);
		left->addr = dst->addr;
		left->size = src_start - dst_start;
		list_add_before(&l->l, &dst->l, &left->l);
		dst->addr = src_end;
		dst->size = dst_end - src_end;
		return 0;
	}
	return -ENOTSUP;
}

/* public: memlist.h */
int memlist_del_elt(struct memlist * l, struct memlist_elt * elt, bool strict)
{
	if (l == NULL || elt == NULL)
		return -EINVAL;

	struct memlist_elt * found = first_overlap_or_next(l, elt);

	if (found == NULL)
		return strict ? -ENOENT : 0;

	if (!mlist_is_inside(found, elt)) {
		if (strict)
			return -ENOENT;
		if (mlist_is_next(found, elt)) {
			found = (struct memlist_elt *) found->l.next;
			if (found == NULL)
				return strict ? -ENOENT : 0;
		}
	}

	int err;
	do {
		struct memlist_elt * next =
			(struct memlist_elt *) found->l.next;
		err = unmerge(l, found, elt);
		found = next;
	} while (!err && found != NULL);

	return (err && err != -ENOTSUP) ? err : 0;
}

/* public: memlist.h */
int memlist_del(struct memlist * l, void * addr, size_t size, bool strict)
{
	struct memlist_elt elt;
	elt.addr = addr;
	elt.size = size;
	int err = memlist_del_elt(l, &elt, strict);
	return err;
}

/* public: memlist.h */
size_t memlist_virtual_size(struct memlist * l) {
	uint8_t * max_addr = (void *) 0;
	struct memlist_elt * cur = (struct memlist_elt *) l->l.first;
	while (cur != NULL) {
		uint8_t * local_max = cur->addr == 0
			? (void *) cur->size
			: (uint8_t *) cur->addr + cur->size;
		if (local_max > max_addr)
			max_addr = local_max;
		cur = (struct memlist_elt *) cur->l.next;
	}
	return (size_t) max_addr;
}
