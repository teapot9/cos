#include <mm.h>
#include "pmm.h"

#include <stdint.h>
#include <errno.h>

#include <cpp.h>
#include <print.h>
#include "helper.h"
#include "block.h"

static bool is_init = false;

int early_pmap(UNUSED void * paddr, UNUSED size_t size)
{
	int err = register_used_pmem(paddr, size);
	pr_debug("early_pmal(%p, %zu) -> %d\n", paddr, size, err);
	return err;
}

void * early_pmalloc(UNUSED size_t size, UNUSED size_t align)
{
	pr_err("early_pmalloc: unavailable during early boot\n", 0);
	return NULL;
}

void early_pfree(UNUSED void * paddr, UNUSED size_t size)
{
	int err;

	err = memblock_rem(&first_used_block, paddr, size, false);
	if (err) {
		pr_err("Cannot free %zu bytes of physical memory at %p: "
		       "memory is not used, errno = %d\n", size, paddr, err);
		return;
	}
	pr_debug("early_pfree(%p, %zu)\n", paddr, size);
}

bool pmm_is_init(void)
{
	return is_init;
}

int pmm_init(void)
{
	is_init = true;
	return 0;
}

int register_used_pmem(void * paddr, size_t size)
{
	if (pmm_is_init())
		return -EINVAL;

	struct memblock ** prev = memblock_search(
		&first_free_block, paddr, size, false
	);
	if (prev != NULL && *prev != NULL
	    && is_overlap((*prev)->addr, (*prev)->size, paddr, size)) {
	    //&& !is_next((*prev)->addr, (*prev)->size, paddr, size)) {
		pr_err("Cannot define %zu bytes of used memory at %p: "
		       "memory is defined as free\n", size, paddr);
		return -EINVAL;
	}

	int err = memblock_add(&first_used_block, paddr, size, false);
	if (err) {
		pr_err("Cannot define %zu bytes of used memory at %p: "
		       "errno = %d\n", size, paddr, err);
		return err;
	}

	pr_debug("register_used_pmem(%p, %zu) -> %d\n", paddr, size, 0);
	return 0;
}

int register_free_pmem(void * paddr, size_t size)
{
	if (pmm_is_init())
		return -EINVAL;

	struct memblock ** prev = memblock_search(
		&first_used_block, paddr, size, false
	);
	if (prev != NULL && *prev != NULL
	    && is_overlap((*prev)->addr, (*prev)->size, paddr, size)) {
	    //&& !is_next((*prev)->addr, (*prev)->size, paddr, size)) {
		pr_err("Cannot define %zu bytes of free memory at %p: "
		       "memory is defined as used\n", size, paddr);
		return -EINVAL;
	}

	int err = memblock_add(&first_free_block, paddr, size, false);
	if (err) {
		pr_err("Cannot define %zu bytes of free memory at %p: "
		       "errno = %d\n", size, paddr, err);
		return err;
	}

	pr_debug("register_free_pmem(%p, %zu) -> %d\n", paddr, size, 0);
	return 0;
}
