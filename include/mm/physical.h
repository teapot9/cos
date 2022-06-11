#ifndef _MM_PHYSICAL_H
#define _MM_PHYSICAL_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Temporary map physical memory
 * @param paddr Physical address
 * @return Usable virtual address
 */
void * physical_tmp_map(void * paddr);

/**
 * @brief Free temporary mapping
 * @param vaddr Virtual address
 */
void physical_tmp_unmap(void * vaddr);

#ifdef __cplusplus
}
#endif
#endif // MM_PHYSICAL_H
