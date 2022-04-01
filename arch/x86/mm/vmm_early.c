#define pr_fmt(fmt) "vmm: " fmt

#include <mm.h>
#include "vmm.h"

#include <stdbool.h>
#include <errno.h>

#include <mm/early.h>
#include <mm/helper.h>
#include <mm/memmap.h>
#include <asm/cpu.h>
#include "../../../mm/pmm.h"
#include <print.h>
#include <debug.h>

static union pml4e * pml4 = NULL;
static struct early_vmap * early_vmap_first = NULL;
bool vmm_fully_init = false;

union pml4e * kpml4(void)
{
	return pml4;
}

union pml4e * cur_pml4(void)
{
	uint64_t raw_cr3 = read_cr3();
	union cr3 * cr3 = (void *) &raw_cr3;
	return (union pml4e *) cr3->normal.pml4;
}

static struct early_vmap ** early_find_overlap(void * vaddr, size_t size)
{
	struct early_vmap ** prec = &early_vmap_first;
	struct early_vmap * cur = early_vmap_first;

	while (cur != NULL
	       && !is_overlap(vaddr, size, cur->vaddr, cur->size)) {
		prec = &(*prec)->next;
		cur = cur->next;
	}
	return prec;
}

int early_vmap(void * paddr, void * vaddr, size_t size)
{
	if (vmm_is_init())
		return -EINVAL;

	struct early_vmap * cur = *early_find_overlap(vaddr, size);

	if (cur != NULL) {
		pr_err("cannot map %p to %p, already mapped\n", vaddr, paddr);
		return -EBUSY;
	}

	struct early_vmap * new = kmalloc(sizeof(*new));
	if (new == NULL) {
		pr_err("cannot allocate memory for list item\n", 0);
		return -ENOMEM;
	}

	new->paddr = paddr;
	new->vaddr = vaddr;
	new->size = size;
	new->next = early_vmap_first;
	early_vmap_first = new;
#ifdef CONFIG_MM_DEBUG
	pr_debug("early_vmap(%p, %p, %zu) -> %d\n", paddr, vaddr, size, 0);
#endif
	return 0;
}

void early_vunmap(void * vaddr, size_t size)
{
	if (vmm_is_init())
		return;

	struct early_vmap ** prec = early_find_overlap(vaddr, size);
	if (*prec == NULL) {
		pr_err("nothing to unmap at %p\n", vaddr);
		return;
	}

	// Save current mapping info
	struct early_vmap block = **prec;

	// Remove mapping
	struct early_vmap * pblock = *prec;
	*prec = (*prec)->next;
	kfree(pblock);

	uint8_t * block_start = block.vaddr;
	uint8_t * block_end = block_start + block.size;
	uint8_t * target_start = vaddr;
	uint8_t * target_end = target_start + size;

	if (block_start < target_start) {
		// There is mapping before, we keep it
		size_t diff_before = target_start - block_start;
		early_vmap(block.paddr, block_start, diff_before);
	}

	if (block_end > target_end) {
		// There is mapping after, we keep it
		size_t diff_after = block_end - target_end;
		early_vmap(
			(uint8_t *) block.paddr + (target_end - block_start),
			target_end, diff_after
		);
	}
#ifdef CONFIG_MM_DEBUG
	pr_debug("early_vunmap(%p, %zu)\n", vaddr, size);
#endif
}

void * early_mmap(void * paddr, size_t size)
{
	if (vmm_is_init())
		return NULL;

	void * vaddr = (pmap(paddr, size) || early_vmap(paddr, paddr, size))
		? NULL : paddr;
#ifdef CONFIG_MM_DEBUG
	pr_debug("early_mmap(%p, %zu) -> %p\n", paddr, size, vaddr);
#endif
	return vaddr;
}

#define X86_64_MSR_EFER 0xC0000080

UNUSED static inline void disable_paging(void)
{
	write_cr0(read_cr0() & ~(1 << 31)); // Clear CR0.PG
	write_cr4(read_cr4() & ~(1 << 5)); // Clear CR4.PAE
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) & ~(1 << 8)); // Clear EFER.LME
}

static inline void enable_paging(void)
{
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) | (1 << 8)); // Set EFER.LME
	write_cr4(read_cr4() | (1 << 5)); // Set CR4.PAE
	write_cr0(read_cr0() | (1 << 31)); // Set CR0.PG
}

/* public: platform_setup.h */
int vmm_init(void)
{
	int err;
	if (vmm_is_init())
		return 0;

	uint64_t raw_cr4 = read_cr4();
	struct cr4 * cr4 = (void *) &raw_cr4;
	if (cr4->pcide) {
		pr_notice("disabling CR4.PCIDE\n", 0);
		cr4->pcide = false;
		write_cr4(raw_cr4);
	}

	pml4 = pmalloc(NB_PML4_ENTRY * sizeof(*pml4), 4096);
	if (pml4 == NULL) {
		pr_emerg("failed to allocate PML4\n", 0);
		return -ENOMEM;
	}

	for (size_t i = 0; i < NB_PML4_ENTRY; i++) {
		struct pml4_absent base_pml4e = {.present = false};
		pml4[i].absent = base_pml4e;
	}
	err = vmap(pml4, pml4, 4096);
	if (err) {
		pr_crit("failed to create virtual memory mapping for PML4, "
			"errno = %d\n", err);
		return err;
	}

	struct early_vmap * cur = early_vmap_first;
	while (cur != NULL) {
		// if (cur->paddr == NULL || cur->vaddr == NULL
		if (cur->size == 0) {
			pr_err("fnvalid early vmap descriptor at %p\n", cur);
			cur = cur->next;
			continue;
		}
		err = vmap(cur->paddr, cur->vaddr, cur->size);
		if (err) {
			pr_crit("failed to create virtual memory mapping for "
				"vmap descriptor at %p, errno = %d\n",
				cur, err);
		}
		cur = cur->next;
	}

	//uint64_t raw_cr3 = read_cr3();
	//union cr3 * cr3 = (void *) &raw_cr3;
	//cr3->normal.pml4 = (unsigned long) pml4 >> 12;
	write_cr3(kcr3());
	enable_paging();

	vmm_fully_init = true;
	return 0;
}
