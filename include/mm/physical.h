#ifndef _MM_PHYSICAL_H
#define _MM_PHYSICAL_H
#ifdef __cplusplus
extern "C" {
#endif

void * physical_tmp_map(void * paddr);
void physical_tmp_unmap(void * vaddr);

#ifdef __cplusplus
}
#endif
#endif // MM_PHYSICAL_H
