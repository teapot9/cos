#include "page.h"

#include <errno.h>

#include <print.h>
#include "vmm.h"
#include <asm/cpu.h>
#include <mm/helper.h>
#include <memlist.h>

#define PAGES_VADDR_BASE (void *) 0xFFFFFFFFFFC00000
static int alloc_table(pid_t pid, struct page pg, struct page_perms perms);

#ifdef BOOTLOADER
/* Bootloader: process management is not compiled, use current cr3 */
static union cr3 * get_cr3(pid_t pid)
{
	static uint64_t cr3 = 0;
	if (!cr3)
		cr3 = read_cr3();
	return (union cr3 *) &cr3;
}
#else // BOOTLOADER
static union cr3 * get_cr3(pid_t pid)
{
#if 0
	if (!vmm_fully_init && !pid) {
		static uint64_t cr3 = 0;
		if (!cr3) {
			struct page pg = {
				.size = VIRTUAL_MEMORY_SIZE,
				.page = {.cr3 = (union cr3 *) &cr3},
			};
			struct page_perms perms = {
				.write = true, .exec = true, .user = true,
			};
			int err = alloc_table(pid, pg, perms);
			if (err) {
				cr3 = 0;
				return NULL;
			}
		}
		return (union cr3 *) &cr3;
	}
#endif
	union cr3 * cr3 = process_cr3(process_get(pid));
	if (cr3 == NULL)
		return NULL;

	if (*(uword_t *) cr3 == 0) {
		/* Alloc PML4 */
		struct page pg = {
			.size = VIRTUAL_MEMORY_SIZE,
			.page = {.cr3 = cr3},
		};
		struct page_perms perms = {
			.write = true, .exec = true, .user = true,
		};
		int err = alloc_table(pid, pg, perms);
		if (err) {
			cr3 = 0;
			return NULL;
		}
	}
	return cr3;
}
#endif // !BOOTLOADER

static inline union pml4e * cr3_pml4(const union cr3 * cr3)
{
	return (void *) (cr3->normal.table << 12);
}

static inline bool is_psize(size_t s)
{
	return s == PAGE_SIZE_PT || s == PAGE_SIZE_PD || s == PAGE_SIZE_PDPT;
}

static inline struct page_perms ptable_perms(const void * vaddr)
{
	return (struct page_perms) {
		.write = true,
		.user = (vaddr < IMAGE_BASE),
		.exec = true,
	};
}

static inline bool is_present(struct page pg)
{
	return pg.page.pt->absent.p;
}

#ifndef BOOTLOADER
static struct table_vaddr * table_vaddr_root(pid_t pid)
{
	struct table_vaddr ** root = process_vrtable(process_get(pid));
	if (root == NULL)
		return NULL;
	if (*root == NULL)
		*root = kmalloc(sizeof(**root));
	return *root;
}

static struct table_vaddr * table_vaddr_get(
	struct table_vaddr * pml4, void * vaddr, size_t type
)
{
	size_t svaddr = ((size_t) vaddr) & 0xFFFFFFFFFFFF;
	if (type == PAGE_SIZE_PML4)
		return pml4;

	struct table_vaddr * pdpt = pml4->subtables + (svaddr / PAGE_SIZE_PML4);
	if (pdpt == NULL)
		return NULL;
	if (type == PAGE_SIZE_PDPT)
		return pdpt;
	svaddr %= PAGE_SIZE_PML4;

	struct table_vaddr * pd = pdpt->subtables + (svaddr / PAGE_SIZE_PDPT);
	if (pd == NULL)
		return NULL;
	if (type == PAGE_SIZE_PD)
		return pd;
	svaddr %= PAGE_SIZE_PDPT;

	struct table_vaddr * pt = pd->subtables + (svaddr / PAGE_SIZE_PD);
	if (pt == NULL)
		return NULL;
	if (type == PAGE_SIZE_PT)
		return pt;
	return NULL;

}

static int table_vaddr_new(struct table_vaddr * pml4,
                           void * vaddr, size_t type, void * taddr)
{
	struct table_vaddr * table = table_vaddr_get(pml4, vaddr, type);
	if (table == NULL)
		return -EINVAL;
	size_t elts;

	switch (type) {
	case PAGE_SIZE_PML4:
		elts = NB_PML4_ENTRY;
		break;
	case PAGE_SIZE_PDPT:
		elts = NB_PDPT_ENTRY;
		break;
	case PAGE_SIZE_PD:
		elts = NB_PD_ENTRY;
		break;
	case PAGE_SIZE_PT:
		elts = 0;
		break;
	default:
		return -EINVAL;
	}

	struct table_vaddr * new = NULL;
	if (elts) {
		new = kmalloc(elts * sizeof(*new));
		for (size_t i = 0; i < elts; i++)
			new[i] = (struct table_vaddr) {
				.vaddr = NULL, .subtables = NULL
			};
	}
	table->subtables = new;
	table->vaddr = taddr;
	return 0;
}

static int table_vaddr_del(struct table_vaddr * pml4,
                           void * vaddr, size_t type)
{
	struct table_vaddr * table = table_vaddr_get(pml4, vaddr, type);
	if (table == NULL)
		return -EINVAL;
	kfree(table->subtables);
	table->subtables = NULL;
	return 0;
}

