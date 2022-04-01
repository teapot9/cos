#include "paging.h"

#include <errno.h>

#include <mm/helper.h>

#define is_table_entry(entry) ( \
	same_type(entry, union pml4e) || same_type(entry, union pdpte) \
	|| same_type(entry, union pde) || same_type(entry, union pte) \
)

#define table_init_entry(entry) do { \
	static_assert(is_table_entry(*entry), "table_init_entry: bad type"); \
	*entry = (typeof(*entry)) { .absent = { .p = false, } }; \
} while (0)

#define table_init(table) do { \
	for (size_t k = 0; k < PAGE_TABLE_ELEMENTS; k++) \
		table_init_entry(&table[k]); \
} while (0)

void table_init_pml4(union pml4e * table) {
	table_init(table);
}

void table_init_pdpt(union pdpte * table) {
	table_init(table);
}

void table_init_pd(union pde * table) {
	table_init(table);
}

void table_init_pt(union pte * table) {
	table_init(table);
}

int page_map_pdpt(union pdpte * entry, void * paddr) {
	if (entry->absent.p)
		return -EBUSY;
	if (!is_aligned(paddr, PAGE_SIZE_PDPT))
		return -EINVAL;
	*entry = (union pdpte) { .page = {
		.p = true,
		.rw = true,
		.us = false,
		.pwt = false,
		.pcd = false,
		.a = false,
		.d = false,
		.ps = true,
		.g = false,
		.pat = false,
		.page = (unsigned long) paddr >> 12,
		.pk = 0,
		.xd = true,
	}};
	return 0;
}

int page_map_pd(union pde * entry, void * paddr) {
	if (entry->absent.p)
		return -EBUSY;
	if (!is_aligned(paddr, PAGE_SIZE_PD))
		return -EINVAL;
	*entry = (union pde) { .page = {
		.p = true,
		.rw = true,
		.us = false,
		.pwt = false,
		.pcd = false,
		.a = false,
		.d = false,
		.ps = true,
		.g = false,
		.pat = false,
		.page = (unsigned long) paddr >> 12,
		.pk = 0,
		.xd = true,
	}};
	return 0;
}

int page_map_pt(union pte * entry, void * paddr) {
	if (entry->absent.p)
		return -EBUSY;
	if (!is_aligned(paddr, PAGE_SIZE_PT))
		return -EINVAL;
	*entry = (union pte) { .page = {
		.p = true,
		.rw = true,
		.us = false,
		.pwt = false,
		.pcd = false,
		.a = false,
		.d = false,
		.pat = false,
		.g = false,
		.page = (unsigned long) paddr >> 12,
		.pk = 0,
		.xd = true,
	}};
	return 0;
}

int page_unmap_pdpt(union pdpte * entry) {
	if (!entry->absent.p)
		return -ENOENT;
	*entry = (union pdpte) { .absent = { .p = false } };
	return 0;
}

int page_unmap_pd(union pde * entry) {
	if (!entry->absent.p)
		return -ENOENT;
	*entry = (union pde) { .absent = { .p = false } };
	return 0;
}

int page_unmap_pt(union pte * entry) {
	if (!entry->absent.p)
		return -ENOENT;
	*entry = (union pte) { .absent = { .p = false } };
	return 0;
}

int page_set_pdpt(union pdpte * entry, bool write, bool user, bool exec,
                  bool accessed, bool dirty) {
	if (!entry->absent.p)
		 return -ENOENT;
	entry->page.rw = write;
	entry->page.us = user;
	entry->page.xd = !exec;
	entry->page.a = accessed;
	entry->page.d = dirty;
	return 0;
}

int page_set_pd(union pde * entry, bool write, bool user, bool exec,
		bool accessed, bool dirty) {
	if (!entry->absent.p)
		 return -ENOENT;
	entry->page.rw = write;
	entry->page.us = user;
	entry->page.xd = !exec;
	entry->page.a = accessed;
	entry->page.d = dirty;
	return 0;
}

int page_set_pt(union pte * entry, bool write, bool user, bool exec,
		bool accessed, bool dirty) {
	if (!entry->absent.p)
		 return -ENOENT;
	entry->page.rw = write;
	entry->page.us = user;
	entry->page.xd = !exec;
	entry->page.a = accessed;
	entry->page.d = dirty;
	return 0;
}

int table_add_pml4(union cr3 * cr3, void * paddr) {
	*cr3 = (union cr3) { .normal = {
		.pcd = false,
		.pwt = false,
		.table = (unsigned long) paddr >> 12,
	}};
	return 0;
}

int table_add_pdpt(union pml4e * entry, void * paddr,
		   bool write, bool user, bool exec) {
	if (entry->absent.p)
		return -EBUSY;
	*entry = (typeof(*entry)) { .pdpt = {
		.p = true,
		.rw = write,
		.us = user,
		.pwt = false,
		.pcd = false,
		.a = false,
		.ps = false,
		.table = (unsigned long) paddr >> 12,
		.xd = !exec,
	}};
	return 0;
}

int table_add_pd(union pdpte * entry, void * paddr,
		 bool write, bool user, bool exec) {
	if (entry->absent.p)
		return -EBUSY;
	*entry = (typeof(*entry)) { .pd = {
		.p = true,
		.rw = write,
		.us = user,
		.pwt = false,
		.pcd = false,
		.a = false,
		.ps = false,
		.table = (unsigned long) paddr >> 12,
		.xd = !exec,
	}};
	return 0;
}

int table_add_pt(union pde * entry, void * paddr,
		 bool write, bool user, bool exec) {
	if (entry->absent.p)
		return -EBUSY;
	*entry = (typeof(*entry)) { .pt = {
		.p = true,
		.rw = write,
		.us = user,
		.pwt = false,
		.pcd = false,
		.a = false,
		.ps = false,
		.table = (unsigned long) paddr >> 12,
		.xd = !exec,
	}};
	return 0;
}
