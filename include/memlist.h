#ifndef MEMLIST_H
#define MEMLIST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>

struct memlist_elt;

struct memlist {
	struct list l;
	size_t elt_size;
	bool (*compat)(struct memlist_elt * a, struct memlist_elt * b);
};

struct memlist_elt {
	struct list_head l;
	void * addr;
	size_t size;
};

#define memlist_new(compat_fcn, size) (struct memlist) { \
		.l = list_new(), \
		.compat = compat_fcn, \
		.elt_size = size, \
	}

#define memlist_new_default() memlist_new(NULL, sizeof(struct memlist_elt))

static inline void memlist_free(struct memlist * list, bool free_ptr)
{
	list_free_all(&list->l, free_ptr);
	if (!free_ptr)
		*list = memlist_new_default();
}

static inline int memlist_copy(struct memlist * dst, struct memlist * src)
{
	return list_copy((struct list *) dst, (struct list *) src,
	                 sizeof(struct memlist_elt));
}

int memlist_add_elt(struct memlist * l, struct memlist_elt * elt, bool strict);
int memlist_add(struct memlist * l, void * addr, size_t size, bool strict);
int memlist_del_elt(struct memlist * l, struct memlist_elt * elt, bool strict);
int memlist_del(struct memlist * l, void * addr, size_t size, bool strict);

struct memlist_elt * memlist_get_ptr(struct memlist * l, void * addr);
struct memlist_elt * memlist_get(
	struct memlist * l, void * addr, size_t size, bool full
);
struct memlist_elt * memlist_search(
	struct memlist * l, size_t size, size_t align
);

size_t memlist_virtual_size(struct memlist * l);

#ifdef __cplusplus
}
#endif
#endif // MEMLIST_H
