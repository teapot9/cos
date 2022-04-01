#ifndef MM_VMM_H
#define MM_VMM_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define NB_PML4_ENTRY 512
#define NB_PDPT_ENTRY 512
#define NB_PD_ENTRY 512
#define NB_PT_ENTRY 512
#define PAGE_SIZE_PT 4096L
#define BASE_BAGE_SIZE PAGE_SIZE_PT
#define PAGE_SIZE_PD (NB_PT_ENTRY * PAGE_SIZE_PT)
#define PAGE_SIZE_PDPT (NB_PD_ENTRY * PAGE_SIZE_PD)
#define PAGE_SIZE_PML4 (NB_PDPT_ENTRY * PAGE_SIZE_PDPT)
#define VIRTUAL_MEMORY_SIZE (NB_PML4_ENTRY * PAGE_SIZE_PML4)

union cr3 {
	struct __attribute__((packed)) cr3_normal {
		unsigned int _ignored1 : 3;
		bool pwt : 1;
		bool pcd : 1;
		unsigned int _ignored2 : 7;
		unsigned long int pml4 : 52;
	} normal;
	struct __attribute__((packed)) cr3_pcide {
		unsigned int pcid : 12;
		unsigned long int pml4 : 52;
	} pcide;
};
static_assert(sizeof(union cr3) == 8, "cr3 must be 64 bits");

union pml4e {
	struct __attribute__((packed)) pml4_pdpt {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		unsigned int _ignored1: 1;
		bool page_size : 1;
		unsigned int _ignored2 : 4;
		unsigned long int pdpt : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pdpt;
	struct __attribute__((packed)) pml4_absent {
		bool present : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pml4e) == 8 && sizeof(struct pml4_pdpt) == 8
              && sizeof(struct pml4_absent) == 8, "pml4e must be 64 bits");

union pdpte {
	struct __attribute__((packed)) pdpt_page {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		bool dirty : 1;
		bool page_size : 1;
		bool global : 1;
		unsigned int _ignored1 : 3;
		bool pat : 1;
		unsigned int _zero1 : 17;
		unsigned long int page : 22;
		unsigned int _ignored2 : 7;
		unsigned int protection_key : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pdpt_pd {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		unsigned int _ignored1 : 1;
		bool page_size : 1;
		unsigned int _ignored2 : 4;
		unsigned long int pd : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pd;
	struct __attribute__((packed)) pdpt_absent {
		bool present : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pdpte) == 8 && sizeof(struct pdpt_page) == 8
              && sizeof(struct pdpt_pd) == 8 && sizeof(struct pdpt_absent) == 8,
              "pdpte must be 64 bits");

union pde {
	struct __attribute__((packed)) pd_page {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		bool dirty : 1;
		bool page_size : 1;
		bool global : 1;
		unsigned int _ignored1 : 3;
		bool pat : 1;
		unsigned int _zero1 : 8;
		unsigned long int page : 31;
		unsigned int _ignored2 : 7;
		unsigned int protection_key : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pd_pt {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		unsigned int _ignored1 : 1;
		bool page_size : 1;
		unsigned int _ignored2 : 4;
		unsigned long int pt : 40;
		unsigned int _ignored3 : 11;
		bool xd : 1;
	} pt;
	struct __attribute__((packed)) pd_absent {
		bool present : 1;
		unsigned long int _ignored1 : 63;
	} absent;
};
static_assert(sizeof(union pde) == 8 && sizeof(struct pd_page) == 8
              && sizeof(struct pd_pt) == 8 && sizeof(struct pd_absent) == 8,
              "pde must be 64 bits");

union pte {
	struct __attribute__((packed)) pt_page {
		bool present : 1;
		bool write_access : 1;
		bool user_access : 1;
		bool pwt : 1;
		bool pcd : 1;
		bool accessed : 1;
		bool dirty : 1;
		bool pat : 1;
		bool global : 1;
		unsigned int _ignored1 : 3;
		unsigned long int page : 40;
		unsigned int _ignored2 : 7;
		unsigned int protection_key : 4;
		bool xd : 1;
	} page;
	struct __attribute__((packed)) pt_absent {
		bool present : 1;
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
	enum {
		PAGE_TYPE_PML4, PAGE_TYPE_PDPT, PAGE_TYPE_PD, PAGE_TYPE_PT,
	} type;
	union {
		union pml4e * pml4;
		union pdpte * pdpt;
		union pde * pd;
		union pte * pt;
	} page;
};

struct mmap {
	void * ptr;
	size_t size;
};

void vmm_init(struct memmap map);

void vfree(void * addr, size_t len);

struct mmap vmalloc(size_t size);

struct mmap mmap(union pml4e * pml4, void * addr, size_t size);

void * get_paddr(union pml4e * pml4, void * ptr);

struct page get_page(union pml4e * pml4, void * addr);

#endif // MM_VMM_H
