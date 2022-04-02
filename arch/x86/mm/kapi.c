#include <mm.h>

#include <errno.h>

#include <print.h>
#include <mm/paging.h>
#include "paging.h"
#include <kconfig.h>

static inline size_t sanitize_size(size_t size)
{
	return (size / PAGE_SIZE_PT + (size % PAGE_SIZE_PT != 0))
		* PAGE_SIZE_PT;
}

int vmap(pid_t pid, void * paddr, void * vaddr, size_t * size) {
	*size = sanitize_size(*size);
	if (is_kmem(vaddr, *size) && pid)
		return -EACCES;

	int err = page_map(pid, vaddr, size, paddr);
	if (err) {
		pr_err("vmap: could not map virtual %p -> physical %p "
		       "[%zu bytes], errno = %d\n", vaddr, paddr, size, err);
	}

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("vmap: mapped virtual %p -> physical %p [%zu bytes]\n",
	         vaddr, paddr, size);
#endif
	return err;
}

int vreset(pid_t pid, void * vaddr, size_t size, bool write, bool user, bool exec) {
	size = sanitize_size(size);
	if (is_kmem(vaddr, size) && pid)
		return -EACCES;

	int err = page_set(pid, vaddr, size, write, user, exec, false, false);
	if (err) {
		pr_err("vmap: could not reset virtual %p [%c%c%c] "
		       "[%zu bytes], errno = %d\n", vaddr, write ? 'W' : ' ',
		       user ? 'U' : ' ', exec ? 'X' : ' ', size, err);
	}

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("vmap: reseted virtual %p [%c%c%c] [%zu bytes]\n",
	         vaddr, write ? 'W' : ' ', user ? 'U' : ' ',
	         exec ? 'X' : ' ', size);
#endif
	return err;
}

int vunmap(pid_t pid, void * vaddr, size_t size) {
	size = sanitize_size(size);
	if (is_kmem(vaddr, size) && pid)
		return -EACCES;

	int err = page_unmap(pid, vaddr, size);
	if (err) {
		pr_err("vunmap: could not unmap virtual %p [%zu bytes], "
		       "errno = %d\n", vaddr, size, err);
	}

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("vmap: unmapped virtual %p [%zu bytes]\n",
	         vaddr, size);
#endif
	return err;
}

void * mmap(pid_t pid, void * paddr, size_t * size, size_t valign,
	    bool write, bool user, bool exec) {
	*size = sanitize_size(*size);
	void * vaddr = page_find_free(pid, *size, valign, 0);
	if (vaddr == NULL) {
		pr_err("mmap: could not find %zu bytes (%zu aligned) "
		       "of virtual memory\n", size, valign);
		return NULL;
	}

	int err = page_map(pid, vaddr, size, paddr);
	if (err) {
		pr_err("mmap: could not map %zu bytes (%zu aligned) "
		       "of memory to %p\n", size, valign, paddr);
		return NULL;
	}

	err = page_set(pid, vaddr, *size, write, user, exec, false, false);
	if (err) {
		pr_err("mmap: could not configure %zu bytes (%zu aligned) "
		       "of memory at %p\n", size, valign, vaddr);
		return NULL;
	}

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("mmap: mapped physical %p <- virtual %p [%zu bytes] "
	         "[%c%c%c]\n", paddr, vaddr, size, write ? 'W' : ' ',
	         user ? 'U' : ' ', exec ? 'X' : ' ');
#endif
	return vaddr;
}

void * valloc(pid_t pid, size_t * size, size_t palign, size_t valign,
              bool write, bool user, bool exec) {
	*size = sanitize_size(*size);
	size_t target_psize = target_page_size(*size);
	palign = target_page_size(palign);
	valign = target_page_size(valign);
	palign = palign > target_psize ? palign : target_psize;
	valign = valign > target_psize ? valign : target_psize;

	void * paddr = palloc(pid, *size, palign);
	if (paddr == NULL) {
		pr_err("valloc: could not allocate %zu (%zu aligned) "
		       "of physical memory\n", size, palign);
		return NULL;
	}

	void * vaddr = mmap(pid, paddr, size, valign, write, user, exec);
	if (vaddr == NULL) {
		pr_err("valloc: could not map allocated physical memory at "
		       "%p [%zu bytes] (%zu aligned)\n", paddr, size, valign);
		return NULL;
	}

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("valloc: allocated %zu bytes of memory: virtual %p -> "
	         "physical %p [%c%c%c]\n", size, vaddr, paddr,
		 write ? 'W' : ' ', user ? 'U' : ' ', exec ? 'X' : ' ');
#endif
	return vaddr;
}

int vfree(pid_t pid, void * vaddr, size_t size) {
	size = sanitize_size(size);
	if (is_kmem(vaddr, size) && pid)
		return -EACCES;

	void * paddr = virt_to_phys(pid, vaddr);
	if (paddr == NULL) {
		pr_err("vfree: no virtual memory mapped at %p\n", vaddr);
		return -ENOENT;
	}

	int err = vunmap(pid, vaddr, size);
	if (err) {
		pr_err("vfree: could not unmap %zu bytes of memory at %p, "
		       "errno = %d\n", vaddr, size, err);
	}

	punmap(pid, paddr, size);

#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("vfree: unmapped virtual memory at %p, freed physical memory "
		 "at %p [%zu bytes]\n", vaddr, paddr, size);
#endif
	return 0;
}
