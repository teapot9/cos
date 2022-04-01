#define pr_fmt(fmt) "pmm: " fmt

#include <mm.h>
#include "pmm.h"

#include <stdint.h>
#include <errno.h>

#include <print.h>
#include <mm/helper.h>
#include <memlist.h>
#include <mm/memmap.h>

/* public: mm.h */
int pmap(void * paddr, size_t size)
{
	int err;
	if (!pmm_is_init())
		return early_pmap(paddr, size);

	if (memlist_get(&used_blocks, paddr, size, false) != NULL) {
		pr_err("cannot allocate %zu bytes of physical memory at %p: "
		       "memory is in use\n", size, paddr);
		return -ENOMEM;
	}

	err = memlist_del(&free_blocks, paddr, size, true);
	if (err == -ENOENT) {
		pr_debug("reserved memory alloc: %zuB at %p\n", size, paddr);
		goto exit_ok;
	} else if (err) {
		pr_crit("cannot remove %zuB of free memory at %p, errno = %d\n",
			size, paddr, err);
		return err;
	}

	err = memlist_add(&used_blocks, paddr, size, true);
	if (err) {
		pr_err("cannot allocate %zu bytes of physical memory at %p: "
		       "memory is in use, errno = %d\n", size, paddr, err);
		err = memlist_add(&free_blocks, paddr, size, true);
		if (err)
			pr_crit("lost %zu bytes of memory at %p: "
				"cannot mark as free, errno = %d",
				size, paddr, err);
		return err;
	}

exit_ok:
#ifdef CONFIG_MM_DEBUG
	pr_debug("pmap(%p, %zu) -> %d\n", paddr, size, 0);
#endif
	return 0;
}

/* public: mm.h */
void * pmalloc(size_t size, size_t align)
{
	int err;
	if (!pmm_is_init())
		return early_pmalloc(size, align);

	struct memlist_elt * found = memlist_search(&free_blocks, size, align);
	if (found == NULL) {
		pr_err("cannot allocate %zu bytes of physical memory: "
		       "out of memory\n", size);
	}

	void * paddr = aligned(found->addr, align);

	err = pmap(paddr, size);
	if (err)
		return NULL;
#ifdef CONFIG_MM_DEBUG
	pr_debug("pmalloc(%zu, %zu) -> %p\n", size, align, paddr);
#endif
	return paddr;
}

/* public: mm.h */
void pfree(void * paddr, size_t size)
{
	int err;
	if (!pmm_is_init()) {
		early_pfree(paddr, size);
		return;
	}

	err = memlist_del(&used_blocks, paddr, size, true);
	if (err) {
		pr_err("cannot free %zu bytes of physical memory at %p: "
		       "memory is not used, errno = %d\n", size, paddr, err);
		memlist_del(&used_blocks, paddr, size, false);
		pr_crit("lost up to %zu bytes of physical memory at %p\n",
			size, paddr);
		return;
	}

	err = memlist_add(&free_blocks, paddr, size, true);
	if (err) {
		pr_err("cannot free %zu bytes of physical memory at %p: "
		       "memory is already free, errno = %d\n",
		       size, paddr, err);
		return;
	}
#ifdef CONFIG_MM_DEBUG
	pr_debug("pfree(%p, %zu)\n", paddr, size);
#endif
}