static void * table_vaddr(struct table_vaddr * table, void * vaddr, size_t type)
{
	if (table == NULL || table->vaddr == NULL)
		return NULL;
	return (uint8_t *) table->vaddr + ((size_t) vaddr % type);
}
#else // !BOOTLOADER
/* Bootloader: memory is mapped 1:1, don't bother with vtables */
#define table_vaddr_get(r, x, y) 0
#define table_vaddr(r, x, y) (x)
#define table_vaddr_new(r, x, y, z) 0
#define table_vaddr_del(r, x, y) 0
#endif // BOOTLOADER

static inline void * new_table(pid_t pid, size_t tab_size,
			       size_t type, struct page_perms perms)
{
	pr_debug("new_table size 0x%zx type 0x%zx\n", tab_size, type);
	void * paddr = palloc(pid, tab_size, PAGE_TABLE_ALIGN);
	if (paddr == NULL)
		return NULL;
	if (!vmm_fully_init) {
		// Pre-init: linear memory mapping, mapping in finalize_table()
		return paddr;
	}
#ifdef BOOTLOADER
	return paddr;
#endif
	void * vaddr = mmap(pid, paddr, tab_size,
	                    PAGE_TABLE_ALIGN, perms);
	int err = table_vaddr_new(table_vaddr_root(pid), paddr, type, vaddr);
	if (err) {
		punmap(pid, paddr, tab_size);
		vunmap(pid, vaddr, tab_size);
		return NULL;
	}
	return vaddr;
}

static inline int finalize_table(pid_t pid, size_t tab_size, size_t type,
				 void * paddr, struct page_perms perms)
{
	if (vmm_fully_init)
		return 0; // Already mapped
#ifdef BOOTLOADER
	return 0;
#endif
	void * vaddr = mmap(pid, paddr, tab_size,
	                    PAGE_TABLE_ALIGN, perms);
	int err = table_vaddr_new(table_vaddr_root(pid), paddr, type, vaddr);
	if (err) {
		punmap(pid, paddr, tab_size);
		vunmap(pid, vaddr, tab_size);
		return err;
	}
	return 0;
}

static int alloc_table(pid_t pid, struct page pg, struct page_perms perms)
{
	pr_debug("alloc_table type 0x%zx\n", pg.size);
	struct page_perms table_perms = {
		.exec = false,
		.user = false,
		.write = true,
	};
	static_assert(
		MIN_PAGE_SIZE == NB_PML4_ENTRY * sizeof(union pml4e)
		&& MIN_PAGE_SIZE == NB_PDPT_ENTRY * sizeof(union pdpte)
		&& MIN_PAGE_SIZE == NB_PD_ENTRY * sizeof(union pde)
		&& MIN_PAGE_SIZE == NB_PT_ENTRY * sizeof(union pte),
		"MIN_PAG_SIZE is not the size of a page table"
	);
	int err;

	switch (pg.size) {
	case VIRTUAL_MEMORY_SIZE:;
		union pml4e * pml4 = new_table(
			pid, NB_PML4_ENTRY * sizeof(*pml4),
			PAGE_SIZE_PML4, table_perms
		);
		if (pml4 == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PML4_ENTRY; i++)
			pml4[i].absent.p = false;

		pg.page.cr3->normal.pwt = false;
		pg.page.cr3->normal.pcd = false;
		pg.page.cr3->normal.table =
			(unsigned long) page_paddr(pid, pml4) >> 12;

		err = finalize_table(pid, NB_PML4_ENTRY * sizeof(*pml4),
				     PAGE_SIZE_PML4, pml4, table_perms);
		if (err)
			return err;
		return 0;
	case PAGE_SIZE_PML4:
		if (pg.page.pml4->absent.p)
			return -EBUSY;

		union pdpte * pdpt = new_table(
			pid, NB_PDPT_ENTRY * sizeof(*pdpt),
			PAGE_SIZE_PDPT, table_perms
		);
		if (pdpt == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PDPT_ENTRY; i++)
			pdpt[i].absent.p = false;

		struct pml4_pdpt pml4e = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.table = (unsigned long) page_paddr(pid, pdpt) >> 12,
			.xd = !perms.exec,
		};
		pg.page.pml4->pdpt = pml4e;

		err = finalize_table(pid, NB_PDPT_ENTRY * sizeof(*pdpt),
				     PAGE_SIZE_PDPT, pdpt, table_perms);
		if (err)
			return err;
		return 0;
	case PAGE_SIZE_PDPT:
		if (pg.page.pdpt->absent.p)
			return -EBUSY;

		union pde * pd = new_table(
			pid, NB_PD_ENTRY * sizeof(*pd),
			PAGE_SIZE_PD, table_perms
		);
		if (pd == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PD_ENTRY; i++)
			pd[i].absent.p = false;

		struct pdpt_pd pdpte = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.ps = false,
			.table = (unsigned long) page_paddr(pid, pd) >> 12,
			.xd = !perms.exec,
		};
		pg.page.pdpt->pd = pdpte;

		err = finalize_table(pid, NB_PD_ENTRY * sizeof(*pd),
				     PAGE_SIZE_PD, pd, table_perms);
		if (err)
			return err;
		return 0;
	case PAGE_SIZE_PD:
		if (pg.page.pd->absent.p)
			return -EBUSY;

		union pte * pt = new_table(
			pid, NB_PT_ENTRY * sizeof(*pt),
			PAGE_SIZE_PT, table_perms
		);
		if (pt == NULL)
			return -ENOMEM;
		for (size_t i = 0; i < NB_PT_ENTRY; i++)
			pt[i].absent.p = false;

		struct pd_pt pde = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.ps = false,
			.table = (unsigned long) page_paddr(pid, pt) >> 12,
			.xd = !perms.exec,
		};
		pg.page.pd->pt = pde;

		err = finalize_table(pid, NB_PT_ENTRY * sizeof(*pt),
				     PAGE_SIZE_PT, pt, table_perms);
		if (err)
			return err;
		return 0;
	default:
		return -EINVAL;
	}
}

