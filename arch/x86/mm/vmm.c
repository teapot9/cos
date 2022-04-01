#define pr_fmt(fmt) "vmm: " fmt

#include <mm.h>
#include "vmm.h"

#include <errno.h>

#include <panic.h>
#include <asm/cpu.h>
#include "../../../mm/pmm.h"
#include <print.h>
#include "page.h"
#include <mm/vmemmap.h>

bool vmm_partially_init = false;
bool vmm_fully_init = false;

int vmap(pid_t pid, void * paddr, void * vaddr, size_t size,
         struct page_perms perms)
{
	if (!vmm_partially_init)
		return -EAGAIN;

#ifndef BOOTLOADER
	if ((pid == 0 && vaddr < IMAGE_BASE)
	    || (pid != 0 && vaddr >= IMAGE_BASE))
		return -EACCES;
#endif // !BOOTLOADER
	if (!size)
		return 0;
	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	int err;
	size_t s = size;
	uint8_t * cpaddr = paddr;
	uint8_t * cvaddr = vaddr;

	while (s > 0) {
		size_t psize = page_size_max(cvaddr, cpaddr, s);
		if (psize == 0) {
			err = -EINVAL;
			break;
		}

		err = page_map(pid, cvaddr, cpaddr, psize, perms);
		if (err)
			break;

		s -= psize;
		cvaddr += psize;
		cpaddr += psize;
	}

	if (err)
		page_unmap(pid, vaddr, s);
	return err;
}

int vunmap(pid_t pid, void * vaddr, size_t size)
{
	if (!vmm_partially_init)
		return -EAGAIN;
	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	return page_unmap(pid, vaddr, size);
}

int vinit(pid_t pid, void * vaddr, size_t size, struct page_perms perms)
{
	if (!vmm_partially_init)
		return -EAGAIN;
	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	return page_set(pid, vaddr, size, perms, false, false);
}

void * mmap(pid_t pid, void * paddr, size_t size, size_t valign,
	    struct page_perms perms)
{
	if (!vmm_partially_init)
		return NULL;

	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	int err;

	if (valign % MIN_PAGE_SIZE)
		return NULL;
	if (!valign)
		valign = page_size_max(0, paddr, size);
	size_t pcount = size / valign;
	if (size % valign)
		pcount++;
	void * vaddr = page_find_free(pid, valign, pcount);

	err = vmap(pid, paddr, vaddr, size, perms);
	if (err)
		return NULL;
	return vaddr;
}

void * valloc(pid_t pid, size_t size, size_t palign, size_t valign,
	      struct page_perms perms)
{
	if (!vmm_partially_init)
		return NULL;

	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	if (palign % MIN_PAGE_SIZE)
		return NULL;
	if (!palign)
		palign = page_size_max(0, 0, size);
	void * paddr = palloc(pid, size, palign);

	void * vaddr = mmap(pid, paddr, size, valign, perms);
	if (vaddr == NULL) {
		punmap(pid, paddr, size);
		return NULL;
	}
	return vaddr;
}

int vfree(pid_t pid, void * vaddr, size_t size)
{
	if (!vmm_partially_init)
		return -EAGAIN;

	if (size % MIN_PAGE_SIZE)
		size += MIN_PAGE_SIZE - (size % MIN_PAGE_SIZE);
	int err = vunmap(pid, vaddr, size);
	punmap(pid, vaddr, size);
	return err;
}

static inline void flush_tlb(void)
{
	uint64_t cr3 = read_cr3();
	write_cr3(cr3);
}

#define CR4_PGE (1 << 7)
static inline void full_flush_tlb(void)
{
	uint64_t cr4 = read_cr4();
	cr4 ^= CR4_PGE;
	write_cr4(cr4);
	cr4 ^= CR4_PGE;
	write_cr4(cr4);
}

#define X86_64_MSR_EFER 0xC0000080
static inline void enable_paging(void)
{
	write_msr(X86_64_MSR_EFER,
		  read_msr(X86_64_MSR_EFER) | (1 << 8)); // Set EFER.LME
	write_cr4(read_cr4() | (1 << 5)); // Set CR4.PAE
	write_cr0(read_cr0() | (1 << 31)); // Set CR0.PG
}

#ifndef BOOTLOADER
int vmm_init(struct vmemmap * map)
#else // !BOOTLOADER
int vmm_init(void)
#endif // BOOTLOADER
{
	/* This function assumes memory is currently mapped 1:1 */
	int err = 0;
	vmm_partially_init = true;
#ifndef BOOTLOADER
	uint64_t cur_cr3r = read_cr3();
	union cr3 cur_cr3 = *(union cr3 *) &cur_cr3r;
	struct memlist ktables = memlist_new_default();
	struct table_vaddr * kvroot;
	err = page_kmem_setup(&ktables, cur_cr3, &kvroot);
	if (err) {
		panic("failed to save kernel virtual memory tables"
		      ", errno = %d", err);
	}

	union cr3 new_cr3;
	struct table_vaddr * vroot;
	err = page_new_vm(&new_cr3, &vroot);
	if (err) {
		panic("failed to create initial virtual memory space"
		      ", errno = %d", err);
	}

	err = page_kmem_finalize(kvroot, vroot);
	if (err) {
		panic("page_kmem_finalize failed, errno = %d", err);
	}
	/*
	struct vmemmap_elt * cur;
	list_foreach(cur, map->l.l.first) {
		int lerr =
			vmap(0, cur->map, cur->l.addr, cur->l.size, cur->perms);
		if (lerr) {
			pr_err("failed to map %p->%p [%zu bytes], errno = %d\n",
			       cur->l.addr, cur->map, cur->l.size, err);
			err = -ENOMEM;
		}
	}
	*/
#else // !BOOTLOADER
	union cr3 new_cr3;
	err = page_identity(&new_cr3);
	if (err)
		return err;
	write_cr3(*(uint64_t *) &new_cr3);
	enable_paging();
	full_flush_tlb();
#endif // BOOTLOADER
	vmm_fully_init = true;
	return err;
}
