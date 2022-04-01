#define pr_fmt(fmt) "elf: " fmt
#include <elf.h>
#include "elf.h"

#include <stddef.h>
#include <errno.h>
#include <stdbool.h>

#include <mm.h>
#include <print.h>
#include <string.h>
#include <mm/vmemmap.h>

#ifdef BOOTLOADER
/* public: setup.h */
struct vmemmap kvmemmap;
#endif // BOOTLOADER

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

static int elf_load64(void (** entry)(void), void * start, size_t size)
{
	Elf64_Ehdr * hdr = start;
#ifdef BOOTLOADER
	kvmemmap = vmemmap_new();
#endif // BOOTLOADER

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

	Elf64_Phdr * phdr;
	array_foreach_s(phdr, (uint8_t *) start + hdr->e_phoff,
	                hdr->e_phentsize, hdr->e_phnum) {
		char flags[4];
		phdr_flags_str(flags, phdr->p_flags);
		pr_debug("%s: %p[0x%lx] -> %p[0x%lx] %s (align 0x%lx)\n",
		         phdr_type_str(phdr->p_type), phdr->p_offset,
		         phdr->p_filesz, phdr->p_vaddr, phdr->p_memsz,
		         flags, phdr->p_align);

		switch (phdr->p_type) {
		case PT_LOAD:;
#if 0
			size_t memsz = (phdr->p_memsz / 4096) * 4096;
			if (phdr->p_memsz > memsz)
				memsz += 4096;
#endif
			/* Set permissions */
			struct page_perms perms = {
				.exec = phdr->p_flags & PF_X,
				.user = (0 != 0),
				.write = phdr->p_flags & PF_W,
			};
			struct page_perms temp_perms = {
				.exec = false, .user = false, .write = true
			};

			/* Allocate */
			void * paddr = palloc(0, phdr->p_memsz, phdr->p_align);
			if (paddr == NULL)
				return -ENOMEM;
			int err = vmap(0, paddr, phdr->p_vaddr,
			               phdr->p_memsz, temp_perms);
			if (err) {
				punmap(0, paddr, phdr->p_memsz);
				return err;
			}

#ifdef BOOTLOADER
			/* Save kernel vmemmap */
			err = vmemmap_map(&kvmemmap, phdr->p_vaddr,
			                  phdr->p_memsz, paddr, perms, false);
			if (err) {
				vfree(0, phdr->p_vaddr, phdr->p_memsz);
				return err;
			}
#endif // BOOTLOADER

			/* Copy data */
			memcpy(phdr->p_vaddr,
			       (uint8_t *) start + phdr->p_offset,
			       phdr->p_filesz);
			memset((uint8_t *) phdr->p_vaddr + phdr->p_filesz, 0,
			       phdr->p_memsz - phdr->p_filesz);

			/* Set permissions */
			// TODO: bug: this only set perms on one page, not whole range mapped with vmap ?
			vinit(0, phdr->p_vaddr, phdr->p_memsz, perms);
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

	*entry = (void (*)(void)) hdr->e_entry;

	return 0;
}

int elf_load(void (** entry)(void), void * start, size_t size)
{
	struct e_ident * ident = start;
	int (* arch_loader)(void (**)(void), void *, size_t);
	if (entry == NULL)
		return -EINVAL;

	if (!check_magic(ident))
		return -EINVAL;

	switch (ident->ei_class) {
	case ELFCLASS32:
		pr_err("32 bit ELF is not supported\n", 0);
		return -ENOTSUP;
	case ELFCLASS64:
		arch_loader = elf_load64;
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

	return arch_loader(entry, start, size);
}