static int free_table(pid_t pid, struct page pg)
{
	void * paddr;
	switch (pg.size) {
	case PAGE_SIZE_PML4:
		if (!pg.page.pml4->absent.p)
			return 0;

		union pdpte * pdpt = (void *) (pg.page.pml4->pdpt.table << 12);
		for (size_t i = 0; i < NB_PDPT_ENTRY; i++) {
			if (pdpt[i].absent.p)
				return -EBUSY;
		}

		pg.page.pml4->pdpt.p = false;
		paddr = page_paddr(pid, pdpt);
		punmap(0, paddr, NB_PDPT_ENTRY * sizeof(*pdpt));
		table_vaddr_del(table_vaddr_root(pid), paddr, PAGE_SIZE_PDPT);
		return 0;
	case PAGE_SIZE_PDPT:
		if (!pg.page.pdpt->absent.p)
			return 0;
		if (pg.page.pdpt->pd.ps)
			return -EINVAL;

		union pde * pd = (void *) (pg.page.pdpt->pd.table << 12);
		for (size_t i = 0; i < NB_PD_ENTRY; i++) {
			if (pd[i].absent.p)
				return -EBUSY;
		}

		pg.page.pdpt->pd.p = false;
		paddr = page_paddr(pid, pd);
		punmap(0, paddr, NB_PD_ENTRY * sizeof(*pd));
		table_vaddr_del(table_vaddr_root(pid), paddr, PAGE_SIZE_PD);
		return 0;
	case PAGE_SIZE_PD:
		if (!pg.page.pd->absent.p)
			return 0;
		if (pg.page.pd->pt.ps)
			return -EINVAL;

		union pte * pt = (void *) (pg.page.pd->pt.table << 12);
		for (size_t i = 0; i < NB_PT_ENTRY; i++) {
			if (pt[i].absent.p)
				return -EBUSY;
		}

		pg.page.pd->pt.p = false;
		paddr = page_paddr(pid, pt);
		punmap(0, paddr, NB_PT_ENTRY * sizeof(*pt));
		table_vaddr_del(table_vaddr_root(pid), paddr, PAGE_SIZE_PT);
		return 0;
	default:
		return -EINVAL;
	}
}

static void set_perms(struct page pg, struct page_perms perms,
                      bool accessed, bool dirty)
{
	switch (pg.size) {
	case PAGE_SIZE_PDPT:
		pg.page.pdpt->page.rw = perms.write;
		pg.page.pdpt->page.us = perms.user;
		pg.page.pdpt->page.xd = !perms.exec;
		pg.page.pdpt->page.a = accessed;
		pg.page.pdpt->page.d = dirty;
		break;
	case PAGE_SIZE_PD:
		pg.page.pd->page.rw = perms.write;
		pg.page.pd->page.us = perms.user;
		pg.page.pd->page.xd = !perms.exec;
		pg.page.pd->page.a = accessed;
		pg.page.pd->page.d = dirty;
		break;
	case PAGE_SIZE_PT:
		pg.page.pt->page.rw = perms.write;
		pg.page.pt->page.us = perms.user;
		pg.page.pt->page.xd = !perms.exec;
		pg.page.pt->page.a = accessed;
		pg.page.pt->page.d = dirty;
		break;
	}
}

static int do_subtable(struct page tb, void * subtb, struct page_perms perms)
{
	switch (tb.size) {
	case PAGE_SIZE_PML4:
		if (tb.page.pml4->pdpt.p)
			return -EBUSY;
		struct pml4_pdpt pdpt_ref = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.ps = false,
			.table = (unsigned long) subtb >> 12,
			.xd = !perms.exec,
		};
		tb.page.pml4->pdpt = pdpt_ref;
		return 0;
	case PAGE_SIZE_PDPT:
		if (tb.page.pdpt->pd.p)
			return -EBUSY;
		struct pdpt_pd pd_ref = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.ps = false,
			.table = (unsigned long) subtb >> 12,
			.xd = !perms.exec,
		};
		tb.page.pdpt->pd = pd_ref;
		return 0;
	case PAGE_SIZE_PD:
		if (tb.page.pd->pt.p)
			return -EBUSY;
		struct pd_pt pt_ref = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.ps = false,
			.table = (unsigned long) subtb >> 12,
			.xd = !perms.exec,
		};
		tb.page.pd->pt = pt_ref;
		return 0;
	default:
		return -EINVAL;
	}
}

