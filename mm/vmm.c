#include <mm.h>
#include "vmm.h"

#include <errno.h>

#include <asm/cpu.h>
#include "pmm.h"
#include <print.h>

#if 0
struct page get_page(const union cr3 cr3, void * ptr)
{
	union lin_addr * addr = (union lin_addr *) &ptr;

	unsigned long int pml4_int;
	union pml4e * pml4;
	if (read_cr4() & (1 << 17))
		pml4_int = (cr3.pcide.pml4 << 12);
	else
		pml4_int = (cr3.normal.pml4 << 12);
	//pml4 |= (ptr & (0x1FF << 39)) >> (39 - 3);
	pml4_int |= (addr->pt.pml4 << 3);
	pml4 = (void *) pml4_int;

	//union pdpte * pdpte = pml4->pml4_pdpt.pdpt << 12
	//	| (ptr & (0x1FF << 30)) >> (30 - 3);
	unsigned long int pdpte_int = pml4->pdpt.pdpt << 12
		| (addr->pt.pdpt << 3);
	union pdpte * pdpte = (void *) pdpte_int;

	if (pdpte->page.page_size)
		return (struct page) {
			.type = PAGE_TYPE_PDPT,
			.page.pdpt = &pdpte->page,
		};
	//union pde * pde = pdpte->pdpte_pd.pd << 12
	//	| (ptr & (0x1FF << 21)) >> (21 - 3);
	unsigned long int pde_int = pdpte->pd.pd << 12
		| (addr->pt.pd << 3);
	union pde * pde = (void *) pde_int;

	if (pde->page.page_size)
		return (struct page) {
			.type = PAGE_TYPE_PD,
			.page.pd = &pde->page,
		};
	//union pte * pte = pde->pt.pt << 12
	//	| (ptr & (0x1FF << 12)) >> (12 - 3);
	unsigned long int pte_int = pde->pt.pt << 12
		| (addr->pt.pt << 3);
	union pte * pte = (void *) pte_int;

	return (struct page) {
		.type = PAGE_TYPE_PT,
		.page.pt = &pte->page,
	};
}
#endif

void * get_paddr(union pml4e * pml4, void * ptr)
{
	union lin_addr * addr = (union lin_addr *) &ptr;
	struct page pg = get_page(pml4, ptr);
	unsigned long int ret = 0;

	switch (pg.type) {
	case PAGE_TYPE_PML4:
		break;
	case PAGE_TYPE_PDPT:
		ret = pg.page.pdpt->page.page << 30 | addr->pdpt.offset;
		break;
	case PAGE_TYPE_PD:
		ret = pg.page.pd->page.page << 21 | addr->pd.offset;
		break;
	case PAGE_TYPE_PT:
		ret = pg.page.pt->page.page << 12 | addr->pt.offset;
		break;
	}
	return (void *) ret;
}

#define X86_64_MSR_EFER 0xC0000080

static inline void disable_paging(void)
{
	write_cr0(read_cr0() & ~(1 << 31)); // Clear CR0.PG
	write_cr4(read_cr0() & ~(1 << 5)); // Clear CR4.PAE
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) & ~(1 << 8)); // Clear EFER.LME
}

static inline void enable_paging(void)
{
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) | (1 << 8)); // Set EFER.LME
	write_cr4(read_cr0() | (1 << 5)); // Set CR4.PAE
	write_cr0(read_cr0() | (1 << 31)); // Set CR0.PG
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

#if 0
static bool is_free_page_pt(struct pml4e * pml4, void * addr, size_t n)
{
	if (!n)
		return true;

	union lin_addr * a = &addr;
	union pml4e * pml4e = pml4[a->pt.pml4];
	if (!pml4e->pml4_absent.present)
		return is_free_page_pt(
			pml4, (uint8_t *) addr + PAGE_SIZE_PML4,
			n - PAGE_SIZE_PML4 / PAGE_SIZE_PT
		);

	union pdpte * pdpt = pml4e->pdpt.pdpt << 64 - 40;
	union pdpte * pdpte = pdpt[a->pt.pdpt];
	if (!pdpte->pdpt_absent.present)
		return is_free_page_pt(
			pml4, (uint8_t *) addr + PAGE_SIZE_PDPT,
			n - PAGE_SIZE_PDPT / PAGE_SIZE_PT
		);
	if (pdpte->page.page_size)
		return false;

	union pde * pd = pdpte->pd.pd << 64 - 40;
	union pde * pde = pd[a->pt.pd];
	if (!pde->pd_absent.present)
		return is_free_page_pt(
			pml4, (uint8_t *) addr + PAGE_SIZE_PD,
			n - PAGE_SIZE_PD / PAGE_SIZE_PT
		);
	if (pde->page.page_size)
		return false;

	union pte * pt = pde->pt.pt << 64 - 40;
	union pte * pte = pt[a->pt.pt];
	if (!pte->pt_absent.present)
		return is_free_page_pt(
			pml4, (uint8_t *) addr + PAGE_SIZE_PT,
			n - PAGE_SIZE_PT / PAGE_SIZE_PT
		);
	return false;
}
#endif

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

