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
#include <mm/block.h>
#include <panic.h>

static bool is_init = false;
static struct memblock_list * first_acpi_block = NULL;
static struct memblock_list * first_early_pmap = NULL;
static struct memblock_list * first_early_pfree = NULL;

int early_pmap(void * paddr, size_t size)
{
	struct memblock_list * new = kmalloc(sizeof(*new));
	if (new == NULL)
		return -ENOMEM;

	new->addr = paddr;
	new->size = size;
	new->next = first_early_pmap;
	first_early_pmap = new;

	// int err = register_used_pmem(paddr, size);
#ifdef CONFIG_MM_DEBUG
	pr_debug("early_pmap(%p, %zu) -> %d\n", paddr, size, err);
#endif
	// return err;
	return 0;
}

void * early_pmalloc(UNUSED size_t size, UNUSED size_t align)
{
	pr_err("early_pmalloc: unavailable during early boot\n", 0);
	return NULL;
}

void early_pfree(void * paddr, size_t size)
{
	struct memblock_list * new = kmalloc(sizeof(*new));
	if (new == NULL)
		return;

	new->addr = paddr;
	new->size = size;
	new->next = first_early_pfree;
	first_early_pfree = new;

#if 0
	int err;
	err = memblock_rem(&first_used_block, paddr, size, false);
	if (err) {
		pr_err("cannot free %zu bytes of physical memory at %p: "
		       "memory is not used, errno = %d\n", size, paddr, err);
		return;
	}
#endif
#ifdef CONFIG_MM_DEBUG
	pr_debug("early_pfree(%p, %zu)\n", paddr, size);
#endif
}

bool pmm_is_init(void)
{
	return is_init;
}

/* public: setup.h */
int pmm_init(void)
{
	struct memblock_list * cur;
	int err = 0;
	is_init = true;

	cur = first_early_pmap;
	while (cur != NULL) {
		struct memblock_list * prev = cur;
		int tmp_err = pmap(cur->addr, cur->size);
		if (tmp_err) {
			err = tmp_err;
			panic("failed to register early pmap at %p [%zuB], "
			      "errno = %d\n", cur->addr, cur->size, err);
		}
		cur = cur->next;
		kfree(prev);
	}

	cur = first_early_pfree;
	while (cur != NULL) {
		struct memblock_list * prev = cur;
		pfree(cur->addr, cur->size);
		cur = cur->next;
		kfree(prev);
	}

	return err;
}

/* public: mm/early.h */
int register_used_pmem(void * paddr, size_t size)
{
	if (pmm_is_init())
		return -EINVAL;

	struct memblock_list ** prev = memblock_search(
		&first_free_block, paddr, size, false
	);
	if (prev != NULL && *prev != NULL
	    && is_overlap((*prev)->addr, (*prev)->size, paddr, size)) {
	    //&& !is_next((*prev)->addr, (*prev)->size, paddr, size)) {
		pr_err("cannot define %zu bytes of used memory at %p: "
		       "memory is defined as free\n", size, paddr);
		return -EINVAL;
	}

	int err = memblock_add(&first_used_block, paddr, size, false);
	if (err) {
		pr_err("cannot define %zu bytes of used memory at %p: "
		       "errno = %d\n", size, paddr, err);
		return err;
	}

#ifdef CONFIG_MM_DEBUG
	pr_debug("register_used_pmem(%p, %zu) -> %d\n", paddr, size, 0);
#endif
	return 0;
}

/* public: mm/early.h */
int register_free_pmem(void * paddr, size_t size)
{
	if (pmm_is_init())
		return -EINVAL;

	struct memblock_list ** prev = memblock_search(
		&first_used_block, paddr, size, false
	);
	if (prev != NULL && *prev != NULL
	    && is_overlap((*prev)->addr, (*prev)->size, paddr, size)) {
	    //&& !is_next((*prev)->addr, (*prev)->size, paddr, size)) {
		pr_err("cannot define %zu bytes of free memory at %p: "
		       "memory is defined as used\n", size, paddr);
		return -EINVAL;
	}

	int err = memblock_add(&first_free_block, paddr, size, false);
	if (err) {
		pr_err("cannot define %zu bytes of free memory at %p: "
		       "errno = %d\n", size, paddr, err);
		return err;
	}

#ifdef CONFIG_MM_DEBUG
	pr_debug("register_free_pmem(%p, %zu) -> %d\n", paddr, size, 0);
#endif
	return 0;
}

/* public: mm/early.h */
int pmm_acpi_register(void * paddr, size_t size)
{
	int err;
	if (pmm_is_init())
		return -EINVAL;

	//err = register_used_pmem(paddr, size);
	//if (err)
	//	goto exit_err;

	err = memblock_add(&first_acpi_block, paddr, size, false);
	if (err)
		goto exit_err;

#ifdef CONFIG_MM_DEBUG
	pr_debug("pmm_acpi_register(%p, %zu) -> %d\n", paddr, size, 0);
#endif
	return 0;
exit_err:
	pr_err("cannot define %zu bytes of ACPI memory at %p, "
	       "errno = %d\n", size, paddr, err);
	return err;
}

/* public: mm/early.h */
bool pmm_acpi_iter(struct memblock * prev)
{
	struct memblock_list * next = NULL;

	if (prev->addr == NULL) {
		next = first_acpi_block;
	} else {
		struct memblock_list ** pprev = memblock_search(
			&first_acpi_block, prev->addr, prev->size, true
		);
		if (pprev != NULL) {
			if (*pprev != NULL)
				next = (*pprev)->next;
			else
				next = NULL;
		}
	}

	if (next == NULL)
		return false;
	prev->addr = next->addr;
	prev->size = next->size;
	return true;
}

/* public: mm/early.h */
void pmm_acpi_free(void)
{
	struct memblock_list * cur = first_acpi_block;
	while (cur != NULL)
		pfree(cur->addr, cur->size);
	memblock_free(&first_acpi_block);
#ifdef CONFIG_MM_DEBUG
	pr_debug("pmm_acpi_free()\n", 0);
#endif
}