static int do_map(struct page pg, void * paddr, struct page_perms perms)
{
	switch (pg.size) {
	case PAGE_SIZE_PDPT:
		if (pg.page.pdpt->page.p)
			return -EBUSY;
		struct pdpt_page pdpt_page = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.d = false,
			.ps = true,
			.g = false,
			.pat = false,
			._zero1 = 0,
			.page = (unsigned long) paddr >> 12,
			.pk = 0,
			.xd = !perms.exec,
		};
		pg.page.pdpt->page = pdpt_page;
		return 0;
	case PAGE_SIZE_PD:
		if (pg.page.pd->page.p)
			return -EBUSY;
		struct pd_page pd_page = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.d = false,
			.ps = true,
			.g = false,
			.pat = false,
			._zero1 = 0,
			.page = (unsigned long) paddr >> 12,
			.pk = 0,
			.xd = !perms.exec,
		};
		pg.page.pd->page = pd_page;
		return 0;
	case PAGE_SIZE_PT:
		if (pg.page.pt->page.p)
			return -EBUSY;
		struct pt_page pt_page = {
			.p = true,
			.rw = perms.write,
			.us = perms.user,
			.pwt = false,
			.pcd = false,
			.a = false,
			.d = false,
			.pat = false,
			.g = false,
			.page = (unsigned long) paddr >> 12,
			.pk = 0,
			.xd = !perms.exec,
		};
		pg.page.pt->page = pt_page;
		return 0;
	default:
		return -EINVAL;
	}
}

static int do_unmap(struct page pg)
{
	switch (pg.size) {
	case PAGE_SIZE_PDPT:
		if (!pg.page.pdpt->page.p || !pg.page.pdpt->page.ps)
			return -EINVAL;
		pg.page.pdpt->page.p = false;
		return 0;
	case PAGE_SIZE_PD:
		if (!pg.page.pd->page.p || !pg.page.pd->page.ps)
			return -EINVAL;
		pg.page.pd->page.p = false;
		return 0;
	case PAGE_SIZE_PT:
		if (!pg.page.pt->page.p)
			return -EINVAL;
		pg.page.pt->page.p = false;
		return 0;
	default:
		return -EINVAL;
	}
}

struct page page_get(pid_t pid, void * vaddr, bool p)
{
	union cr3 * cr3 = get_cr3(pid);
	union lin_addr * a = (union lin_addr *) &vaddr;
	union pml4e * pml4 = table_vaddr(table_vaddr_root(pid), cr3_pml4(cr3), PAGE_SIZE_PML4);
	struct page prev;
	struct page cur = {
		.size = VIRTUAL_MEMORY_SIZE,
		.page.cr3 = cr3,
	};

	union pml4e * pml4e = &pml4[a->pt.pml4];
	prev = cur;
	cur = (struct page) {
		.size = PAGE_SIZE_PML4,
		.page.pml4 = pml4e,
	};
	if (!pml4e->absent.p)
		return p ? prev : cur;

	union pdpte * pdpt = table_vaddr(
		table_vaddr_root(pid), (void *) (pml4e->pdpt.table << 12), PAGE_SIZE_PDPT
	);
	union pdpte * pdpte = &pdpt[a->pt.pdpt];
	prev = cur;
	cur = (struct page) {
		.size = PAGE_SIZE_PDPT,
		.page.pdpt = pdpte,
	};
	if (!pdpte->absent.p || pdpte->page.ps)
		return (p && !pdpte->absent.p) ? prev : cur;

	union pde * pd = table_vaddr(
		table_vaddr_root(pid), (void *) (pdpte->pd.table << 12), PAGE_SIZE_PD
	);
	union pde * pde = &pd[a->pt.pd];
	prev = cur;
	cur = (struct page) {
		.size = PAGE_SIZE_PD,
		.page.pd = pde,
	};
	if (!pde->absent.p || pde->page.ps)
		return (p && !pde->absent.p) ? prev : cur;

	union pte * pt = table_vaddr(
		table_vaddr_root(pid), (void *) (pde->pt.table << 12), PAGE_SIZE_PT
	);
	union pte * pte = &pt[a->pt.pt];
	prev = cur;
	cur = (struct page) {
		.size = PAGE_SIZE_PT,
		.page.pt = pte,
	};
	return (p && !pte->absent.p) ? prev : cur;
}

void * page_paddr(pid_t pid, void * vaddr)
{
	if (!vmm_fully_init) {
		/* Kernel starts with 1:1 mapping */
		return vaddr;
	}

	union lin_addr * addr = (union lin_addr *) &vaddr;
	struct page pg = page_get(pid, vaddr, false);
	unsigned long ret = (unsigned long) NULL;

	switch (pg.size) {
	case PAGE_SIZE_PML4:
		break;
	case PAGE_SIZE_PDPT:
		if (pg.page.pdpt->absent.p)
			ret = pg.page.pdpt->page.page << 30 | addr->pdpt.offset;
		break;
	case PAGE_SIZE_PD:
		if (pg.page.pd->absent.p)
			ret = pg.page.pd->page.page << 21 | addr->pd.offset;
		break;
	case PAGE_SIZE_PT:
		if (pg.page.pt->absent.p)
			ret = pg.page.pt->page.page << 12 | addr->pt.offset;
		break;
	}
	return (void *) ret;
}

