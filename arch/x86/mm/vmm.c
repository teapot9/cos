#define pr_fmt(fmt) "vmm: " fmt

#include <mm.h>
#include "vmm.h"

#include <errno.h>

#include <asm/cpu.h>
#include "../../../mm/pmm.h"
#include <print.h>

void * get_paddr(union pml4e * pml4, void * ptr)
{
	union lin_addr * addr = (union lin_addr *) &ptr;
	struct page pg = get_page(pml4, ptr);
	unsigned long int ret = (unsigned long) NULL;

	switch (pg.type) {
	case PAGE_TYPE_PML4:
		break;
	case PAGE_TYPE_PDPT:
		if (pg.page.pdpt->absent.present)
			ret = pg.page.pdpt->page.page << 30 | addr->pdpt.offset;
		break;
	case PAGE_TYPE_PD:
		if (pg.page.pd->absent.present)
			ret = pg.page.pd->page.page << 21 | addr->pd.offset;
		break;
	case PAGE_TYPE_PT:
		if (pg.page.pt->absent.present)
			ret = pg.page.pt->page.page << 12 | addr->pt.offset;
		break;
	}
	return (void *) ret;
}

struct page get_page(union pml4e * pml4, void * addr)
{
	union lin_addr * a = (union lin_addr *) &addr;
	union pml4e * pml4e = &pml4[a->pt.pml4];
	if (!pml4e->absent.present)
		return (struct page) {
			.type = PAGE_TYPE_PML4,
			.page.pml4 = pml4e,
		};

	//union pdpte * pdpt = (void *) (pml4e->pdpt.pdpt << (64 - 40));
	union pdpte * pdpt = (void *) (pml4e->pdpt.pdpt << 12);
	union pdpte * pdpte = &pdpt[a->pt.pdpt];
	if (!pdpte->absent.present || pdpte->page.page_size)
		return (struct page) {
			.type = PAGE_TYPE_PDPT,
			.page.pdpt = pdpte,
		};

	//union pde * pd = (void *) (pdpte->pd.pd << (64 - 40));
	union pde * pd = (void *) (pdpte->pd.pd << 12);
	union pde * pde = &pd[a->pt.pd];
	if (!pde->absent.present || pde->page.page_size)
		return (struct page) {
			.type = PAGE_TYPE_PD,
			.page.pd = pde,
		};

	//union pte * pt = (void *) (pde->pt.pt << (64 - 40));
	union pte * pt = (void *) (pde->pt.pt << 12);
	union pte * pte = &pt[a->pt.pt];
	return (struct page) {
		.type = PAGE_TYPE_PT,
		.page.pt = pte,
	};
}

static bool is_free_page_pt(union pml4e * pml4, void * addr, size_t n)
{
	if (!n)
		return true;

	struct page pg = get_page(pml4, addr);
	switch (pg.type) {
	case PAGE_TYPE_PML4:
		if (!pg.page.pml4->absent.present) {
			size_t inc = PAGE_SIZE_PML4;
			if (n <= inc / PAGE_SIZE_PT)
				return true;
			else
				return is_free_page_pt(
					pml4, (uint8_t *) addr + inc,
					n - inc / PAGE_SIZE_PT
				);
		}
		pr_crit("PML4.present = 1 should not happen here\n", 0);
		return false;
	case PAGE_TYPE_PDPT:
		if (!pg.page.pdpt->absent.present) {
			size_t inc = PAGE_SIZE_PDPT;
			if (n <= inc / PAGE_SIZE_PT)
				return true;
			else
				return is_free_page_pt(
					pml4, (uint8_t *) addr + inc,
					n - inc / PAGE_SIZE_PT
				);
		}
		if (!pg.page.pdpt->page.page_size)
			pr_crit("PDPT.page_size = 0 should not happen here\n",
				0);
		return false;
	case PAGE_TYPE_PD:
		if (!pg.page.pd->absent.present) {
			size_t inc = PAGE_SIZE_PD;
			if (n <= inc / PAGE_SIZE_PT)
				return true;
			else
				return is_free_page_pt(
					pml4, (uint8_t *) addr + inc,
					n - inc / PAGE_SIZE_PT
				);
		}
		if (!pg.page.pd->page.page_size)
			pr_crit("PD.page_size = 0 should not happen here\n", 0);
		return false;
	case PAGE_TYPE_PT:
		if (!pg.page.pt->absent.present) {
			size_t inc = PAGE_SIZE_PT;
			if (n <= inc / PAGE_SIZE_PT)
				return true;
			else
				return is_free_page_pt(
					pml4, (uint8_t *) addr + inc,
					n - inc / PAGE_SIZE_PT
				);
		}
		return false;
	}
}

static void * find_free_page_pt(union pml4e * pml4, size_t n)
{
	void * addr = 0;

	while (!is_free_page_pt(pml4, addr, n) && (uint8_t *) addr
	       < (uint8_t *) VIRTUAL_MEMORY_SIZE - PAGE_SIZE_PT - 1)
		addr = (uint8_t *) addr + PAGE_SIZE_PT;
	return addr;
}

