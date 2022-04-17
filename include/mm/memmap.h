#ifndef MM_MEMMAP_H
#define MM_MEMMAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <list.h>
#include <memlist.h>
#include <task.h>
#include <kconfig.h>

struct memmap {
	struct memlist l;
};

struct memmap_elt {
	struct memlist_elt l;
	enum memory_type {
		MEMORY_TYPE_RESERVED,
		MEMORY_TYPE_EFI_SERVICES,
		MEMORY_TYPE_ACPI_RECLAIMABLE,
		MEMORY_TYPE_ACPI_NVS,
		MEMORY_TYPE_PERSISTENT,
		MEMORY_TYPE_USED,
		MEMORY_TYPE_FREE,
	} type;
	pid_t owner;
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

static inline struct memmap_elt * memmap_get(
	struct memmap * m, void * addr, size_t size, bool full
)
{
	return (struct memmap_elt *) memlist_get(&m->l, addr, size, full);
}

const char * memmap_type_str(enum memory_type type);
#if IS_ENABLED(CONFIG_MM_DEBUG)
void memmap_print(struct memmap * map, const char * prefix);
#endif

struct memmap_elt * memmap_search(
	struct memmap * map, size_t size, size_t align,
	enum memory_type type, pid_t owner
);
int memmap_type(
	enum memory_type * dst, struct memmap * map, void * addr, size_t size
);
int memmap_update(
	struct memmap * map, void * start, size_t size,
	enum memory_type type, pid_t owner
);
int memmap_undef(
	struct memmap * map, void * start, size_t size
);

#ifdef __cplusplus
}
#endif
#endif // MM_MEMMAP_H