static void * _page_find_free(
	pid_t pid, size_t align, size_t count, void * start
)
{
	uint8_t * vaddr = start;
	struct page pg;
	size_t n = count;

	while (n > 0) {
		/*
		if (is_present((pg = page_get(pid, vaddr, false))))
			return _page_find_free(pid, align, count,
			                       (uint8_t *) vaddr + align);
		*/
		if (is_present((pg = page_get(pid, vaddr, false)))) {
			start = vaddr + align;
			n = count;
		} else {
			n--;
		}
		if (vaddr + align < vaddr)
			return NULL;
		vaddr += align;
	}
	return start;
}

void * page_find_free(pid_t pid, size_t align, size_t count)
{
#ifndef BOOTLOADER
	void * start = pid == 0 ? IMAGE_BASE : (void *) 0;
#else // !BOOTLOADER
	void * start = (void *) 0;
#endif
	start = aligned(start, align);
	if (is_inside(start, align * count, NULL, 1))
		start = (uint8_t *) start + align;
	return _page_find_free(pid, align, count, start);
}

int page_map(pid_t pid, void * vaddr, void * paddr,
             size_t size, struct page_perms perms)
{
	int err;
	if (!is_psize(size))
		return -EINVAL;
	struct page pg = page_get(pid, vaddr, false);
	if (size > pg.size)
		return -ENOMEM;

	while (pg.size > size) {
		err = alloc_table(pid, pg, ptable_perms(vaddr));
		if (err)
			return err;
		pg = page_get(pid, vaddr, false);
	}

	err = do_map(pg, paddr, perms);
	invlpg(vaddr);
	if (err)
		return err;
	return 0;
}

int page_unmap(pid_t pid, void * vaddr, size_t size)
{
	while (size > 0) {
		struct page pg = page_get(pid, vaddr, true);
		if (pg.size > size)
			return -EINVAL;

		int err = do_unmap(pg);
		if (err)
			return err;

		struct page table;
		do {
			table = page_get(pid, vaddr, true);
			err = free_table(pid, table);
		} while (!err && table.size < VIRTUAL_MEMORY_SIZE);
		invlpg(vaddr);

		vaddr = (uint8_t *) vaddr + pg.size;
		size -= pg.size;
	}
	return 0;
}

int page_set(pid_t pid, void * vaddr, size_t size,
             struct page_perms perms, bool accessed, bool dirty)
{
	while (size > 0) {
		struct page pg = page_get(pid, vaddr, true);
		if (pg.size > size)
			return -EINVAL;

		set_perms(pg, perms, accessed, dirty);
		invlpg(vaddr);

		vaddr = (uint8_t *) vaddr + pg.size;
		size -= pg.size;
	}
	return 0;
}

#ifdef BOOTLOADER
int page_identity(union cr3 * cr3)
{
#if 0
	struct page pg = {
		.size = VIRTUAL_MEMORY_SIZE, // alloc pml4
		.page = {.cr3 = cr3},
	};
	struct page_perms perms = {
		.write = true,
		.user = false,
		.exec = true,
	};
	int err = alloc_table(0, pg, perms);
	if (err)
		return err;

	union pml4e * pml4 = cr3_pml4(cr3);
	pg = (struct page) {
		.size = PAGE_SIZE_PML4, // alloc pdpt
		.page = {.pml4 = &pml4[0]},
	};
	err = alloc_table(0, pg, perms);
	if (err)
		return err;

	err = page_map(0, 0, 0, PAGE_SIZE_PDPT, perms);
	if (err)
		return err;
	return 0;
	//union pdpte * pdpt = table_vaddr(
	//	0, (void *) (pml4[0].pdpt.table << 12), PAGE_SIZE_PDPT
	//);
#endif
	union pml4e * pml4 = palloc(0, NB_PML4_ENTRY * sizeof(*pml4), 4096);
	if (pml4 == NULL)
		return -ENOMEM;
	for (size_t i = 0; i < NB_PML4_ENTRY; i++)
		pml4[i].absent.p = false;

	union pdpte * pdpt = palloc(0, NB_PDPT_ENTRY * sizeof(*pdpt), 4096);
	if (pdpt == NULL) {
		punmap(0, pml4, NB_PML4_ENTRY * sizeof(*pml4));
		return -ENOMEM;
	}
	for (size_t i = 0; i < NB_PDPT_ENTRY; i++)
		pdpt[i].absent.p = false;

	pdpt[0].page = (struct pdpt_page) {
		.p = true,
		.rw = true,
		.us = false,
		.pwt = false,
		.pcd = false,
		.a = false,
		.d = false,
		.ps = true, // identity paging with 1 * 1GB page
		.g = false,
		.pat = false,
		.page = 0,
		.pk = 0,
		.xd = false,
	};
	pml4[0].pdpt = (struct pml4_pdpt) {
		.p = true,
		.rw = true,
		.us = false,
		.pwt = false,
		.pcd = false,
		.a = false,
		.ps = false,
		.table = (unsigned long) pdpt >> 12,
		.xd = false,
	};
	cr3->normal = (struct cr3_normal) {
		.pwt = false,
		.pcd = false,
		.table = (unsigned long) pml4 >> 12,
	};

	return 0;
}
#endif // BOOTLOADER

