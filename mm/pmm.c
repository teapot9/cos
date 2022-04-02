#define pr_fmt(fmt) "pmm: " fmt

#include <mm.h>
#include "pmm.h"

#include <stdint.h>
#include <errno.h>

#include <print.h>
#include <mm/helper.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <kconfig.h>

/* public: mm.h */
struct memmap memmap;

/* public: mm.h */
int pmap(pid_t pid, void * paddr, size_t size)
{
	if (unlikely(!pmm_is_init))
		return -EAGAIN;
		//return early_pmap(paddr, size);

	if (size % 4096)
		pr_warn("physical mapping of size %zu "
		        "bytes is not multiple of 4096\n", size);

	struct memmap_elt * found = memmap_get(&memmap, paddr, size, true);
	if (found == NULL)
		return -ENOENT;
	if (pid != 0) {
		if (found->type != MEMORY_TYPE_FREE)
			return -EACCES;
	} else {
		if (found->type == MEMORY_TYPE_USED)
			return -EBUSY;
		if (found->type != MEMORY_TYPE_FREE)
			return 0;
	}
	int err = memmap_update(&memmap, paddr, size, MEMORY_TYPE_USED, pid);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("pmap(%zu, %p, %zu) -> %d\n", pid, paddr, size, err);
#endif
	return err;
}

/* public: mm.h */
void * palloc(pid_t pid, size_t size, size_t align)
{
	if (unlikely(!pmm_is_init))
		return NULL;

	struct memmap_elt * found =
		memmap_search(&memmap, size, align, MEMORY_TYPE_FREE, 0);
	if (found == NULL) {
		pr_crit("cannot find %zu bytes of pmem\n", size);
		return NULL;
	}
	void * paddr = aligned(found->l.addr, align);
	int err = pmap(pid, paddr, size);
	if (err) {
		pr_err("cannot allocate %zu bytes of pmem, errno = %d\n",
		       size, err);
		paddr = NULL;
	}
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("palloc(%zu, %zu, %zu) -> %p\n", pid, size, align, paddr);
#endif
	return paddr;
}

/* public: mm.h */
void punmap(pid_t pid, void * paddr, size_t size)
{
	if (unlikely(!pmm_is_init)) {
		//early_punmap(paddr, size);
		return;
	}

	struct memmap_elt * found = memmap_get(&memmap, paddr, size, true);
	if (found == NULL
	    || found->type != MEMORY_TYPE_USED
	    || found->owner != pid)
		return;
	memmap_update(&memmap, paddr, size, MEMORY_TYPE_FREE, 0);
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("punmap(%zu, %p, %zu)\n", pid, paddr, size);
#endif
}
