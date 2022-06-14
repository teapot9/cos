#include <mm/paging.h>
#include <mm/align.h>
#include <mm/pmm.h>
#include "paging.h"
#include "physical.h"
#include <errno.h>
#include <asm/cpu.h>
#include <mm/vmm.h>
#include <printk.h>

#ifndef BOOTLOADER
#include <task.h>
union cr3 * get_cr3(pid_t pid) {
	struct process * proc = process_get(pid);
	return proc == NULL ? NULL : process_cr3(proc);
}
#else
union cr3 * get_cr3(pid_t pid) {
	return pid == 0 ? (void *) &kernel_cr3 : NULL;
}
#endif

static void * to_kmem(void * vaddr) {
	if (vaddr == NULL)
		// Never NULL because NULL == 1:1 mapping
		return NULL;
	union linear_addr * laddr = (void *) &vaddr;
	laddr->pdpt._unused = ~0u;
	return vaddr;
}

void * virt_to_phys(pid_t pid, void * vaddr) {
	union cr3 * cr3 = get_cr3(pid);
	return virt_to_phys_raw(cr3, vaddr);
}

void * virt_to_phys_current(void * vaddr) {
	uint64_t cr3 = read_cr3();
	return virt_to_phys_raw((void *) &cr3, vaddr);
}

void * virt_to_phys_raw(union cr3 * cr3, void * vaddr) {
	uint64_t ret = (uint64_t) NULL;
	union pml4e * pml4 = NULL;
	union pdpte * pdpt = NULL;
	union pde * pd = NULL;
	union pte * pt = NULL;
	union linear_addr * laddr = (void *) &vaddr;
	if (cr3 == NULL)
		goto cleanup;

	pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		goto cleanup;
	union pml4e * pml4e = &pml4[laddr->pdpt.pml4];
	if (!pml4e->absent.p)
		goto cleanup;

	pdpt = physical_tmp_map(get_pdpt(pml4e));
	if (pdpt == NULL)
		goto cleanup;
	union pdpte * pdpte = &pdpt[laddr->pdpt.pdpt];
	if (!pdpte->absent.p)
		goto cleanup;
	if (pdpte->page.ps) {
		ret = (pdpte->page.page << 30) + laddr->pdpt.offset;
		goto cleanup;
	}

	pd = physical_tmp_map(get_pd(pdpte));
	if (pd == NULL)
		goto cleanup;
	union pde * pde = &pd[laddr->pd.pd];
	if (!pde->absent.p)
		goto cleanup;
	if (pde->page.ps) {
		ret = (pde->page.page << 21) + laddr->pd.offset;
		goto cleanup;
	}

	pt = physical_tmp_map(get_pt(pde));
	if(pt == NULL)
		goto cleanup;
	union pte * pte = &pt[laddr->pt.pt];
	if (!pte->absent.p)
		goto cleanup;
	ret = (pte->page.page << 12) + laddr->pt.offset;

cleanup:
	if (pml4 != NULL)
		physical_tmp_unmap(pml4);
	if (pdpt != NULL)
		physical_tmp_unmap(pdpt);
	if (pd != NULL)
		physical_tmp_unmap(pd);
	if (pt != NULL)
		physical_tmp_unmap(pt);
	return (void *) ret;
}