int page_kmem_setup(struct memlist * l, union cr3 cr3,
                    struct table_vaddr ** vroot)
{
	int err;
	union pml4e * pml4 = cr3_pml4(&cr3);
	union pdpte * pdpt = (void *) (pml4[511].pdpt.table << 12);

	/* Initialize table_vaddr */
	struct table_vaddr * table = kmalloc(sizeof(*table));
	if (vroot == NULL)
		return -ENOMEM;
	table->subtables = NULL;
	table->vaddr = NULL;
	table_vaddr_new(table, PAGES_VADDR_BASE, PAGE_SIZE_PML4, NULL);
	table_vaddr_new(table, PAGES_VADDR_BASE, PAGE_SIZE_PDPT, NULL);

#define page_early_ktables_pd(n) do { \
		union pde * pd = palloc(0, NB_PD_ENTRY * sizeof(*pd), 4096); \
		if (pd == NULL) { \
			err = -ENOMEM; \
			break; \
		} \
		for (size_t i = 0; i < NB_PD_ENTRY; i++) \
			pd[i].absent.p = false; \
		struct page pg = \
			{.page.pdpt = &pdpt[n], .size = PAGE_SIZE_PDPT}; \
		struct page_perms perms = \
			{.exec = false, .write = true, .user = false}; \
		err = do_subtable(pg, pd, perms); \
		if (err) \
			break; \
	} while (0)

#define page_early_ktables_pt(n) do { \
		union pte * pt = palloc(0, NB_PT_ENTRY * sizeof(*pt), 4096); \
		if (pt == NULL) { \
			err = -ENOMEM; \
			break; \
		} \
		for (size_t i = 0; i < NB_PT_ENTRY; i++) \
			pt[i].absent.p = false; \
		struct page pg = \
			{.page.pd = &pd[n], .size = PAGE_SIZE_PD}; \
		struct page_perms perms = \
			{.exec = false, .write = true, .user = false}; \
		err = do_subtable(pg, pt, perms); \
		if (err) \
			break; \
	} while (0)

	/* Create missing PDs */
	if (!pdpt[510].absent.p) {
		page_early_ktables_pd(510);
		if (err)
			return err;
	}
	if (!pdpt[511].absent.p) {
		page_early_ktables_pd(511);
		if (err)
			return err;
	}

	/* Create missing PTs */
	union pde * pd = (void *) (pdpt[511].pd.table << 12);
	if (!pd[510].absent.p) {
		do {
			union pte * pt = palloc(0, NB_PT_ENTRY * sizeof(*pt), 4096);
			if (pt == NULL) {
				err = -ENOMEM;
				break;
			}
			for (size_t i = 0; i < NB_PT_ENTRY; i++)
				pt[i].absent.p = false;
			struct page pg =
				{.page.pd = &pd[510], .size = PAGE_SIZE_PD};
			struct page_perms perms =
				{.exec = false, .write = true, .user = false};
			err = do_subtable(pg, pt, perms);
			if (err)
				break;
		} while (0);
		if (err)
			return err;
	}
	if (!pd[511].absent.p) {
		do {
			union pte * pt = palloc(0, NB_PT_ENTRY * sizeof(*pt), 4096);
			if (pt == NULL) {
				err = -ENOMEM;
				break;
			}
			for (size_t i = 0; i < NB_PT_ENTRY; i++)
				pt[i].absent.p = false;
			struct page pg =
				{.page.pd = &pd[511], .size = PAGE_SIZE_PD};
			struct page_perms perms =
				{.exec = false, .write = true, .user = false};
			err = do_subtable(pg, pt, perms);
			if (err)
				break;
		} while (0);
		if (err)
			return err;
	}

	union pte * pts[2];
	pts[0] = (void *) (pd[510].pt.table << 12);
	pts[1] = (void *) (pd[511].pt.table << 12);
	size_t cur_pt = 0;
	size_t cur_pte = 0;
	struct page pg;
	struct page_perms perms = \
		{.exec = false, .write = true, .user = false}; \

	for (size_t i = 510; i < 512; i++) {
		if (pdpt[i].page.ps)
			return -ENOTSUP;
		union pde * pd = (void *) (pdpt[i].pd.table << 12);

		/* Register page containing PD */
		err = memlist_add(l, pd, NB_PD_ENTRY * sizeof(*pd), false);
		if (err)
			return err;

		/* Map table */
		pg = (struct page) {
			.page.pt = &pts[cur_pt][cur_pte],
			.size = PAGE_SIZE_PT,
		};
		err = do_map(pg, pd, perms);
		if (err)
			return err;

		/* Save vaddr */
		void * tptr = (uint8_t *) PAGES_VADDR_BASE
			+ cur_pt * PAGE_SIZE_PD
			+ cur_pte * PAGE_SIZE_PT;
		void * vptr = (uint8_t *) IMAGE_BASE
			+ (i - 510) * PAGE_SIZE_PDPT;
		table_vaddr_new(table, vptr, PAGE_SIZE_PD, tptr);

		if (++cur_pte >= 512) {
			cur_pte = 0;
			cur_pt++;
		}

		for (size_t j = 0; j < NB_PD_ENTRY; j++) {
			if (!pd[i].absent.p)
				continue;
			if (pd[i].page.ps)
				continue;
			union pte * pt = (void *) (pd[i].pt.table << 12);

			/* Register page containing PT */
			err = memlist_add(
				l, pt, NB_PT_ENTRY * sizeof(*pt), false
			);
			if (err)
				return err;

			/* Map table */
			pg = (struct page) {
				.page.pt = &pts[cur_pt][cur_pte],
				.size = PAGE_SIZE_PT,
			};
			if (err)
				return err;

			/* Save vaddr */
			tptr = (uint8_t *) PAGES_VADDR_BASE
				+ cur_pt * PAGE_SIZE_PD
				+ cur_pte * PAGE_SIZE_PT;
			vptr = (uint8_t *) IMAGE_BASE
				+ (i - 510) * PAGE_SIZE_PDPT
				+ j * PAGE_SIZE_PD;
			table_vaddr_new(table, vptr, PAGE_SIZE_PT, tptr);

			if (++cur_pte >= 512) {
				cur_pte = 0;
				cur_pt++;
			}
		}
	}

	*vroot = table;
	return 0;
}

