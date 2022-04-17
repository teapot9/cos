#ifndef ELF_H
#define ELF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <task.h>

int elf64_check(void * start, size_t size);
int elf64_mem_setup(pid_t pid, void * start, size_t size);
int elf64_load_data(pid_t pid, void * start, size_t size);
int elf64_mem_finalize(pid_t pid, void * start, size_t size);
int elf64_kernel_remap(void * start, size_t size);

int elf64_load(pid_t pid, void (** entry)(void), void * start, size_t size);

#ifdef __cplusplus
}
#endif
#endif // ELF_H
