#ifndef _MM_PAGING_H
#define _MM_PAGING_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "vmm.h"
#include <mm/block.h>
#include <alloc.h>
#include <asm/asm.h>
#include <cpp.h>

#define PAGE_TABLE_ELEMENTS 512
#define PAGE_TABLE_SIZE 4096
#define PAGE_SIZE_PT ((size_t) 4096UL)
#define PAGE_SIZE_PD (PAGE_SIZE_PT * 512)
#define PAGE_SIZE_PDPT (PAGE_SIZE_PD * 512)
#define PAGE_SIZE_PML4 (PAGE_SIZE_PDPT * 512)

/* Data types */

union cr3 {
	struct _packed_ cr3_normal {
		unsigned int _ignored1 : 3;
		bool pwt : 1;
		bool pcd : 1;
		unsigned int _ignored2 : 7;
		unsigned long int table : 52;
	} normal;
	struct _packed_ cr3_pcide {
		unsigned int pcid : 12;
		unsigned long int table : 52;
	} pcide;
};
static_assert(sizeof(union cr3) == 8, "cr3 must be 64 bits");

union pml4e {
	struct _packed_ pml4_pdpt {
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
	struct _packed_ pml4_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pml4e) == 8 && sizeof(struct pml4_pdpt) == 8
              && sizeof(struct pml4_absent) == 8, "pml4e must be 64 bits");

union pdpte {
	struct _packed_ pdpt_page {
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
	struct _packed_ pdpt_pd {
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
	struct _packed_ pdpt_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pdpte) == 8 && sizeof(struct pdpt_page) == 8
              && sizeof(struct pdpt_pd) == 8 && sizeof(struct pdpt_absent) == 8,
              "pdpte must be 64 bits");

union pde {
	struct _packed_ pd_page {
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
	struct _packed_ pd_pt {
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
	struct _packed_ pd_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pde) == 8 && sizeof(struct pd_page) == 8
              && sizeof(struct pd_pt) == 8 && sizeof(struct pd_absent) == 8,
              "pde must be 64 bits");

union pte {
	struct _packed_ pt_page {
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
	struct _packed_ pt_absent {
		bool p : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pte) == 8 && sizeof(struct pt_page) == 8
              && sizeof(struct pt_absent) == 8, "pte must be 64 bits");

union linear_addr {
	struct _packed_ lin_addr_pdpt {
		unsigned long int offset : 30;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pdpt;
	struct _packed_ lin_addr_pd {
		unsigned long int offset : 21;
		unsigned long int pd : 9;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pd;
	struct _packed_ lin_addr_pt {
		unsigned long int offset : 12;
		unsigned long int pt : 9;
		unsigned long int pd : 9;
		unsigned long int pdpt : 9;
		unsigned long int pml4 : 9;
		unsigned long int _unused : 16;
	} pt;
};
static_assert(sizeof(union linear_addr) == 8
              && sizeof(struct lin_addr_pdpt) == 8
	      && sizeof(struct lin_addr_pd) == 8
	      && sizeof(struct lin_addr_pt) == 8,
	      "linear_addr must be 64 bits");

/* Low level paging functions */

// Informational
void * virt_to_phys_raw(union cr3 * cr3, void * vaddr);

// Initialize existing table
void table_init_pml4(union pml4e * table);
void table_init_pdpt(union pdpte * table);
void table_init_pd(union pde * table);
void table_init_pt(union pte * table);

// Map physical page to virtual address
int page_map_pdpt(union pdpte * entry, void * paddr);
int page_map_pd(union pde * entry, void * paddr);
int page_map_pt(union pte * entry, void * paddr);

// Unmap physical page
int page_unmap_pdpt(union pdpte * entry);
int page_unmap_pd(union pde * entry);
int page_unmap_pt(union pte * entry);

// Configure existing mapping
int page_set_pdpt(union pdpte * entry, bool write, bool user, bool exec,
                  bool accessed, bool dirty);
int page_set_pd(union pde * entry, bool write, bool user, bool exec,
		bool accessed, bool dirty);
int page_set_pt(union pte * entry, bool write, bool user, bool exec,
		bool accessed, bool dirty);

// Add initialized table to its parent
int table_add_pml4(union cr3 * cr3, void * paddr);
int table_add_pdpt(union pml4e * entry, void * paddr,
		   bool write, bool user, bool exec);
int table_add_pd(union pdpte * entry, void * paddr,
		 bool write, bool user, bool exec);
int table_add_pt(union pde * entry, void * paddr,
		 bool write, bool user, bool exec);

// Conversion functions
static inline union pml4e * get_pml4(const union cr3 * cr3)
{
	return (void *) (cr3->normal.table << 12);
}
static inline union pdpte * get_pdpt(const union pml4e * entry)
{
	if (!entry->absent.p)
		return NULL;
	return (void *) (entry->pdpt.table << 12);
}
static inline union pde * get_pd(const union pdpte * entry)
{
	if (!entry->absent.p || entry->page.ps)
		return NULL;
	return (void *) (entry->pd.table << 12);
}
static inline union pte * get_pt(const union pde * entry)
{
	if (!entry->absent.p || entry->page.ps)
		return NULL;
	return (void *) (entry->pt.table << 12);
}

/* Helper */
static inline size_t target_page_size(size_t needed_size) {
	if (needed_size >= PAGE_SIZE_PDPT)
		return PAGE_SIZE_PDPT;
	if (needed_size >= PAGE_SIZE_PD)
		return PAGE_SIZE_PD;
	return PAGE_SIZE_PT;
}

/* Process level paging functions */

static inline bool is_kmem(void * base, size_t size) {
	return is_overlap(
		base, size, KERNEL_SPACE_START,
		(uint64_t) KERNEL_SPACE_END - (uint64_t) KERNEL_SPACE_START
	);
}
static inline bool is_umem(void * base, size_t size) {
	return is_overlap(
		base, size, USER_SPACE_START,
		(uint64_t) USER_SPACE_END - (uint64_t) USER_SPACE_START
	);
}

#endif