int page_kmem_finalize(struct table_vaddr * kvroot, struct table_vaddr * vroot)
{
	void * kpd1ptr = (uint8_t *) IMAGE_BASE + 0 * PAGE_SIZE_PDPT;
	void * kpd2ptr = (uint8_t *) IMAGE_BASE + 1 * PAGE_SIZE_PDPT;

	struct table_vaddr * kpd1 = table_vaddr_get(
		kvroot, kpd1ptr, PAGE_SIZE_PD
	);
	struct table_vaddr * kpd2 = table_vaddr_get(
		kvroot, kpd2ptr, PAGE_SIZE_PD
	);

	table_vaddr_new(
		vroot, kpd1ptr, PAGE_SIZE_PD, kpd1
	);
	table_vaddr_new(
		vroot, kpd2ptr, PAGE_SIZE_PD, kpd2
	);

	return 0;
}

int page_new_vm(union cr3 * cr3, struct table_vaddr ** vroot)
{
	int err;
	union pml4e * pml4 = NULL;
	union pml4e * pml4_tmp = NULL;
	union pdpte * pdpt = NULL;
	union pdpte * pdpt_tmp = NULL;
	union pde * pd = NULL;
	union pde * pd_tmp = NULL;
	union pte * pt = NULL;
	union pte * pt_tmp = NULL;
	union pdpte * epdpt = NULL;
	union pdpte * epdpt_tmp = NULL;
	struct table_vaddr * table = NULL;

#define maybe_mmap(paddr, size) ( \
	vmm_fully_init ? mmap( \
		0, (paddr), (size), 4096, (struct page_perms) \
		{.exec = false, .user = false, .write = true} \
	) : (paddr) \
)
#define maybe_munmap(vaddr, size) \
	if (vmm_fully_init) vunmap(0, (vaddr), (size))

	/* Create PML4 */
	size_t pml4_size = NB_PML4_ENTRY * sizeof(*pml4);
	pml4 = palloc(0, pml4_size, 4096);
	if (pml4 == NULL)
		goto err_nomem;
	pml4_tmp = maybe_mmap(pml4, pml4_size);
	if (pml4_tmp == NULL)
		goto err_nomem;
	for (size_t i = 0; i < NB_PML4_ENTRY; i++)
		pml4[i].absent.p = false;

	/* Create first PDPT */
	size_t pdpt_size = NB_PDPT_ENTRY * sizeof(*pdpt);
	pdpt = palloc(0, pdpt_size, 4096);
	if (pdpt == NULL)
		goto err_nomem;
	pdpt_tmp = maybe_mmap(pdpt, pdpt_size);
	if (pdpt_tmp == NULL)
		goto err_nomem;
	for (size_t i = 0; i < NB_PDPT_ENTRY; i++)
		pdpt_tmp[i].absent.p = false;

	/* Create first PD */
	size_t pd_size = NB_PD_ENTRY * sizeof(*pd);
	pd = palloc(0, pd_size, 4096);
	if (pd == NULL)
		goto err_nomem;
	pd_tmp = maybe_mmap(pd, pd_size);
	if (pd_tmp == NULL)
		goto err_nomem;
	for (size_t i = 0; i < NB_PD_ENTRY; i++)
		pd_tmp[i].absent.p = false;

	/* Create first PT */
	size_t pt_size = NB_PT_ENTRY * sizeof(*pt);
	pt = palloc(0, pt_size, 4096);
	if (pt == NULL)
		goto err_nomem;
	pt_tmp = maybe_mmap(pt, pt_size);
	if (pt_tmp == NULL)
		goto err_nomem;
	for (size_t i = 0; i < NB_PT_ENTRY; i++)
		pt_tmp[i].absent.p = false;

	/* Create last PDPT */
	epdpt = palloc(0, pdpt_size, 4096);
	if (epdpt == NULL)
		goto err_nomem;
	epdpt_tmp = maybe_mmap(epdpt, pdpt_size);
	if (epdpt_tmp == NULL)
		goto err_nomem;
	for (size_t i = 0; i < NB_PDPT_ENTRY; i++)
		epdpt_tmp[i].absent.p = false;

	void * pml4_vaddr = (void *) (PAGE_SIZE_PT * 1);
	void * pdpt_vaddr = (void *) (PAGE_SIZE_PT * 2);
	void * pd_vaddr = (void *) (PAGE_SIZE_PT * 3);
	void * pt_vaddr = (void *) (PAGE_SIZE_PT * 4);
	void * epdpt_vaddr = (void *) (PAGE_SIZE_PT * 5);
	struct page pg;
	struct page_perms perms;

	/* References between tables */
	perms = (struct page_perms)
		{.exec = true, .user = true, .write = true};

	pg = (struct page) {.page.pml4 = &pml4_tmp[0], .size = PAGE_SIZE_PML4};
	err = do_subtable(pg, pdpt, perms);
	if (err)
		goto err_exit;

	pg = (struct page) {.page.pdpt = &pdpt_tmp[0], .size = PAGE_SIZE_PDPT};
	err = do_subtable(pg, pd, perms);
	if (err)
		goto err_exit;

	pg = (struct page) {.page.pd = &pd_tmp[0], .size = PAGE_SIZE_PD};
	err = do_subtable(pg, pt, perms);
	if (err)
		goto err_exit;

	pg = (struct page)
		{.page.pml4 = &pml4_tmp[511], .size = PAGE_SIZE_PML4};
	err = do_subtable(pg, epdpt, perms);
	if (err)
		goto err_exit;

	/* Map tables in PT */
	perms = (struct page_perms)
		{.exec = false, .user = false, .write = true};
	pg = (struct page) {.page.pt = NULL, .size = PAGE_SIZE_PT};

	pg.page.pt = &pt_tmp[1]; // pml4_vaddr
	err = do_map(pg, pml4, perms);
	if (err)
		goto err_exit;

	pg.page.pt = &pt_tmp[2]; // pdpt_vaddr
	err = do_map(pg, pdpt, perms);
	if (err)
		goto err_exit;

	pg.page.pt = &pt_tmp[3]; // pd_vaddr
	err = do_map(pg, pd, perms);
	if (err)
		goto err_exit;

	pg.page.pt = &pt_tmp[4]; // pt_vaddr
	err = do_map(pg, pt, perms);
	if (err)
		goto err_exit;

	pg.page.pt = &pt_tmp[5]; // epdpt_vaddr
	err = do_map(pg, epdpt, perms);
	if (err)
		goto err_exit;

