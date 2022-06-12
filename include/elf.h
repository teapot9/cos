/**
 * @file elf.h
 * @brief ELF executable manipulation
 */

#ifndef ELF_H
#define ELF_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <task.h>

/**
 * @brief Check if ELF executable is valid
 * @param start Start of file
 * @param size Size of file
 * @return errno
 */
int elf64_check(void * start, size_t size);

/**
 * @brief Setup memory to execute ELF file
 * @param pid Process executing the file
 * @param start Start of ELF
 * @param size Size of ELF
 * @return errno
 */
int elf64_mem_setup(pid_t pid, void * start, size_t size);

/**
 * @brief Load ELF data into memory
 * @param pid Process executing the file
 * @param start Start of ELF
 * @param size Size of ELF
 * @return errno
 */
int elf64_load_data(pid_t pid, void * start, size_t size);

/**
 * @brief Configure memory for execution of ELF file
 * @param pid Process executing the file
 * @param start Start of ELF
 * @param size Size of ELF
 * @return errno
 */
int elf64_mem_finalize(pid_t pid, void * start, size_t size);

/**
 * @brief Load ELF file to be executed
 * @param pid Process executing the file
 * @param entry ELF entry point address
 * @param start Start of ELF
 * @param size Size of ELF
 * @return errno
 */
int elf64_load(pid_t pid, void (** entry)(void), void * start, size_t size);

#ifdef __cplusplus
}
#endif
#endif // ELF_H
