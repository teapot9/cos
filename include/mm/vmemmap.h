#ifndef MM_VMEMMAP_H
#define MM_VMEMMAP_H

#include <stddef.h>

#include <list.h>
#include <memlist.h>
#include <mm.h>

struct vmemmap {
	struct memlist l;
};

struct vmemmap_elt {
	struct memlist_elt l;
	void * map;
	struct page_perms perms;
};

struct vmemmap vmemmap_new(void);
#define vmemmap_free(elt, free_ptr) do { \
	memlist_free(elt.l, free_ptr); \
	if (!free_ptr) \
		*elt = memmap_new(); \
	} while (0);

static inline int vmemmap_copy(struct vmemmap * dst, struct vmemmap * src)
{
	return list_copy((struct list *) dst, (struct list *) src,
			 sizeof(struct vmemmap_elt));
}

static inline struct vmemmap_elt * vmemmap_get(
	struct vmemmap * m, void * addr, size_t size, bool full
)
{
	return (struct vmemmap_elt *) memlist_get(&m->l, addr, size, full);
}

#ifdef CONFIG_MM_DEBUG
void vmemmap_print(struct vmemmap * map, const char * prefix);
#endif

int vmemmap_map(
	struct vmemmap * map, void * start, size_t size,
	void * map_addr, struct page_perms perms, bool override
);
int vmemmap_unmap(
	struct vmemmap * map, void * start, size_t size
);

#endif // MM_VMEMMAP_H