#define maybe_vaddr(ptr, type) ( \
	vmm_fully_init \
		? table_vaddr(table_vaddr_root(0), (ptr), (type)) \
		: (ptr) \
)

	/* Map kernel memory */
	uint64_t kcr3 = read_cr3();
	union pml4e * kpml4 =
		maybe_vaddr(cr3_pml4((union cr3 *) &kcr3), PAGE_SIZE_PML4);
	union pdpte * kpdpt = maybe_vaddr(
		(void *) (kpml4[511].pdpt.table << 12), PAGE_SIZE_PDPT
	);
	epdpt_tmp[510] = kpdpt[510];
	epdpt_tmp[511] = kpdpt[511];

	/* Create page vaddr table */
	table = kmalloc(sizeof(*table));
	if (table == NULL)
		goto err_nomem;
	table->subtables = NULL;
	table->vaddr = NULL;

	/* Save page virtual addresses */
	table_vaddr_new(table, 0, PAGE_SIZE_PML4, pml4_vaddr);
	table_vaddr_new(table, 0, PAGE_SIZE_PDPT, pdpt_vaddr);
	table_vaddr_new(table, 0, PAGE_SIZE_PD, pd_vaddr);
	table_vaddr_new(table, 0, PAGE_SIZE_PT, pt_vaddr);
	table_vaddr_new(table, IMAGE_BASE, PAGE_SIZE_PDPT, epdpt_vaddr);

	/* Set CR3 and vaddr table */
	cr3->normal.pcd = false;
	cr3->normal.pwt = false;
	cr3->normal.table = (unsigned long) pml4 >> 12;
	*vroot = table;

	err = 0;
	goto exit;

err_nomem:
	err = -ENOMEM;
err_exit:
	/* Free all memory */
	if (pml4 != NULL)
		punmap(0, pml4, pml4_size);
	if (pdpt != NULL)
		punmap(0, pdpt, pdpt_size);
	if (pd != NULL)
		punmap(0, pd, pd_size);
	if (pt != NULL)
		punmap(0, pt, pt_size);
	if (epdpt != NULL)
		punmap(0, epdpt, pdpt_size);
	if (table != NULL)
		kfree(table);
exit:
	/* Remove temporary unmap */
	if (pml4_tmp != NULL)
		maybe_munmap(pml4_tmp, pml4_size);
	if (pdpt_tmp != NULL)
		maybe_munmap(pdpt_tmp, pdpt_size);
	if (pd_tmp != NULL)
		maybe_munmap(pd_tmp, pd_size);
	if (pt_tmp != NULL)
		maybe_munmap(pt_tmp, pt_size);
	if (epdpt_tmp != NULL)
		maybe_munmap(epdpt_tmp, pdpt_size);

	return err;
}
