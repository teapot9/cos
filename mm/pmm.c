#include <mm.h>
#include "pmm.h"

#include <stdint.h>
#include <errno.h>

#include <print.h>
#include "helper.h"
#include "block.h"

struct memblock * first_free_block = NULL;
struct memblock * first_used_block = NULL;

/* public: mm.h */
int pmap(void * paddr, size_t size)
{
	int err;
	if (!pmm_is_init())
		return early_pmap(paddr, size);

	err = memblock_rem(&first_free_block, paddr, size, true);
	if (err) {
		pr_err("Cannot allocate %zu bytes of physical memory at %p: "
		       "memory is not free, errno = %d\n", size, paddr, err);
		return err;
	}

	err = memblock_add(&first_used_block, paddr, size, true);
	if (err) {
		pr_err("Cannot allocate %zu bytes of physical memory at %p: "
		       "memory is in use, errno = %d\n", size, paddr, err);
		err = memblock_add(&first_free_block, paddr, size, true);
		if (err)
			pr_crit("Lost %zu bytes of memory at %p: "
				"cannot mark as free, errno = %d",
				size, paddr, err);
		return err;
	}

	pr_debug("pmap(%p, %zu) -> %d\n", paddr, size, 0);
	return 0;
}

/* public: mm.h */
void * pmalloc(size_t size, size_t align)
{
	int err;
	if (!pmm_is_init())
		return early_pmalloc(size, align);

	struct memblock ** prev =
		memblock_search_size(&first_free_block, size, align);
	if (prev == NULL || *prev == NULL) {
		pr_err("Cannot allocate %zu bytes of physical memory: "
		       "out of memory\n", size);
	}

	void * paddr = aligned((*prev)->addr, align);

	err = pmap(paddr, size);
	if (err)
		return NULL;
	pr_debug("pmalloc(%zu, %zu) -> %p\n", size, align, paddr);
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

	err = memblock_rem(&first_used_block, paddr, size, true);
	if (err) {
		pr_err("Cannot free %zu bytes of physical memory at %p: "
		       "memory is not used, errno = %d\n", size, paddr, err);
		memblock_rem(&first_used_block, paddr, size, false);
		pr_crit("Lost up to %zu bytes of physical memory at %p\n",
			size, paddr);
		return;
	}

	err = memblock_add(&first_free_block, paddr, size, true);
	if (err) {
		pr_err("Cannot free %zu bytes of physical memory at %p: "
		       "memory is already free, errno = %d\n",
		       size, paddr, err);
		return;
	}
	pr_debug("pfree(%p, %zu)\n", paddr, size);
}
