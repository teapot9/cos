#include <mm/vmm.h>
#include <mm/pmm.h>
#include <setup.h>
#include "physical.h"
#include <memlist.h>
#include <mm/memmap.h>
#include "paging.h"
#include "vmm.h"
#include <alloc.h>
#include <errno.h>
#include <printk.h>
#include "vkernel.h"
#include <asm/cpu.h>

uint64_t kernel_cr3;
bool vmm_paging_not_identity = false;

void vmm_flush_tlb(void)
{
	uint64_t cr3 = read_cr3();
	write_cr3(cr3);
}

#define CR4_PGE (1 << 7)
void vmm_full_flush_tlb(void)
{
	vmm_flush_tlb();
	uint64_t cr4 = read_cr4();
	cr4 ^= CR4_PGE;
	write_cr4(cr4);
	cr4 ^= CR4_PGE;
	write_cr4(cr4);
}

#if 0
static int identity_paging(void)
{
	union cr3 * cr3 = (void *) &kernel_cr3;

	union pml4e * pml4 = palloc(0, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
	if (pml4 == NULL)
		return -ENOMEM;
	for (size_t i = 0; i < PAGE_TABLE_ELEMENTS; i++)
		pml4[i].absent.p = false;

	union pdpte * pdpt = palloc(0, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
	if (pdpt == NULL) {
		punmap(0, pml4, PAGE_TABLE_SIZE);
		return -ENOMEM;
	}
	for (size_t i = 0; i < PAGE_TABLE_ELEMENTS; i++)
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
#endif

/* public: mm/early.h */
int vmm_init(void) {
	/* This function expects memory to be identity mapped */
	int err;

#ifdef BOOTLOADER
	err = vkernel_init();
	if (err) {
		pr_err("vkernel_init: failed, errno = %d\n", err);
		return err;
	}
	err = vkernel_identity_paging();
	if (err) {
		pr_err("vkernel_identity_paging failed, errno = %d\n", err);
		return err;
	}
#else
	err = vkernel_init();
	if (err)
		return err;

	union cr3 * cr3 = (void *) &kernel_cr3;
	union pml4e * pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		return -ENOMEM;

	err = physical_init(pml4);
	if (err)
		return err;
#endif
	return 0;
}

#define X86_64_MSR_EFER 0xC0000080
static void enable_paging(void) {
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) | (1 << 8)); // Set EFER.LME
	write_cr4(read_cr4() | (1 << 5)); // Set CR4.PAE
	write_cr0(read_cr0() | ((unsigned) 1 << 31)); // Set CR0.PG
}

/* public: mm/early.h */
void vmm_enable_paging(void)
{
	vmm_paging_not_identity = true;
	write_cr3(kernel_cr3);
	enable_paging();
	vmm_full_flush_tlb();
}
