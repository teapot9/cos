#ifndef MM_MEMMAP_H
#define MM_MEMMAP_H

#include <stddef.h>

#include <list.h>
#include <memlist.h>

struct memmap {
	struct memlist l;
};

struct memmap_elt {
	struct memlist_elt l;
	enum memory_type {
		MEMORY_TYPE_RESERVED,
		MEMORY_TYPE_EFI_SERVICES,
		MEMORY_TYPE_AVAILABLE,
		MEMORY_TYPE_ACPI_RECLAIMABLE,
		MEMORY_TYPE_ACPI_NVS,
		MEMORY_TYPE_PERSISTENT,
	} type;
};

struct memmap memmap_new(void);
#define memmap_free(elt, free_ptr) do { \
	memlist_free(elt.l, free_ptr); \
	if (!free_ptr) \
		*elt = memmap_new(); \
	} while (0)
static inline int memmap_copy(struct memmap * dst, struct memmap * src)
{
	return list_copy((struct list *) dst, (struct list *) src,
	                 sizeof(struct memmap_elt));
}

const char * memmap_type_str(enum memory_type type);
#ifdef CONFIG_MM_DEBUG
void memmap_print(struct memmap * map, const char * prefix);
#endif

int memmap_type(
	enum memory_type * dst, struct memmap * map, void * addr, size_t size
);
int memmap_update(
	struct memmap * map, void * start, size_t size, enum memory_type type
);
int memmap_undef(
	struct memmap * map, void * start, size_t size
);

#endif // MM_MEMMAP_H
