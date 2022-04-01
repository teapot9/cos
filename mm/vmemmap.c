#define pr_fmt(fmt) "mm: " fmt
#include <mm/vmemmap.h>

#include <errno.h>

#include <mm.h>
#include <mm/helper.h>

#ifdef CONFIG_MM_DEBUG
#include <print.h>
void vmemmap_print(struct vmemmap * map, const char * prefix)
{
	struct vmemmap_elt * cur;
	if (prefix == NULL)
		prefix = "";
	list_foreach(cur, map->l.l.first) {
		pr_debug(
			"%s: [%p; %p] -> [%p; %p] (%zu bytes) [%c%c%c]\n",
			prefix,
			cur->l.addr,
			(uint8_t *) cur->l.addr + cur->l.size,
			cur->map,
			(uint8_t *) cur->map + cur->l.size,
			cur->l.size,
			cur->perms.write ? 'W' : ' ',
			cur->perms.exec ? 'X' : ' ',
			cur->perms.user ? 'U' : ' '
		);
	}
}
#endif

static bool compat(struct memlist_elt * a, struct memlist_elt * b)
{
	struct vmemmap_elt * x = (void *) a;
	struct vmemmap_elt * y = (void *) b;
	return is_next(x->map, x->l.size, y->map, y->l.size)
		&& x->perms.write == y->perms.write
		&& x->perms.user == y->perms.user
		&& x->perms.exec == y->perms.exec;
}

struct vmemmap vmemmap_new(void)
{
	return (struct vmemmap) {
		.l = memlist_new(compat, sizeof(struct vmemmap_elt))
	};
}

int vmemmap_map(
	struct vmemmap * map, void * start, size_t size,
	void * map_addr, struct page_perms perms, bool override
)
{
	int err;
	if (override) {
		err = vmemmap_unmap(map, start, size);
		if (err && err != -ENOENT)
			return err;
	}

	struct vmemmap_elt * new = malloc(sizeof(*new));
	new->l.addr = start;
	new->l.size = size;
	new->map = map_addr;
	new->perms = perms;

	err = memlist_add_elt(&map->l, &new->l, !override);
	if (err)
		kfree(new);
	return err;
}

int vmemmap_unmap(
	struct vmemmap * map, void * start, size_t size
)
{
	return memlist_del(&map->l, start, size, false);
}
