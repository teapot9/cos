#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <cpp.h>
#include <assert.h>

struct list {
	struct list_head * first;
	struct list_head * last;
};

struct list_head {
	struct list_head * next;
	struct list_head * prev;
};

#define list_new() (struct list) {.first = NULL, .last = NULL}
void list_free_all(struct list * l, bool free_list);
int list_copy(struct list * dst, struct list * l, size_t elt_size);

void list_add(struct list * l, size_t index, struct list_head * elt);
void list_del(struct list * l, struct list_head * elt, bool free);
void list_append(struct list * l, struct list_head * elt);
void list_push(struct list * l, struct list_head * elt);
void list_add_after(struct list * l, struct list_head * prev,
                    struct list_head * elt);
void list_add_before(struct list * l, struct list_head * next,
		     struct list_head * elt);

#define list_foreach(elt, start) \
	static_assert(same_type(start, struct list_head *), \
	              "start should be a struct list_head *"); \
	for (elt = (typeof(elt)) start; elt != NULL; \
	     elt = (typeof(elt)) ((struct list_head *) elt)->next)

#endif // LIST_H
