#define pr_fmt(fmt) "elf: " fmt
#include <elf.h>
#include "elf.h"

#include <stddef.h>
#include <errno.h>
#include <stdbool.h>

#include <mm/pmm.h>
#include <mm/vmm.h>
#include <alloc.h>
#include <print.h>
#include <string.h>
#include <mm/physical.h>
#include <mm/paging.h>

#define array_foreach_s(iterator, start, ent_size, nb_ent) \
	for ((iterator) = (void *) (start); \
	     (void *) (iterator) < (void *) ((uint8_t *) (start) \
	                            + (ent_size) * (nb_ent)); \
	     (iterator) = (void *) ((uint8_t *) (iterator) + (ent_size)))

static void phdr_flags_str(char dst[], Elf64_Word flags)
{
	strcpy(dst, "   ");
	if (flags & PF_R)
		dst[0] = 'R';
	if (flags & PF_W)
		dst[1] = 'W';
	if (flags & PF_X)
		dst[2] = 'X';
}

static const char * phdr_type_str(Elf64_Word type)
{
	switch (type) {
	case PT_LOAD:
		return "LOAD";
	case PT_DYNAMIC:
		return "DYNAMIC";
	case PT_INTERP:
		return "INTERP";
	case PT_NOTE:
		return "NOTE";
	case PT_SHLIB:
		return "SHLIB";
	case PT_PHDR:
		return "PHDR";
	default:
		return "UNDEFINED";
	}
}

static bool check_magic(struct e_ident * ident)
{
	char magic[] = EI_MAG;
	return strncmp((const char *) ident->ei_mag, magic, 4) == 0;
}

int elf64_check(void * start, size_t size) {
	struct e_ident * ident = start;

	if (!check_magic(ident))
		return -EINVAL;

	switch (ident->ei_class) {
	case ELFCLASS32:
		pr_err("32 bit ELF is not supported\n", 0);
		return -ENOTSUP;
	case ELFCLASS64:
		break;
	default:
		return -EINVAL;
	}

	switch (ident->ei_data) {
	case ELFDATA2LSB:
		break;
	case ELFDATA2MSB:
		pr_err("only little-endian ELF are supported\n", 0);
		return -ENOTSUP;
	default:
		return -EINVAL;
	}

	if (ident->ei_version != EV_CURRENT)
		return -ENOTSUP;

	if (ident->ei_osabi != ELFOSABI_SYSV)
		return -ENOTSUP;
	if (ident->ei_abiversion != 0)
		return -EINVAL;

	Elf64_Ehdr * hdr = start;

	if (hdr->e_ehsize < sizeof(*hdr))
		return -EINVAL;

	if (hdr->e_type != ET_EXEC)
		return -ENOTSUP;

	if (hdr->e_machine != EM_X86_64)
		return -ENOTSUP;

	if (hdr->e_version != EV_CURRENT)
		return -ENOTSUP;

	if (hdr->e_entry == 0)
		return -EINVAL;

	if (hdr->e_phoff == 0 || hdr->e_phnum == 0)
		return -EINVAL;

	return 0;
}

#define pr_debug_header(prefix, phdr) do { \
	char flags[4]; \
	phdr_flags_str(flags, phdr->p_flags); \
	pr_debug("%s%s: %p[0x%lx] -> %p[0x%lx] %s (align 0x%lx)\n", prefix, \
	         phdr_type_str(phdr->p_type), phdr->p_offset, phdr->p_filesz, \
		 phdr->p_vaddr, phdr->p_memsz, flags, phdr->p_align); \
} while (0)

int elf64_mem_setup(pid_t pid, void * start, size_t size) {
	int err = elf64_check(start, size);
	if (err)
		return err;

	Elf64_Ehdr * hdr = start;
	Elf64_Phdr * phdr;
	array_foreach_s(phdr, (uint8_t *) start + hdr->e_phoff,
	                hdr->e_phentsize, hdr->e_phnum) {
		pr_debug_header("elf64_mem_setup: ", phdr);

		switch (phdr->p_type) {
		case PT_LOAD:;
			/* Allocate */
			if (phdr->p_memsz == 0 && phdr->p_filesz == 0)
				break;
			void * paddr = palloc(0, phdr->p_memsz, phdr->p_align);
			if (paddr == NULL)
				return -ENOMEM;
			size_t memsz = phdr->p_memsz;
			int err = vmap(0, paddr, phdr->p_vaddr, &memsz);
			if (err) {
				punmap(0, paddr, phdr->p_memsz);
				return err;
			}
			break;
		case PT_DYNAMIC:
		case PT_NOTE:
			break;
		case PT_INTERP:
		case PT_SHLIB:
		case PT_PHDR:
			return -ENOTSUP;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

int elf64_load_data(pid_t pid, void * start, size_t size) {
	int err = elf64_check(start, size);
	if (err)
		return err;

	Elf64_Ehdr * hdr = start;
	Elf64_Phdr * phdr;
	array_foreach_s(phdr, (uint8_t *) start + hdr->e_phoff,
	                hdr->e_phentsize, hdr->e_phnum) {
		pr_debug_header("elf64_load_data: ", phdr);

		switch (phdr->p_type) {
		case PT_LOAD:;
			/* Copy data */
			void * paddr = virt_to_phys(pid, phdr->p_vaddr);
			void * vaddr = physical_tmp_map(paddr);
			memcpy(vaddr,
			       (uint8_t *) start + phdr->p_offset,
			       phdr->p_filesz);
			void * zero_start = (void *)
				((uintptr_t) vaddr + phdr->p_filesz);
			memset(zero_start, 0, phdr->p_memsz - phdr->p_filesz);
			physical_tmp_unmap(vaddr);
			break;
		case PT_DYNAMIC:
		case PT_NOTE:
			break;
		case PT_INTERP:
		case PT_SHLIB:
		case PT_PHDR:
			return -ENOTSUP;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

int elf64_mem_finalize(pid_t pid, void * start, size_t size) {
	int err = elf64_check(start, size);
	if (err)
		return err;

	Elf64_Ehdr * hdr = start;
	Elf64_Phdr * phdr;
	array_foreach_s(phdr, (uint8_t *) start + hdr->e_phoff,
	                hdr->e_phentsize, hdr->e_phnum) {
		pr_debug_header("elf64_mem_finalize: ", phdr);

		switch (phdr->p_type) {
		case PT_LOAD:;
			/* Set permissions */
			if (phdr->p_memsz == 0)
				break;
			bool exec = phdr->p_flags & PF_X;
			bool user = pid != 0;
			bool write = phdr->p_flags & PF_W;
			err = vreset(pid, phdr->p_vaddr, phdr->p_memsz,
			             write, user, exec);
			if (err)
				return err;
			break;
		case PT_DYNAMIC:
		case PT_NOTE:
			break;
		case PT_INTERP:
		case PT_SHLIB:
		case PT_PHDR:
			return -ENOTSUP;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

int elf64_load(pid_t pid, void (** entry)(void), void * start, size_t size)
{
	int err = elf64_check(start, size);
	if (err)
		return err;
	if (entry == NULL)
		return -EINVAL;

	if ((err = elf64_mem_setup(pid, start, size)))
		return err;
	if ((err = elf64_load_data(pid, start, size)))
		return err;
	if ((err = elf64_mem_finalize(pid, start, size)))
		return err;

	Elf64_Ehdr * hdr = start;
	*entry = (void (*)(void)) hdr->e_entry;

	return 0;
}
