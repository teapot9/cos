#define pr_fmt(fmt) "efistub: " fmt

#include <firmware/efistub.h>
#include "efistub.h"

#include <errno.h>

#include <setup.h>
#include <device.h>
#include <print.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/boot.h>
#include <alloc.h>
#include <mm/memmap.h>
#include <mm/pmm.h>
#include <kconfig.h>

#if IS_ENABLED(CONFIG_MM_DEBUG)
static void print_efi_mem_desc(efi_memory_descriptor_t * desc)
{
	const char * desc_type_str;
	switch (desc->type) {
	case EFI_RESERVED_MEMORY_TYPE:
		desc_type_str = "EFI_RESERVED_MEMORY_TYPE";
		break;
	case EFI_LOADER_CODE:
		desc_type_str = "EFI_LOADER_CODE";
		break;
	case EFI_LOADER_DATA:
		desc_type_str = "EFI_LOADER_DATA";
		break;
	case EFI_BOOT_SERVICES_CODE:
		desc_type_str = "EFI_BOOT_SERVICES_CODE";
		break;
	case EFI_BOOT_SERVICES_DATA:
		desc_type_str = "EFI_BOOT_SERVICES_DATA";
		break;
	case EFI_RUNTIME_SERVICES_CODE:
		desc_type_str = "EFI_RUNTIME_SERVICES_CODE";
		break;
	case EFI_RUNTIME_SERVICES_DATA:
		desc_type_str = "EFI_RUNTIME_SERVICES_DATA";
		break;
	case EFI_CONVENTIONAL_MEMORY:
		desc_type_str = "EFI_CONVENTIONAL_MEMORY";
		break;
	case EFI_UNUSABLE_MEMORY:
		desc_type_str = "EFI_UNUSABLE_MEMORY";
		break;
	case EFI_ACPIRECLAIM_MEMORY:
		desc_type_str = "EFI_ACPIRECLAIM_MEMORY";
		break;
	case EFI_ACPIMEMORY_NVS:
		desc_type_str = "EFI_ACPIMEMORY_NVS";
		break;
	case EFI_MEMORY_MAPPED_IO:
		desc_type_str = "EFI_MEMORY_MAPPED_IO";
		break;
	case EFI_MEMORY_MAPPED_IOPORT_SPACE:
		desc_type_str = "EFI_MEMORY_MAPPED_IOPORT_SPACE";
		break;
	case EFI_PAL_CODE:
		desc_type_str = "EFI_PAL_CODE";
		break;
	case EFI_PERSISTENT_MEMORY:
		desc_type_str = "EFI_PERSISTENT_MEMORY";
		break;
	case EFI_MAX_MEMORY_TYPE:
		desc_type_str = "EFI_MAX_MEMORY_TYPE";
		break;
	default:
		desc_type_str = "(unknown)";
		break;
	}
	pr_debug("Descriptor: %s, phy=%p, virt=%p, pages=%d\n",
		 desc_type_str, desc->physical_start, desc->virtual_start,
		 desc->number_of_pages);
}
#else // CONFIG_MM_DEBUG
#define print_efi_mem_desc(...)
#endif // !CONFIG_MM_DEBUG


#if 0
static enum memory_type get_mem_type(uint32_t efi_type)
{
	switch (efi_type) {
	default:
	case EFI_RESERVED_MEMORY_TYPE:
		return MEMORY_TYPE_RESERVED;
	case EFI_LOADER_CODE:
	case EFI_BOOT_SERVICES_CODE: // debug
		return MEMORY_TYPE_KERNEL_CODE;
	case EFI_LOADER_DATA:
	case EFI_BOOT_SERVICES_DATA: // debug
		return MEMORY_TYPE_KERNEL_DATA;
	case EFI_RUNTIME_SERVICES_CODE:
	case EFI_RUNTIME_SERVICES_DATA:
		return MEMORY_TYPE_EFI_SERVICES;
	case EFI_CONVENTIONAL_MEMORY:
		return MEMORY_TYPE_AVAILABLE;
	case EFI_UNUSABLE_MEMORY:
		return MEMORY_TYPE_BAD;
	case EFI_ACPIRECLAIM_MEMORY:
		return MEMORY_TYPE_ACPI_RECLAIMABLE;
	case EFI_ACPIMEMORY_NVS:
		return MEMORY_TYPE_ACPI_NVS;
	case EFI_MEMORY_MAPPED_IO:
	case EFI_MEMORY_MAPPED_IOPORT_SPACE:
	case EFI_PAL_CODE:
		return MEMORY_TYPE_HARDWARE;
	case EFI_PERSISTENT_MEMORY:
		return MEMORY_TYPE_PERSISTENT;
	}
}

