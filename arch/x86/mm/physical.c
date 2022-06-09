#include "physical.h"

#include <assert.h>

#include <mm/pmm.h>
#include <alloc.h>
#include <cpp.h>
#include "vmm.h"
#include "paging.h"
#include <errno.h>
#include <memlist.h>
#include <mm/memmap.h>
#include <print.h>

void * const physical_map = KERNEL_SPACE_START;

void * physical_tmp_map(void * paddr)
{
#ifndef BOOTLOADER
	if (!vmm_paging_not_identity || paddr == NULL)
#endif
		return paddr;
	return (uint8_t *) physical_map + (size_t) paddr;
}

void physical_tmp_unmap(UNUSED void * vaddr)
{
	// noop
}

static int pinit(union pml4e * pml4, void * vaddr, void * paddr)
{
	/* This function expects memory to be identity mapped */
	int err;
	union linear_addr * laddr = (void *) &vaddr;

	if (laddr->pdpt.offset)
		return -EINVAL;

	union pml4e * pml4e = &pml4[laddr->pdpt.pml4];
	if (!pml4e->absent.p) {
		void * new_pdpt =
			palloc(0, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
		if (new_pdpt == NULL)
			return -ENOMEM;
		table_init_pdpt(new_pdpt);
		err = table_add_pdpt(pml4e, new_pdpt, true, false, false);
		if (err)
			return err;
	}

	union pdpte * pdpt = get_pdpt(pml4e);
	if (pdpt == NULL)
		return -ENOMEM;

	union pdpte * pdpte = &pdpt[laddr->pdpt.pdpt];
	if (pdpte->absent.p)
		return -EBUSY;
	return page_map_pdpt(pdpte, paddr);
}

int physical_init(union pml4e * pml4)
{
	/* This function expects memory to be identity mapped */
	int err;
	size_t memory = memlist_virtual_size(&memmap.l);
	uint8_t * paddr = 0;
	uint8_t * vaddr = physical_map;

	while (paddr < (uint8_t *) memory) {
		if ((err = pinit(pml4, vaddr, paddr))) {
			pr_crit("cannot map %p to %p", paddr, vaddr);
			return err;
		}
		vaddr = vaddr == 0
			? (void *) PAGE_SIZE_PDPT : vaddr + PAGE_SIZE_PDPT;
		paddr = paddr == 0
			? (void *) PAGE_SIZE_PDPT : paddr + PAGE_SIZE_PDPT;
	}
	return 0;
}
