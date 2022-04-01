#ifndef MM_PAGE_H
#define MM_PAGE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <mm.h>
#include <asm/asm.h>

#define IMAGE_BASE (void *) 0xffffffff80000000

#define NB_PML4_ENTRY 512
#define NB_PDPT_ENTRY 512
#define NB_PD_ENTRY 512
#define NB_PT_ENTRY 512

#define PAGE_SIZE_PT 4096L
#define PAGE_SIZE_PD (NB_PT_ENTRY * PAGE_SIZE_PT)
#define PAGE_SIZE_PDPT (NB_PD_ENTRY * PAGE_SIZE_PD)
#define PAGE_SIZE_PML4 (NB_PDPT_ENTRY * PAGE_SIZE_PDPT)

#define MIN_PAGE_SIZE PAGE_SIZE_PT
#define PAGE_TABLE_ALIGN MIN_PAGE_SIZE
#define VIRTUAL_MEMORY_SIZE (NB_PML4_ENTRY * PAGE_SIZE_PML4)

union cr3 {
	struct __attribute__((packed)) cr3_normal {
		unsigned int _ignored1 : 3;
		bool pwt : 1;
		bool pcd : 1;
		unsigned int _ignored2 : 7;
		unsigned long int table : 52;
	} normal;
	struct __attribute__((packed)) cr3_pcide {
		unsigned int pcid : 12;
		unsigned long int table : 52;
	} pcide;
};
static_assert(sizeof(union cr3) == 8, "cr3 must be 64 bits");

union pml4e {
	struct __attribute__((packed)) pml4_pdpt {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		unsigned int _ignored1: 1;
		bool ps : 1;
		unsigned int _ignored2 : 4;
		unsigned long int table : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pdpt;
	struct __attribute__((packed)) pml4_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pml4e) == 8 && sizeof(struct pml4_pdpt) == 8
              && sizeof(struct pml4_absent) == 8, "pml4e must be 64 bits");

union pdpte {
	struct __attribute__((packed)) pdpt_page {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		bool d : 1;
		bool ps : 1;
		bool g : 1;
		unsigned int _ignored1 : 3;
		bool pat : 1;
		unsigned int _zero1 : 17;
		unsigned long int page : 22;
		unsigned int _ignored2 : 7;
		unsigned int pk : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pdpt_pd {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		unsigned int _ignored1 : 1;
		bool ps : 1;
		unsigned int _ignored2 : 4;
		unsigned long int table : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pd;
	struct __attribute__((packed)) pdpt_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pdpte) == 8 && sizeof(struct pdpt_page) == 8
              && sizeof(struct pdpt_pd) == 8 && sizeof(struct pdpt_absent) == 8,
              "pdpte must be 64 bits");

union pde {
	struct __attribute__((packed)) pd_page {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		bool d : 1;
		bool ps : 1;
		bool g : 1;
		unsigned int _ignored1 : 3;
		bool pat : 1;
		unsigned int _zero1 : 8;
		unsigned long int page : 31;
		unsigned int _ignored2 : 7;
		unsigned int pk : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pd_pt {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		unsigned int _ignored1 : 1;
		bool ps : 1;
		unsigned int _ignored2 : 4;
		unsigned long int table : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pt;
	struct __attribute__((packed)) pd_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pde) == 8 && sizeof(struct pd_page) == 8
              && sizeof(struct pd_pt) == 8 && sizeof(struct pd_absent) == 8,
              "pde must be 64 bits");

union pte {
	struct __attribute__((packed)) pt_page {
		bool p : 1;
		bool rw : 1;
		bool us : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool a : 1;
		bool d : 1;
		bool pat : 1;
		bool g : 1;
		unsigned int _ignored1 : 3;
		unsigned long int page : 40;
		unsigned int _ignored2 : 7;
		unsigned int pk : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pt_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pte) == 8 && sizeof(struct pt_page) == 8
              && sizeof(struct pt_absent) == 8, "pte must be 64 bits");

union lin_addr {
	struct __attribute__((packed)) lin_addr_pdpt {
		unsigned long int offset : 30;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pdpt;
	struct __attribute__((packed)) lin_addr_pd {
		unsigned long int offset : 21;
		unsigned long int pd : 9;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pd;
	struct __attribute__((packed)) lin_addr_pt {
		unsigned long int offset : 12;
		unsigned long int pt : 9;
		unsigned long int pd : 9;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pt;
};
static_assert(sizeof(union lin_addr) == 8
              && sizeof(struct lin_addr_pdpt) == 8
	      && sizeof(struct lin_addr_pd) == 8
	      && sizeof(struct lin_addr_pt) == 8,
	      "lin_addr must be 64 bits");

struct page {
	size_t size;
	union {
		union cr3 * cr3;
		union pml4e * pml4;
		union pdpte * pdpt;
		union pde * pd;
		union pte * pt;
	} page;
};

struct table_vaddr {
	void * vaddr;
	struct table_vaddr * subtables;
};

static inline bool _ptr_aligned(void * addr, size_t size)
{
	return (unsigned long) addr % size == 0;
}

static inline size_t page_size_max(void * vaddr, void * paddr, size_t size)
{
	if (_ptr_aligned(vaddr, PAGE_SIZE_PDPT)
	    && _ptr_aligned(paddr, PAGE_SIZE_PDPT)
	    && size >= PAGE_SIZE_PDPT)
		return PAGE_SIZE_PDPT;
	if (_ptr_aligned(vaddr, PAGE_SIZE_PD)
	    && _ptr_aligned(paddr, PAGE_SIZE_PD)
	    && size >= PAGE_SIZE_PD)
		return PAGE_SIZE_PD;
	if (_ptr_aligned(vaddr, PAGE_SIZE_PT)
	    && _ptr_aligned(paddr, PAGE_SIZE_PT)
	    && size >= PAGE_SIZE_PT)
		return PAGE_SIZE_PT;
	return 0;
}

static inline void invlpg(void * ptr)
{
	asm volatile (intel("invlpg [rax]\n") : : "a" (ptr));
}

struct page page_get(pid_t pid, void * vaddr, bool p);
void * page_paddr(pid_t pid, void * vaddr);
void * page_find_free(pid_t pid, size_t align, size_t count);

int page_map(pid_t pid, void * vaddr, void * paddr,
             size_t size, struct page_perms perms);
int page_unmap(pid_t pid, void * vaddr, size_t size);
int page_set(pid_t pid, void * vaddr, size_t size,
             struct page_perms perms, bool accessed, bool dirty);

void * virt_to_phys(pid_t pid, void * vaddr);
int page_new_vm(union cr3 * cr3, struct table_vaddr ** vroot);
struct memlist;
int page_kmem_setup(struct memlist * l, union cr3 cr3,
                    struct table_vaddr ** vroot);
int page_kmem_finalize(struct table_vaddr * kvroot, struct table_vaddr * vroot);

#ifdef BOOTLOADER
int page_identity(union cr3 * cr3);
#endif // BOOTLOADER

#endif // MM_PAGE_H
