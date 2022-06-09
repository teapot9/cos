#define pr_fmt(fmt) "pmm: " fmt

#include <alloc.h>
#include "pmm.h"
#include <setup.h>

#include <stdint.h>
#include <errno.h>

#include <cpp.h>
#include <print.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <mm/pmm.h>
#include <panic.h>
#include <kconfig.h>

static bool is_early_init = false;
static struct memlist early_pmaps;

bool pmm_is_init = false;

static void early_init(void)
{
	early_pmaps = memlist_new_default();
	memmap = memmap_new();
	is_early_init = true;
}

int early_pmap(void * paddr, size_t size)
{
	if (unlikely(!is_early_init))
		early_init();
	if (unlikely(pmm_is_init))
		return -EINVAL;
	int err = memlist_add(&early_pmaps, paddr, size, false);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("early_pmap(%p, %zu) -> %d\n", paddr, size, err);
#endif
	return err;
}

void early_punmap(void * paddr, size_t size)
{
	if (unlikely(!is_early_init))
		early_init();
	if (unlikely(pmm_is_init))
		return;
	int err = memlist_del(&early_pmaps, paddr, size, false);
	if (err)
		pr_err("early_punmap: failed to unmap %zu bytes", size);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("early_punmap(%p, %zu)\n", paddr, size);
#endif
}

/* public: setup.h */
int pmm_init(struct memmap * newmap)
{
	if (unlikely(!is_early_init))
		early_init();
	if (unlikely(pmm_is_init))
		return -EINVAL;

	int err = memmap_copy(&memmap, newmap);
	if (err)
		return err;

	pmm_is_init = true;

	/* Add early pmap to used memory */
	struct memlist_elt * early;
	list_foreach(early, early_pmaps.l.first) {
		int terr = pmap(0, early->addr, early->size);
		if (terr) {
			err = -ENOMEM;
			pr_err("failed to register early pmap at "
			       "%p [%zu bytes], errno = %d\n",
			       early->addr, early->size, terr);
		}
	}
	memlist_free(&early_pmaps, false);

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("pmm initialization, errno = %d\n", err);
#endif
	return err;
}