static int map_page_pt(union pml4e * pml4, void * paddr, void * vaddr)
{
	//pr_debug("map_page_pt(paddr=%p, vaddr=%p)\n", paddr, vaddr);
	struct page pg = get_page(pml4, vaddr);

	switch (pg.type) {
	case PAGE_TYPE_PML4:
		if (pg.page.pml4->absent.present)
			return -EINVAL;
		union pdpte * pdpt = pmalloc(NB_PDPT_ENTRY * sizeof(*pdpt),
					     PAGE_SIZE_PT);
		if (pdpt == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PDPT_ENTRY; i++)
			pdpt[i].absent.present = false;

		struct pml4_pdpt pml4e = {
			.present = true,
			.write_access = true,
			.user_access = false,
			.pwt = false,
			.pcd = false,
			.accessed = false,
			.page_size = false,
			.pdpt = (unsigned long int) pdpt >> 12,
			.xd = false,
		};
		pg.page.pml4->pdpt = pml4e;

		map_page_pt(pml4, pdpt, pdpt); // TODO: error?
		return map_page_pt(pml4, paddr, vaddr);
		pg = get_page(pml4, vaddr);
		if (pg.type != PAGE_TYPE_PDPT)
			return -EINVAL;
		// fallthrough
	case PAGE_TYPE_PDPT:
		if (pg.page.pdpt->absent.present)
			return -EINVAL;
		union pde * pd = pmalloc(NB_PD_ENTRY * sizeof(*pd),
					 PAGE_SIZE_PT);
		if (pd == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PD_ENTRY; i++)
			pd[i].absent.present = false;

		struct pdpt_pd pdpte = {
			.present = true,
			.write_access = true,
			.user_access = false,
			.pwt = false,
			.pcd = false,
			.accessed = false,
			.page_size = false,
			.pd = (unsigned long int) pd >> 12,
			.xd = false,
		};
		pg.page.pdpt->pd = pdpte;

		map_page_pt(pml4, pd, pd); // TODO: error?
		return map_page_pt(pml4, paddr, vaddr);
		pg = get_page(pml4, vaddr);
		if (pg.type != PAGE_TYPE_PD)
			return -EINVAL;
		// fallthrough
	case PAGE_TYPE_PD:
		if (pg.page.pd->absent.present)
			return -EINVAL;
		union pte * pt = pmalloc(NB_PT_ENTRY * sizeof(*pt),
					 PAGE_SIZE_PT);
		if (pt == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PT_ENTRY; i++)
			pt[i].absent.present = false;

		struct pd_pt pde = {
			.present = true,
			.write_access = true,
			.user_access = false,
			.pwt = false,
			.pcd = false,
			.accessed = false,
			.page_size = false,
			.pt = (unsigned long int) pt >> 12,
			.xd = false,
		};
		pg.page.pd->pt = pde;

		map_page_pt(pml4, pt, pt); // TODO: error?
		return map_page_pt(pml4, paddr, vaddr);
		pg = get_page(pml4, vaddr);
		if (pg.type != PAGE_TYPE_PT)
			return -EINVAL;
		// fallthrough
	case PAGE_TYPE_PT:
		if (pg.page.pt->absent.present)
			return -EINVAL;

		struct pt_page pte = {
			.present = true,
			.write_access = true,
			.user_access = false,
			.pwt = false,
			.pcd = false,
			.accessed = false,
			.dirty = false,
			.pat = false,
			.global = false,
			.page = (unsigned long int) paddr >> 12,
			.protection_key = 0,
			.xd = false,
		};
		pg.page.pt->page = pte;
	}
	return 0;
}

static int map_pages_pt(union pml4e * pml4, void * paddr, void * vaddr,
                        size_t size)
{
#ifdef CONFIG_MM_DEBUG
	pr_debug("map_pages_pt(paddr=%p, vaddr=%p, size=%zu)\n", paddr, vaddr, size);
#endif
	int ret;
	size_t nb_pages = size / PAGE_SIZE_PT;
	if (size % PAGE_SIZE_PT)
		nb_pages++;

	for (size_t i = 0; i < nb_pages; i++)
		if ((ret = map_page_pt(
			pml4, (uint8_t *) paddr + PAGE_SIZE_PT * i,
			(uint8_t *) vaddr + PAGE_SIZE_PT * i
		)))
			return ret;
	return 0;
}

static void unmap_page_pt(union pml4e * pml4, void * addr)
{
	struct page pg = get_page(pml4, addr);

	switch (pg.type) {
	case PAGE_TYPE_PML4:
		break;
	case PAGE_TYPE_PDPT:
		if (pg.page.pdpt->absent.present
		    && pg.page.pdpt->page.page_size) {
			pfree(get_paddr(pml4, addr), PAGE_SIZE_PT);
			pg.page.pdpt->page.present = false;
		}
		break;
	case PAGE_TYPE_PD:
		if (pg.page.pd->absent.present
		    && pg.page.pd->page.page_size) {
			pfree(get_paddr(pml4, addr), PAGE_SIZE_PT);
			pg.page.pd->page.present = false;
		}
		break;
	case PAGE_TYPE_PT:
		if (pg.page.pt->absent.present) {
			pfree(get_paddr(pml4, addr), PAGE_SIZE_PT);
			pg.page.pt->page.present = false;
		}
		break;
	}
}

