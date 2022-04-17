#ifndef MM_PAGING_H
#define MM_PAGING_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <task.h>

void * virt_to_phys(pid_t pid, void * vaddr);
void * virt_to_phys_current(void * vaddr);

int page_map(pid_t pid, void * vaddr, size_t * size, void * paddr);
int page_set(pid_t pid, void * vaddr, size_t size, bool write, bool user,
             bool exec, bool accessed, bool dirty);
int page_unmap(pid_t pid, void * vaddr, size_t size);
void * page_find_free(pid_t pid, size_t size, size_t align, void * start);

#ifdef __cplusplus
}
#endif
#endif // MM_PAGING_H