static int register_memmap_desc(uint32_t efi_type, void * paddr,
                                void * vaddr, size_t size)
{
	int err;
	switch (efi_type) {
	case EFI_LOADER_CODE:
	case EFI_LOADER_DATA:
	case EFI_BOOT_SERVICES_CODE: // debug
	case EFI_BOOT_SERVICES_DATA: // debug
	case EFI_RUNTIME_SERVICES_CODE:
	case EFI_RUNTIME_SERVICES_DATA:
		err = register_used_pmem(paddr, size);
		if (err)
			return err;
		err = vmap(paddr, vaddr, size);
		if (err)
			return err;
		break;
	case EFI_CONVENTIONAL_MEMORY:
		err = register_free_pmem(paddr, size);
		if (err)
			return err;
		break;
	case EFI_ACPIRECLAIM_MEMORY:
		err = pmm_acpi_register(paddr, size);
		if (err)
			return err;
		break;
	case EFI_RESERVED_MEMORY_TYPE:
	case EFI_PERSISTENT_MEMORY:
	case EFI_UNUSABLE_MEMORY:
	case EFI_ACPIMEMORY_NVS:
	case EFI_MEMORY_MAPPED_IO:
	case EFI_MEMORY_MAPPED_IOPORT_SPACE:
	case EFI_PAL_CODE:
	default:
		;
	}
	return 0;
}

static int register_memmap(
	efi_memory_descriptor_t * efi_memmap,
	efi_uintn desc_size, efi_uintn map_size
)
{
	/*
	map->desc_count = map_size / desc_size;
	map->desc = malloc((map->desc_count) * sizeof(*map->desc));
	if (map->desc == NULL) {
		map->desc_count = 0;
		return -ENOMEM;
	}
	size_t d = 0;
	*/
	int err;

	for (size_t i = 0; i < map_size / desc_size; i++) {
		efi_memory_descriptor_t * desc = (void *)
			((uint8_t *) efi_memmap + i * desc_size);
		print_efi_mem_desc(desc);

		err = register_memmap_desc(
			desc->type,
			(void *) desc->physical_start,
			(void *) (desc->virtual_start + desc->physical_start),
			desc->number_of_pages * 4096
		);
		if (err) {
			pr_err("Failed to register memory map descriptor, "
			       "errno = %d\n", err);
			continue;
		}
#if 0
		if (d && get_mem_type(desc->type) == map->desc[d - 1].type) {
			// Merge descriptor
			d--;
			map->desc[d].size += desc->number_of_pages * 4096;
		} else {
			// New descriptor
			map->desc[d].type = get_mem_type(desc->type);
			map->desc[d].phy_start = (void *) desc->physical_start;
			map->desc[d].virt_start = (void *) desc->virtual_start;
			map->desc[d].size = desc->number_of_pages * 4096;
		}
		d++;
#endif
	}

#if 0
	struct memmap_desc * tmp = krealloc(map->desc, d * sizeof(*map->desc));
	if (tmp == NULL)
		pr_err("Cannot reduce memmap buffer size\n", 0);
	else
		map->desc = tmp;
	map->desc_count = d;
#endif
	return 0;
}
#endif

/* Return 0: no vmap, 1: code, 2: data */
static int get_memtype(enum memory_type * dst, uint32_t type)
{
	switch (type) {
	case EFI_RESERVED_MEMORY_TYPE:
	case EFI_MEMORY_MAPPED_IO:
	case EFI_MEMORY_MAPPED_IOPORT_SPACE:
		*dst = MEMORY_TYPE_RESERVED;
		return 0;
	case EFI_RUNTIME_SERVICES_CODE:
		*dst = MEMORY_TYPE_EFI_SERVICES;
		return 1;
	case EFI_RUNTIME_SERVICES_DATA:
		*dst = MEMORY_TYPE_EFI_SERVICES;
		return 2;
	case EFI_CONVENTIONAL_MEMORY:
		*dst = MEMORY_TYPE_FREE;
		return 0;
	case EFI_ACPIRECLAIM_MEMORY:
		*dst = MEMORY_TYPE_ACPI_RECLAIMABLE;
		return 0;
	case EFI_ACPIMEMORY_NVS:
		*dst = MEMORY_TYPE_ACPI_NVS;
		return 0;
	case EFI_PERSISTENT_MEMORY:
		*dst = MEMORY_TYPE_PERSISTENT;
		return 0;
	case EFI_LOADER_CODE:
	case EFI_BOOT_SERVICES_CODE:
		*dst = MEMORY_TYPE_USED;
		return 1; // in-use, free when loading kernel elf
	case EFI_LOADER_DATA:
	case EFI_BOOT_SERVICES_DATA:
		*dst = MEMORY_TYPE_USED;
		return 2; // in-use, free when loading kernel elf
	case EFI_UNUSABLE_MEMORY:
	case EFI_PAL_CODE:
	default:
		return -ENOENT;
	}
}