struct mmap mmap(union pml4e * pml4, void * addr, size_t size)
{
	size_t nb_pages = size / 4096 + (size % 4096 != 0);
	struct mmap map = (struct mmap) {.ptr = NULL, .size = 0};

	if (addr == NULL)
		addr = find_free_page_pt(pml4, nb_pages);
	if (addr == NULL)
		return map; // Out of virtual memory

	uint8_t * vptr = addr;
	for (size_t i = 0; i < nb_pages; i++) {
		void * pptr = pmalloc(PAGE_SIZE_PT, PAGE_SIZE_PT);
		if (pptr == NULL)
			goto pptr_failed;
		map_page_pt(pml4, pptr, vptr);
		vptr += PAGE_SIZE_PT;
	}

	map.ptr = addr;
	map.size = nb_pages * 4096;
	return map;

pptr_failed:
	pr_err("Out of physical memory, cannot allocate %zu bytes", size);
	vptr = addr;
	for (size_t i = 0; i < nb_pages; i++) {
		vfree(vptr, PAGE_SIZE_PT);
		vptr += PAGE_SIZE_PT;
	}
	return map;
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

void vfree(void * addr, size_t len)
{
	size_t n = len / PAGE_SIZE_PT;
	uint64_t raw_cr3 = read_cr3();
	union cr3 * cr3 = (void *) &raw_cr3;
	unmap_pages_pt((union pml4e *) cr3->normal.pml4, addr, n);
}

struct mmap vmalloc(size_t size)
{
	uint64_t raw_cr3 = read_cr3();
	union cr3 * cr3 = (void *) &raw_cr3;
	return mmap((union pml4e *) cr3->normal.pml4, NULL, size);
}

static void print_vmmap_pt(union pte * pt, uint8_t * base)
{
	for (size_t i = 0; i < 512; i++, base += 4096) {
		if (!pt[i].absent.present)
			continue;
		pr_debug(
			"pt[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d d=%d "
			"pat=%d g=%d page=%p pk=%d xd=%d (%p)\n",
			i,
			pt[i].page.write_access,
			pt[i].page.user_access,
			pt[i].page.pwt,
			pt[i].page.pcd,
			pt[i].page.accessed,
			pt[i].page.dirty,
			pt[i].page.pat,
			pt[i].page.global,
			pt[i].page.page,
			pt[i].page.protection_key,
			pt[i].page.xd,
			base
		);
	}
}

static void print_vmmap_pd(union pde * pd, uint8_t * base)
{
	for (size_t i = 0; i < 512; i++, base += 2*1024*1024) {
		if (!pd[i].absent.present)
			continue;
		pr_debug(
			"pd[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d ",
			i,
			pd[i].page.write_access,
			pd[i].page.user_access,
			pd[i].page.pwt,
			pd[i].page.pcd,
			pd[i].page.accessed
		);
		if (pd[i].page.page_size) {
			pr_debug(
				"d=%d ps=1 g=%d pat=%d page=%p pk=%d xd=%d "
				"(%p)\n",
				pd[i].page.dirty,
				pd[i].page.global,
				pd[i].page.pat,
				pd[i].page.page,
				pd[i].page.protection_key,
				pd[i].page.xd,
				base
			);
			continue;
		}
		pr_debug(
			"ps=0 pt=%p xd=%d\n",
			pd[i].pt.pt,
			pd[i].pt.xd
		);
		union pte * pt = (void *) (pd[i].pt.pt << 12);
		print_vmmap_pt(pt, base);
	}
}

static void print_vmmap_pdpt(union pdpte * pdpt, uint8_t * base)
{
	for (size_t i = 0; i < 512; i++, base += 1024*1024*1024) {
		if (!pdpt[i].absent.present)
			continue;
		pr_debug(
			"pdpt[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d ",
			i,
			pdpt[i].page.write_access,
			pdpt[i].page.user_access,
			pdpt[i].page.pwt,
			pdpt[i].page.pcd,
			pdpt[i].page.accessed
		);
		if (pdpt[i].page.page_size) {
			pr_debug(
				"d=%d ps=1 g=%d pat=%d page=%p pk=%d xd=%d"
				" (%p)\n",
				pdpt[i].page.dirty,
				pdpt[i].page.global,
				pdpt[i].page.pat,
				pdpt[i].page.page,
				pdpt[i].page.protection_key,
				pdpt[i].page.xd,
				base
			);
			continue;
		}
		pr_debug(
			"ps=0 pd=%p xd=%d\n",
			pdpt[i].pd.pd,
			pdpt[i].pd.xd
		);
		union pde * pd = (void *) (pdpt[i].pd.pd << 12);
		print_vmmap_pd(pd, base);
	}
}

void print_vmmap_pml4(union pml4e * pml4, uint8_t * base)
{
	for (size_t i = 0; i < 512; i++, base += 512*1024*1024*1024) {
		if (!pml4[i].absent.present)
			continue;
		pr_debug(
			"pml4[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d "
			"pdpt=%p xd=%d\n",
			i,
			pml4[i].pdpt.write_access,
			pml4[i].pdpt.user_access,
			pml4[i].pdpt.pwt,
			pml4[i].pdpt.pcd,
			pml4[i].pdpt.accessed,
			pml4[i].pdpt.pdpt,
			pml4[i].pdpt.xd
		);
		union pdpte * pdpt = (void *) (pml4[i].pdpt.pdpt << 12);
		print_vmmap_pdpt(pdpt, base);
	}
}

static union pml4e * pml4 = NULL;

void vmm_init(struct memmap map)
{
	uint64_t raw_cr4 = read_cr4();
	struct cr4 * cr4 = (void *) &raw_cr4;
	if (cr4->pcide) {
		pr_notice("Disabling CR4.PCIDE\n", 0);
		cr4->pcide = false;
		write_cr4(raw_cr4);
	}

	pml4 = pmalloc(NB_PML4_ENTRY * sizeof(*pml4), 4096);
	if (pml4 == NULL) {
		pr_emerg("Failed to allocate PML4\n", 0);
		return;
	}

	for (size_t i = 0; i < NB_PML4_ENTRY; i++) {
		struct pml4_absent base_pml4e = {.present = false};
		pml4[i].absent = base_pml4e;
	}
	map_page_pt(pml4, pml4, pml4);

	for (size_t i = 0; i < map.desc_count; i++) {
		switch (map.desc[i].type) {
		case MEMORY_TYPE_KERNEL_CODE:
		case MEMORY_TYPE_KERNEL_DATA:
		case MEMORY_TYPE_EFI_SERVICES:
		case MEMORY_TYPE_ACPI_RECLAIMABLE:
		case MEMORY_TYPE_ACPI_NVS:
		case MEMORY_TYPE_HARDWARE:
			map_pages_pt(pml4, map.desc[i].phy_start,
				     (uint8_t *) map.desc[i].phy_start
				     + (size_t) map.desc[i].virt_start,
				     map.desc[i].size);
		default:;
		}
		//vmm_map(
		//	map.desc[i].phy_start,
		//	(uint8_t *) map.desc[i].phy_start
		//	+ (size_t) map.desc[i].virt_start,
		//	map.desc[i].size
		//);
	}

	disable_paging();
	uint64_t raw_cr3 = read_cr3();
	union cr3 * cr3 = (void *) &raw_cr3;
	//print_vmmap_pml4((void *) (cr3->normal.pml4 << 12), 0);
	//print_vmmap_pml4(pml4, 0);
	cr3->normal.pml4 = (unsigned long int) pml4 >> 12;
	write_cr3(raw_cr3);

	//union pml4e * pml4 = pzalloc(512 * sizeof(*pml4), 4096);
	enable_paging();
}

