#define pr_fmt(fmt) "mm: " fmt
#include <mm/memmap.h>
#include "memmap.h"

#include <errno.h>

#include <mm.h>
#include <mm/helper.h>

#ifdef CONFIG_MM_DEBUG
#include <print.h>
void memmap_print(struct memmap * map, const char * prefix)
{
	struct memmap_elt * cur;
	if (prefix == NULL)
		prefix = "";
	list_foreach(cur, map->l.l.first) {
		pr_debug("%s: [%p; %p] (%zu bytes): %s\n", prefix,
		         cur->l.addr, (uint8_t *) cur->l.addr + cur->l.size,
		         cur->l.size, memmap_type_str(cur->type));
	}
}
#endif

static bool compat(struct memlist_elt * a, struct memlist_elt * b)
{
	struct memmap_elt * x = (struct memmap_elt *) a;
	struct memmap_elt * y = (struct memmap_elt *) b;
	return x->type == y->type
		&& (x->type != MEMORY_TYPE_USED || x->owner == y->owner);
}

struct memmap memmap_new(void)
{
	return (struct memmap) {
		.l = memlist_new(compat, sizeof(struct memmap_elt))
	};
}

const char * memmap_type_str(enum memory_type type)
{
	switch (type) {
	case MEMORY_TYPE_RESERVED:
		return "reserved";
	case MEMORY_TYPE_EFI_SERVICES:
		return "EFI services";
	case MEMORY_TYPE_ACPI_RECLAIMABLE:
		return "ACPI reclaimable";
	case MEMORY_TYPE_ACPI_NVS:
		return "ACPI NVS";
	case MEMORY_TYPE_PERSISTENT:
		return "persistent";
	case MEMORY_TYPE_USED:
		return "used";
	case MEMORY_TYPE_FREE:
		return "free";
	default:
		return "invalid";
	}
}

struct memmap_elt * memmap_search(
	struct memmap * map, size_t size, size_t align,
	enum memory_type type, pid_t owner
)
{
	struct memmap_elt * cur;
	list_foreach(cur, map->l.l.first) {
		if (cur->l.size >= size + align_diff(cur->l.addr, align)
		    && cur->type == type
		    && (type != MEMORY_TYPE_USED || cur->owner == owner))
			return cur;
	}
	return NULL;
}

int memmap_type(
	enum memory_type * dst, struct memmap * map, void * addr, size_t size
)
{
	if (dst == NULL)
		return -EINVAL;
	struct memmap_elt * found =
		(struct memmap_elt *) memlist_get(&map->l, addr, size, true);
	if (found == NULL)
		return -ENOENT;
	*dst = found->type;
	return 0;
}

int memmap_update(
	struct memmap * map, void * start, size_t size,
	enum memory_type type, pid_t owner
)
{
	int err = memmap_undef(map, start, size);
	if (err && err != -ENOENT)
		return err;

	struct memmap_elt * new = malloc(sizeof(*new));
	new->l.addr = start;
	new->l.size = size;
	new->type = type;
	new->owner = type == MEMORY_TYPE_USED ? owner : 0;
	return memlist_add_elt(&map->l, &new->l, true);
}

int memmap_undef(
	struct memmap * map, void * start, size_t size
)
{
	return memlist_del(&map->l, start, size, false);
}
