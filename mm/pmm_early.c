#define pr_fmt(fmt) "pmm: " fmt

#include <mm.h>
#include <mm/early.h>
#include "pmm.h"
#include <setup.h>

#include <stdint.h>
#include <errno.h>

#include <cpp.h>
#include <print.h>
#include <mm/helper.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <panic.h>

static bool is_init = false;
static bool is_structs_init = false;
static struct memlist early_pmaps;
struct memmap memmap;
struct memlist used_blocks;
struct memlist free_blocks;

static void do_structs_init(void)
{
	if (is_structs_init)
		return;
	early_pmaps = memlist_new_default();
	used_blocks = memlist_new_default();
	free_blocks = memlist_new_default();
	memmap = memmap_new();
	is_structs_init = true;
}

int early_pmap(void * paddr, size_t size)
{
	if (unlikely(!is_structs_init))
		do_structs_init();

	int err = memlist_add(&early_pmaps, paddr, size, false);

#ifdef CONFIG_MM_DEBUG
	pr_debug("early_pmap(%p, %zu) -> %d\n", paddr, size, err);
#endif
	return err;
}

void * early_pmalloc(UNUSED size_t size, UNUSED size_t align)
{
	if (unlikely(!is_structs_init))
		do_structs_init();

	pr_err("early_pmalloc: unavailable during early boot\n", 0);
	return NULL;
}

void early_pfree(void * paddr, size_t size)
{
	if (unlikely(!is_structs_init))
		do_structs_init();

	int err = memlist_del(&early_pmaps, paddr, size, false);
	if (err)
		pr_warn("failed to free %zu bytes of physical memory, "
		        "errno = %d\n", err);

#ifdef CONFIG_MM_DEBUG
	pr_debug("early_pfree(%p, %zu)\n", paddr, size);
#endif
}

bool pmm_is_init(void)
{
	return is_init;
}

/* public: setup.h */
int pmm_init(struct memmap * newmap)
{
	if (unlikely(!is_structs_init))
		do_structs_init();

	int err = memmap_copy(&memmap, newmap);
	if (err)
		return err;

	/* Mark available memory as free */
	struct memmap_elt * cur;
	list_foreach(cur, memmap.l.l.first) {
		if (cur->type != MEMORY_TYPE_AVAILABLE)
			continue;

		int terr = memlist_add(
			&free_blocks, cur->l.addr, cur->l.size, false
		);
		if (terr) {
			err = terr;
			pr_err("failed to register available memory at "
			       "%p [%zu bytes], errno = %d\n",
			       cur->l.addr, cur->l.size, terr);
		}
	}

	is_init = true;

	/* Add early pmap to used memory */
	struct memlist_elt * early;
	list_foreach(early, early_pmaps.l.first) {
		int terr = pmap(early->addr, early->size);
		if (terr) {
			err = -ENOMEM;
			pr_err("failed to register early pmap at "
			       "%p [%zu bytes], errno = %d\n",
			       early->addr, early->size, terr);
		}
	}
	memlist_free(&early_pmaps, false);

	return err;
}
