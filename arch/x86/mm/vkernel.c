#include <mm/vmm.h>
#include "vkernel.h"
#include "paging.h"
#include "physical.h"
#include <errno.h>
#include <asm/cpu.h>
#include <mm/paging.h>
#include <cpp.h>

#define KERNEL_PDPT_START 256
#define KERNEL_PDPT_COUNT 256

struct full_pml4 {
	union pml4e _aligned_(PAGE_TABLE_SIZE) entries[PAGE_TABLE_ELEMENTS];
};
struct full_pdpt {
	union pdpte _aligned_(PAGE_TABLE_SIZE) entries[PAGE_TABLE_ELEMENTS];
};

static struct full_pml4 _aligned_(PAGE_TABLE_SIZE) kpml4;
static struct full_pdpt _aligned_(PAGE_TABLE_SIZE) kpdpt[KERNEL_PDPT_COUNT];

int vkernel_map_kmem(union pml4e * pml4) {
	int err;
	for (size_t k = 0; k < KERNEL_PDPT_COUNT; k++) {
		union pml4e * pml4e = &pml4[KERNEL_PDPT_START + k];
		void * pdpt = virt_to_phys_current(kpdpt[k].entries);
		if (pdpt == NULL)
			return -ENOMEM;
		err = table_add_pdpt(pml4e, pdpt, true, false, true);
		if (err)
			return err;
	}
	return 0;
}

static int kernel_cr3_init(void) {
	/* This function expect virtual memory to be identity mapped */
	void * pml4 = virt_to_phys_current(kpml4.entries);
	if (pml4 == NULL)
		return -ENOMEM;

	int err = table_add_pml4((union cr3 *) &kernel_cr3, pml4);
	if (err)
		return err;

	return 0;
}

static int clone_kmem(void) {
	/* This function expect virtual memory to be identity mapped */
	uint64_t raw_cur_cr3 = read_cr3();
	union pml4e * cur_pml4 = get_pml4((void *) &raw_cur_cr3);

	for (size_t k = 0; k < KERNEL_PDPT_COUNT; k++) {
		union pml4e * cur_pml4e = &cur_pml4[KERNEL_PDPT_START + k];
		union pml4e * new_pml4e = &kpml4.entries[KERNEL_PDPT_START + k];
		union pdpte * cur_pdpt = get_pdpt(cur_pml4e);
		if (cur_pdpt == NULL)
			continue;
		union pdpte * new_pdpt = get_pdpt(new_pml4e);
		if (new_pdpt == NULL)
			continue;

		for (size_t l = 0; l < PAGE_TABLE_ELEMENTS; l++) {
			union pdpte * cur_pdpte = &cur_pdpt[l];
			union pdpte * new_pdpte = &new_pdpt[l];
			*new_pdpte = *cur_pdpte;
		}
	}
	return 0;
}

int vkernel_init(void) {
	/* This function expect virtual memory to be identity mapped */
	int err = kernel_cr3_init();
	if (err)
		return err;

	table_init_pml4(kpml4.entries);
	for (size_t k = 0; k < KERNEL_PDPT_COUNT; k++)
		table_init_pdpt(kpdpt[k].entries);

	err = vkernel_map_kmem(kpml4.entries);
	if (err)
		return err;

#ifndef BOOTLOADER
	err = clone_kmem();
	if (err)
		return err;
#endif

	return 0;
}

int vkernel_identity_paging(void) {
	/* This function expect virtual memory to be identity mapped */
	static struct full_pdpt _aligned_(PAGE_TABLE_SIZE) identity;
	int err;

	/* Create PDPT0 */
	table_init_pdpt(identity.entries);
	void * phys_identity_pdpt = virt_to_phys_current(identity.entries);
	err = table_add_pdpt(&kpml4.entries[0], phys_identity_pdpt,
			     true, false, true);
	if (err)
		return err;

	/* 1:1 mapping for entire PDPT */
	uint8_t * paddr = 0;
	for (size_t k = 0; k < PAGE_TABLE_ELEMENTS; k++) {
		err = page_map_pdpt(&identity.entries[k], paddr);
		if (err)
			return err;
		err = page_set_pdpt(&identity.entries[k], true, false, true,
		                    false, false);
		if (err)
			return err;
		if (paddr == 0) // special case to avoid adding to NULL pointer
			paddr = (void *) PAGE_SIZE_PDPT;
		else
			paddr += PAGE_SIZE_PDPT;
	}
	return 0;
}
