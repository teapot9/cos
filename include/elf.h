#ifndef ELF_H
#define ELF_H

#include <stddef.h>

int elf_load(void (** entry)(void), void * start, size_t size);

#endif // ELF_H
