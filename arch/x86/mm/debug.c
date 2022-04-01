#include "vmm.h"

#include <mm.h>
#include <print.h>

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

static void print_vmmap_pml4(union pml4e * pml4, uint8_t * base)
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
