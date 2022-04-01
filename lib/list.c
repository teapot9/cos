#define pr_fmt(fmt) "list: " fmt
#include <list.h>

#include <errno.h>

#include <mm.h>
#include <string.h>

static inline void add(struct list * l, struct list_head * prev,
                       struct list_head * next, struct list_head * new)
{
	new->prev = prev;
	new->next = next;
	if (prev != NULL)
		prev->next = new;
	if (next != NULL)
		next->prev = new;
	if (l->first == next)
		l->first = new;
	if (l->last == prev)
		l->last = new;
}

void list_free_all(struct list * l, bool free_list)
{
	struct list_head * elt = l->first;
	while (elt != NULL) {
		struct list_head * next = elt->next;
		kfree(elt);
		elt = next;
	}
	if (free_list)
		kfree(l);
}

void list_add(struct list * l, size_t index, struct list_head * elt)
{
	struct list_head * prev;
	struct list_head * cur;
	size_t i;
	for (i = 0, prev = NULL, cur = l->first;
	     i < index && cur != NULL;
	     i++, prev = cur, cur = cur->next);
	if (cur == NULL)
		return;
	add(l, prev, cur, elt);
}

void list_del(struct list * l, struct list_head * elt, bool do_free)
{
	elt->prev->next = elt->next;
	elt->next->prev = elt->prev;
	if (l->first == elt)
		l->first = elt->next;
	if (l->last == elt)
		l->last = elt->prev;
	if (do_free)
		kfree(elt);
}

void list_append(struct list * l, struct list_head * elt)
{
	add(l, l->last, NULL, elt);
}

void list_push(struct list * l, struct list_head * elt)
{
	add(l, NULL, l->first, elt);
}

void list_add_after(struct list * l, struct list_head * prev,
                    struct list_head * elt)
{
	add(l, prev, prev->next, elt);
}

void list_add_before(struct list * l, struct list_head * next,
		     struct list_head * elt)
{
	add(l, next->prev, next, elt);
}

int list_copy(struct list * dst, struct list * l, size_t elt_size)
{
	if (dst == NULL || l == NULL || elt_size < sizeof(struct list_head))
		return -EINVAL;

	struct list new_list = list_new();

	struct list_head * cur;
	list_foreach(cur, l->first) {
		struct list_head * new = malloc(elt_size);
		if (new == NULL) {
			list_free_all(&new_list, true);
			return -ENOMEM;
		}

		memcpy(new, cur, elt_size);
		list_append(&new_list, new);
	}

	*dst = new_list;
	return 0;
}