static void unmap_pages_pt(union pml4e * pml4, void * addr, size_t n)
{
	for (size_t i = 0; i < n; i++)
		unmap_page_pt(pml4, (uint8_t *) addr + i * PAGE_SIZE_PT);
}

/* public: mm.h */
int vmap(void * paddr, void * vaddr, size_t size)
{
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL)
		return early_vmap(paddr, vaddr, size);
	int err = map_pages_pt(pml4, paddr, vaddr, size);
#ifdef CONFIG_MM_DEBUG
	pr_debug("vmap(%p, %p, %zu) -> %d\n", paddr, vaddr, size, err);
#endif
	return err;
}

/* public: mm.h */
struct memblock vmalloc(size_t size)
{
	struct memblock null_vmalloc = {.addr = NULL, .size = 0};
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL) {
		pr_err("Cannot allocate virtual memory: not initialized\n", 0);
		return null_vmalloc;
	}

	size_t nb_pages = size / 4096 + (size % 4096 != 0);

	void * vaddr = find_free_page_pt(pml4, nb_pages);
	if (vaddr == NULL) {
		pr_err("Cannot find %zu bytes of free virtual memory\n", size);
		return null_vmalloc;
	}

	uint8_t * ivaddr = vaddr;
	size_t allocated = 0;
	for (size_t i = 0; i < nb_pages; i++) {
		void * paddr = pmalloc(PAGE_SIZE_PT, PAGE_SIZE_PT);
		if (paddr == NULL)
			goto paddr_failed;
		map_page_pt(pml4, paddr, vaddr);
		ivaddr += PAGE_SIZE_PT;
		allocated += PAGE_SIZE_PT;
	}

#ifdef CONFIG_MM_DEBUG
	pr_debug("vmalloc(%zu) -> {%p, %zu}\n", size, vaddr, allocated);
#endif
	return (struct memblock) {.addr = vaddr, .size = allocated};

paddr_failed:
	pr_err("Out of phisical memory, cannot allocate %zu bytes\n", size);
	ivaddr = vaddr;
	for (size_t i = 0; i < nb_pages; i++) {
		vfree(ivaddr, PAGE_SIZE_PT);
		ivaddr += PAGE_SIZE_PT;
	}
	return null_vmalloc;
}

/* public: mm.h */
void vunmap(void * vaddr, size_t size)
{
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL) {
		early_vunmap(vaddr, size);
		return;
	}

	size_t nb_pages = size / 4096 + (size % 4096 != 0);
	unmap_pages_pt(pml4, vaddr, nb_pages);
#ifdef CONFIG_MM_DEBUG
	pr_debug("vunmap(%p, %zu)\n", vaddr, size);
#endif
}

/* public: mm.h */
void vfree(void * vaddr, size_t size)
{
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL) {
		pr_alert("Cannot free virtual memory: not initialized\n", 0);
		pr_alert("This should never have happened\n", 0);
		return;
	}

	size_t nb_pages = size / 4096 + (size % 4096 != 0);

	vunmap(vaddr, size);

	uint8_t * ivaddr = vaddr;
	for (size_t i = 0; i < nb_pages; i++) {
		pfree(get_paddr(pml4, ivaddr), PAGE_SIZE_PT);
		ivaddr += PAGE_SIZE_PT;
	}
#ifdef CONFIG_MM_DEBUG
	pr_debug("vfree(%p, %zu)\n", vaddr, size);
#endif
}

/* public: mm.h */
void * mmap(void * paddr, size_t size)
{
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL)
		return early_mmap(paddr, size);

	size_t nb_pages = size / 4096 + (size % 4096 != 0);
	int err;

	err = pmap(paddr, size);
	if (err)
		return NULL;

	void * vaddr = find_free_page_pt(pml4, nb_pages);
	if (vaddr == NULL) {
		pr_err("Cannot find %zu bytes of free virtual memory\n", size);
		pfree(paddr, size);
		return NULL;
	}

	err = map_pages_pt(pml4, paddr, vaddr, size);
	if (err) {
		pr_err("Cannot map %p to %p (%zu bytes)\n", vaddr, paddr, size);
		pfree(paddr, size);
		return NULL;
	}

#ifdef CONFIG_MM_DEBUG
	pr_debug("mmap(%p, %zu) -> %p\n", paddr, size, vaddr);
#endif
	return vaddr;
}

/* public: mm.h */
void * virt_to_phys(void * vaddr)
{
	union pml4e * pml4 = kpml4();
	if (pml4 == NULL)
		return NULL;
	return get_paddr(pml4, vaddr);
}