static int page_map_at(pid_t pmm_pid, union pml4e * pml4, void * vaddr,
                       size_t req_size, void * paddr, size_t * mapped_size) {
	int err = 0;
	union linear_addr * laddr = (void *) &vaddr;
	union pdpte * pdpt = NULL;
	union pde * pd = NULL;
	union pte * pt = NULL;
	bool write = true;
	bool exec = true;
	bool want_page;

	if (pmm_pid != 0 && is_kmem(vaddr, req_size)) {
		err = -EACCES;
		goto cleanup;
	}

	if (!is_aligned(vaddr, PAGE_SIZE_PT))
		return -EINVAL;

	want_page = false; // true if we want to create page this size
	*mapped_size = PAGE_SIZE_PML4;
	union pml4e * pml4e = &pml4[laddr->pdpt.pml4];
	if (!pml4e->absent.p) {
		// new table
		void * new_table =
			palloc(pmm_pid, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
		pdpt = physical_tmp_map(new_table);
		if (new_table == NULL || pdpt == NULL) {
			err = -ENOMEM;
			goto cleanup;
		}
		table_init_pdpt(pdpt);
		bool user = !is_kmem(vaddr, *mapped_size);
		err = table_add_pdpt(pml4e, new_table, write, user, exec);
		if (err)
			goto cleanup;
	} else {
		// get table
		pdpt = physical_tmp_map(get_pdpt(pml4e));
		if (pdpt == NULL) {
			err = -EINVAL;
			goto cleanup;
		}
	}

	*mapped_size = PAGE_SIZE_PDPT;
	want_page = is_aligned(vaddr, PAGE_SIZE_PDPT)
		&& req_size >= *mapped_size;
	union pdpte * pdpte = &pdpt[laddr->pdpt.pdpt];
	if (!pdpte->absent.p) {
		if (want_page) {
			// map page
			err = page_map_pdpt(pdpte, paddr);
			goto cleanup;
		}
		// new table
		void * new_table =
			palloc(pmm_pid, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
		pd = physical_tmp_map(new_table);
		if (new_table == NULL || pd == NULL) {
			err = -ENOMEM;
			goto cleanup;
		}
		table_init_pd(pd);
		bool user = !is_kmem(vaddr, *mapped_size);
		err = table_add_pd(pdpte, new_table, write, user, exec);
		if (err)
			goto cleanup;
	} else {
		if (pdpte->page.ps) {
			err = -EBUSY;
			goto cleanup;
		}
		// get table
		pd = physical_tmp_map(get_pd(pdpte));
		if (pd == NULL) {
			err = -EINVAL;
			goto cleanup;
		}
	}

	*mapped_size = PAGE_SIZE_PD;
	want_page = is_aligned(vaddr, PAGE_SIZE_PD)
		&& req_size >= *mapped_size;
	union pde * pde = &pd[laddr->pd.pd];
	if (!pde->absent.p) {
		if (want_page) {
			// map page
			err = page_map_pd(pde, paddr);
			goto cleanup;
		}
		// new table
		void * new_table =
			palloc(pmm_pid, PAGE_TABLE_SIZE, PAGE_TABLE_SIZE);
		pt = physical_tmp_map(new_table);
		if (new_table == NULL || pt == NULL) {
			err = -ENOMEM;
			goto cleanup;
		}
		table_init_pt(pt);
		bool user = !is_kmem(vaddr, *mapped_size);
		err = table_add_pt(pde, new_table, write, user, exec);
		if (err)
			goto cleanup;
	} else {
		if (pde->page.ps) {
			err = -EBUSY;
			goto cleanup;
		}
		// get table
		pt = physical_tmp_map(get_pt(pde));
		if (pt == NULL) {
			err = -EINVAL;
			goto cleanup;
		}
	}

	*mapped_size = PAGE_SIZE_PT;
	union pte * pte = &pt[laddr->pt.pt];
	if (!pte->absent.p) {
		// map page
		err = page_map_pt(pte, paddr);
		goto cleanup;
	}
	err = -EBUSY;
	goto cleanup;

cleanup:
	if (pdpt != NULL)
		physical_tmp_unmap(pdpt);
	if (pd != NULL)
		physical_tmp_unmap(pd);
	if (pt != NULL)
		physical_tmp_unmap(pt);
	return err;
}

int page_map(pid_t pid, void * vaddr, size_t * size, void * paddr) {
	// TODO: should tell the caller the size of the actual mapping
	int err;
	if (is_kmem(vaddr, *size))
		pid = 0;

	union cr3 * cr3 = get_cr3(pid);
	if (cr3 == NULL)
		return -EINVAL;

	union pml4e * pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		return -EINVAL;

	size_t cur = 0;
	uint8_t * vaddr8 = vaddr;
	uint8_t * paddr8 = paddr;
	while (cur < *size) {
		size_t psize;
		err = page_map_at(pid, pml4, vaddr8 + cur, *size - cur,
		                  paddr8 + cur, &psize);
		if (err) {
			pr_err("page_map: PID %zu: failed to map %p [%zu bytes]"
			       " to %p, errno = %d\n", pid, vaddr8 + cur,
			       size - cur, paddr8 + cur, err);
			break;
		}
		cur += psize;
	}

	physical_tmp_unmap(pml4);
	*size = cur; // tell caller the actual size of mapping
	return err;
}

static int page_set_at(
	union pml4e * pml4, void * vaddr, size_t * size,
	bool write, bool user, bool exec, bool accessed, bool dirty
) {
	int err = 0;
	union linear_addr * laddr = (void *) &vaddr;
	union pdpte * pdpt = NULL;
	union pde * pd = NULL;
	union pte * pt = NULL;

	*size = PAGE_SIZE_PML4;
	union pml4e * pml4e = &pml4[laddr->pdpt.pml4];
	if (!pml4e->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	pdpt = physical_tmp_map(get_pdpt(pml4e));
	if (pdpt == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PDPT;
	union pdpte * pdpte = &pdpt[laddr->pdpt.pdpt];
	if (!pdpte->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	if (pdpte->page.ps) {
		err = page_set_pdpt(pdpte, write, user, exec, accessed, dirty);
		goto cleanup;
	}
	pd = physical_tmp_map(get_pd(pdpte));
	if (pd == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PD;
	union pde * pde = &pd[laddr->pd.pd];
	if (!pde->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	if (pde->page.ps) {
		err = page_set_pd(pde, write, user, exec, accessed, dirty);
		goto cleanup;
	}
	pt = physical_tmp_map(get_pt(pde));
	if(pt == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PT;
	union pte * pte = &pt[laddr->pt.pt];
	if (!pte->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	err = page_set_pt(pte, write, user, exec, accessed, dirty);
	goto cleanup;

cleanup:
	if (pdpt != NULL)
		physical_tmp_unmap(pdpt);
	if (pd != NULL)
		physical_tmp_unmap(pd);
	if (pt != NULL)
		physical_tmp_unmap(pt);
	return err;
}

int page_set(pid_t pid, void * vaddr, size_t size, bool write, bool user,
             bool exec, bool accessed, bool dirty) {
	int err;
	union cr3 * cr3 = get_cr3(pid);
	if (cr3 == NULL)
		return -EINVAL;

	union pml4e * pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		return 0;

	size_t cur = 0;
	uint8_t * vaddr8 = vaddr;
	while (cur < size) {
		size_t psize;
		err = page_set_at(pml4, vaddr8 + cur, &psize,
		                  write, user, exec, accessed, dirty);
		if (err)
			break;
		cur += psize;
	}

	physical_tmp_unmap(pml4);
	return err;
}

static int page_unmap_at(union pml4e * pml4, void * vaddr, size_t * size) {
	int err;
	union linear_addr * laddr = (void *) &vaddr;
	union pdpte * pdpt = NULL;
	union pde * pd = NULL;
	union pte * pt = NULL;

	*size = PAGE_SIZE_PML4;
	union pml4e * pml4e = &pml4[laddr->pdpt.pml4];
	if (!pml4e->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	pdpt = physical_tmp_map(get_pdpt(pml4e));
	if (pdpt == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PDPT;
	union pdpte * pdpte = &pdpt[laddr->pdpt.pdpt];
	if (!pdpte->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	if (pdpte->page.ps) {
		err = page_unmap_pdpt(pdpte);
		goto cleanup;
	}
	pd = physical_tmp_map(get_pd(pdpte));
	if (pd == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PD;
	union pde * pde = &pd[laddr->pd.pd];
	if (!pde->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	if (pde->page.ps) {
		err = page_unmap_pd(pde);
		goto cleanup;
	}
	pt = physical_tmp_map(get_pt(pde));
	if(pt == NULL) {
		err = -EINVAL;
		goto cleanup;
	}

	*size = PAGE_SIZE_PT;
	union pte * pte = &pt[laddr->pt.pt];
	if (!pte->absent.p) {
		err = -ENOENT;
		goto cleanup;
	}
	err = page_unmap_pt(pte);
	goto cleanup;

cleanup:
	if (pdpt != NULL)
		physical_tmp_unmap(pdpt);
	if (pd != NULL)
		physical_tmp_unmap(pd);
	if (pt != NULL)
		physical_tmp_unmap(pt);
	return err;
}

int page_unmap(pid_t pid, void * vaddr, size_t size) {
	union cr3 * cr3 = get_cr3(pid);
	if (cr3 == NULL)
		return -EINVAL;

	union pml4e * pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		return 0;

	size_t cur = 0;
	uint8_t * vaddr8 = vaddr;
	while (cur < size) {
		size_t psize;
		page_unmap_at(pml4, vaddr8 + cur, &psize);
		cur += psize;
	}

	physical_tmp_unmap(pml4);
	return 0;
}

void * page_find_free(pid_t pid, size_t size, size_t align, void * start) {
	void * ret = NULL;
	union pml4e * pml4 = NULL;
	union pdpte * pdpt = NULL;
	union pde * pd = NULL;
	union pte * pt = NULL;

	size_t current_psize = target_page_size(size);
	void * _start = pid ? USER_SPACE_START : KERNEL_SPACE_START;
	uint8_t * real_start = start < _start ? _start : start;
	real_start = aligned_up(aligned_up(real_start, align), current_psize);
	bool kmem = is_kmem(real_start, size);

	bool is_first_iter = true;
	union linear_addr * laddr = (void *) &real_start;

	union cr3 * cr3 = get_cr3(pid);
	if (cr3 == NULL)
		goto cleanup;

	pml4 = physical_tmp_map(get_pml4(cr3));
	if (pml4 == NULL)
		goto cleanup;

	size_t cur_free = 0;
	uint64_t cur_addr = 0;
	for (size_t pml4_i = is_first_iter ? laddr->pt.pml4 : 0;
	     pml4_i < PAGE_TABLE_ELEMENTS; pml4_i++) {
		if (!pml4[pml4_i].absent.p) {
			cur_free += PAGE_SIZE_PML4;
			if (cur_free >= size) {
				ret = (void *) cur_addr;
				goto cleanup;
			}
			continue;
		}

		if (pdpt != NULL)
			physical_tmp_unmap(pdpt);
		pdpt = physical_tmp_map(get_pdpt(&pml4[pml4_i]));
		if (pdpt == NULL) {
			// no pdpt
			cur_free = 0;
			cur_addr = (pml4_i+1) * PAGE_SIZE_PML4;
			continue;
		}

		for (size_t pdpt_i = is_first_iter ? laddr->pt.pdpt : 0;
		     pdpt_i < PAGE_TABLE_ELEMENTS; pdpt_i++) {
			if (!pdpt[pdpt_i].absent.p) {
				cur_free += PAGE_SIZE_PDPT;
				if (cur_free >= size) {
					ret = (void *) cur_addr;
					goto cleanup;
				}
				continue;
			}

			if (pd != NULL)
				physical_tmp_unmap(pd);
			pd = physical_tmp_map(get_pd(&pdpt[pdpt_i]));
			if (target_page_size(size - cur_free)
			    == PAGE_SIZE_PDPT || pd == NULL) {
				// no pd
				cur_free = 0;
				cur_addr = pml4_i * PAGE_SIZE_PML4
					+ (pdpt_i+1) * PAGE_SIZE_PDPT;
				continue;
			}

			for (size_t pd_i = is_first_iter ? laddr->pt.pd : 0;
			     pd_i < PAGE_TABLE_ELEMENTS; pd_i++) {
				if (!pd[pd_i].absent.p) {
					cur_free += PAGE_SIZE_PD;
					if (cur_free >= size) {
						ret = (void *) cur_addr;
						goto cleanup;
					}
					continue;
				}

				if (pt != NULL)
					physical_tmp_unmap(pt);
				pt = physical_tmp_map(get_pt(&pd[pd_i]));
				if (target_page_size(size - cur_free)
				    == PAGE_SIZE_PD || pt == NULL) {
					// no pt
					cur_free = 0;
					cur_addr = pml4_i * PAGE_SIZE_PML4
						+ pdpt_i * PAGE_SIZE_PDPT
						+ (pd_i+1) * PAGE_SIZE_PD;
					continue;
				}

				for (size_t pt_i =
				     is_first_iter ? laddr->pt.pt : 0;
				     pt_i < PAGE_TABLE_ELEMENTS; pt_i++) {
					if (!pt[pt_i].absent.p) {
						cur_free += PAGE_SIZE_PT;
						if (cur_free >= size) {
							ret = (void *) cur_addr;
							goto cleanup;
						}
						continue;
					}
					cur_free = 0;
					cur_addr = pml4_i * PAGE_SIZE_PML4
						+ pdpt_i * PAGE_SIZE_PDPT
						+ pd_i * PAGE_SIZE_PD
						+ (pt_i+1) * PAGE_SIZE_PT;
				}
				is_first_iter = false;
			}
			is_first_iter = false;
		}
		is_first_iter = false;
	}
	is_first_iter = false;

cleanup:
	if (pdpt != NULL)
		physical_tmp_unmap(pdpt);
	if (pd != NULL)
		physical_tmp_unmap(pd);
	if (pt != NULL)
		physical_tmp_unmap(pt);
	return kmem ? to_kmem(ret) : ret;
}
