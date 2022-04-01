#define pr_fmt(fmt) "vmm: " fmt

#include "vmm.h"
#include "page.h"

#include <asm/cpu.h>
#include <mm.h>
#include <print.h>

static void print_vmmap_pt(union pte * pt, uint64_t base)
{
	for (size_t i = 0; i < 512; i++, base += 4096) {
		if (!pt[i].absent.p)
			continue;
		pr_debug(
			"pt[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d d=%d "
			"pat=%d g=%d page=%p pk=%d xd=%d (%p)\n",
			i,
			pt[i].page.rw,
			pt[i].page.us,
			pt[i].page.pwt,
			pt[i].page.pcd,
			pt[i].page.a,
			pt[i].page.d,
			pt[i].page.pat,
			pt[i].page.g,
			pt[i].page.page,
			pt[i].page.pk,
			pt[i].page.xd,
			base
		);
	}
}

static void print_vmmap_pd(union pde * pd, uint64_t base)
{
	for (size_t i = 0; i < 512; i++, base += 2*1024*1024) {
		if (!pd[i].absent.p)
			continue;
		pr_debug(
			"pd[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d ",
			i,
			pd[i].page.rw,
			pd[i].page.us,
			pd[i].page.pwt,
			pd[i].page.pcd,
			pd[i].page.a
		);
		if (pd[i].page.ps) {
			pr_debug(
				"d=%d ps=1 g=%d pat=%d page=%p pk=%d xd=%d "
				"(%p)\n",
				pd[i].page.d,
				pd[i].page.g,
				pd[i].page.pat,
				pd[i].page.page,
				pd[i].page.pk,
				pd[i].page.xd,
				base
			);
			continue;
		}
		pr_debug(
			"ps=0 pt=%p xd=%d\n",
			pd[i].pt.table,
			pd[i].pt.xd
		);
		union pte * pt = (void *) (pd[i].pt.table << 12);
		print_vmmap_pt(pt, base);
	}
}

static void print_vmmap_pdpt(union pdpte * pdpt, uint64_t base)
{
	for (size_t i = 0; i < 512; i++, base += 1024*1024*1024) {
		if (!pdpt[i].absent.p)
			continue;
		pr_debug(
			"pdpt[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d ",
			i,
			pdpt[i].page.rw,
			pdpt[i].page.us,
			pdpt[i].page.pwt,
			pdpt[i].page.pcd,
			pdpt[i].page.a
		);
		if (pdpt[i].page.ps) {
			pr_debug(
				"d=%d ps=1 g=%d pat=%d page=%p pk=%d xd=%d"
				" (%p)\n",
				pdpt[i].page.d,
				pdpt[i].page.g,
				pdpt[i].page.pat,
				pdpt[i].page.page,
				pdpt[i].page.pk,
				pdpt[i].page.xd,
				base
			);
			continue;
		}
		pr_debug(
			"ps=0 pd=%p xd=%d\n",
			pdpt[i].pd.table,
			pdpt[i].pd.xd
		);
		union pde * pd = (void *) (pdpt[i].pd.table << 12);
		print_vmmap_pd(pd, base);
	}
}

static void print_vmmap_pml4(union pml4e * pml4, uint64_t base)
{
	for (size_t i = 0; i < 512; i++, base += 512*1024*1024*1024L) {
		if (!pml4[i].absent.p)
			continue;
		pr_debug(
			"pml4[%d]: p=1 rw=%d us=%d pwt=%d pcd=%d a=%d "
			"pdpt=%p xd=%d\n",
			i,
			pml4[i].pdpt.rw,
			pml4[i].pdpt.us,
			pml4[i].pdpt.pwt,
			pml4[i].pdpt.pcd,
			pml4[i].pdpt.a,
			pml4[i].pdpt.table,
			pml4[i].pdpt.xd
		);
		union pdpte * pdpt = (void *) (pml4[i].pdpt.table << 12);
		print_vmmap_pdpt(pdpt, base);
	}
}

void dump_vmm(union cr3 * cr3)
{
	union pml4e * pml4 = (void *) (cr3->normal.table << 12);
	print_vmmap_pml4(pml4, 0);
}

void dump_vmm_current(void)
{
	uint64_t cr3 = read_cr3();
	dump_vmm((union cr3 *) &cr3);
}