static inline void * memstart(efi_memory_descriptor_t * desc)
{
	return (void *) desc->physical_start;
}

static inline size_t memsize(efi_memory_descriptor_t * desc)
{
	return desc->number_of_pages * 4096;
}

static int register_efi_memmap_desc(
	struct memmap * map, efi_memory_descriptor_t * desc
)
{
	enum memory_type type;
	int typeinfo = get_memtype(&type, desc->type);
	if (typeinfo == -ENOENT)
		return 0;
	int err = memmap_update(map, memstart(desc),
	                        memsize(desc), type, 0);
	if (err)
		return err;
#if IS_ENABLED(CONFIG_MM_DEBUG)
	pr_debug("register memmap [%p; %p] (%zu bytes): %s\n",
		 memstart(desc), (uintptr_t) memstart(desc) + memsize(desc),
		 memsize(desc), memmap_strtype(type));
#endif
	return typeinfo;
}

static void register_efi_memmap(
	struct memmap * map, efi_memory_descriptor_t * efimap,
	size_t desc_size, size_t map_size
)
{
	for (size_t i = 0; i < map_size / desc_size; i++) {
		efi_memory_descriptor_t * desc = (void *)
			((uint8_t *) efimap + i * desc_size);
		print_efi_mem_desc(desc);

		int merr = register_efi_memmap_desc(map, desc);
#if 0
		if (merr == 1 || merr == 2) {
			struct page_perms perms = {
				.user = false,
				.exec = merr == 1,
				.write = merr == 2,
			};
			merr = pmap(0, memstart(desc), memsize(desc));
			if (!merr)
				merr = vmap(0, memstart(desc), memstart(desc),
				            memsize(desc), perms);
		}
#endif
		if (merr < 0) {
			pr_crit("cannot register physical memory at "
			        "%p (%zu bytes), errno = %d\n",
			        memstart(desc), memsize(desc), merr);
		}
	}
}

/* firmware/efistub.h */
int efistub_memmap_and_exit(struct memmap * map)
{
	efi_memory_descriptor_t * efi_memmap = NULL;
	efi_uintn map_size = 0;
	efi_status_t status;
	efi_uintn map_key;
	efi_uintn desc_size;
	uint32_t desc_version;

	if (!efistub_is_init())
		return -EINVAL;

	pr_debug("Getting memory map and exiting boot services\n", 0);

	/* Get memory map buffer size */
	status = efistub_system_table()->boot_services->get_memory_map(
		&map_size, efi_memmap,
		&map_key, &desc_size, &desc_version
	);
	if (status != EFI_BUFFER_TOO_SMALL) {
		pr_err("Cannot get memory map: expected EFI_BUFFER_TOO_SMALL, "
		       "got %d\n", status);
		return -ENOTSUP;
	}
	efi_memmap = malloc(map_size);
	if (efi_memmap == NULL) {
		pr_err("Cannot allocate %d bytes for memory map\n", map_size);
		return -ENOMEM;
	}

	/* Get memory map */
	status = efistub_system_table()->boot_services->get_memory_map(
		&map_size, efi_memmap, &map_key, &desc_size, &desc_version
	);
	if (status != EFI_SUCCESS) {
		kfree(efi_memmap);
		pr_err("Cannot get memory map (error %d)\n", status);
		return -ENOTSUP;
	}
	if (desc_version != 1) {
		kfree(efi_memmap);
		pr_err("Unknown memory descriptor version: %d\n", desc_version);
		return -ENOTSUP;
	}

	/* Exit boot services */
	status = efistub_system_table()->boot_services->exit_boot_services(
		efistub_image_handle(), map_key
	);
	if (status != EFI_SUCCESS) {
		kfree(efi_memmap);
		pr_err("Cannot exit boot services (error %d)\n", status);
		return -ENOTSUP;
	}
	device_delete(efiboot_dev);

#if IS_ENABLED(CONFIG_SERIAL_EARLY_DEBUG)
	/* Start serial */
	serial_init();
#endif

	/* Convert memory map */
	register_efi_memmap(map, efi_memmap, desc_size, map_size);
	kfree(efi_memmap);
	return 0;
}
