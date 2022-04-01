#define pr_fmt(fmt) "efistub: " fmt

#include <firmware/efistub.h>
#include "efistub.h"

#include <errno.h>

#include <device.h>
#include <print.h>
#include <firmware/efiapi/efiapi.h>
#include <firmware/efiapi/system_table.h>
#include <firmware/efiapi/boot.h>
#include <mm.h>
#include <mm/early.h>

static __attribute__((unused)) void print_efi_mem_desc(
#ifndef CONFIG_MM_DEBUG
	UNUSED
#endif
	efi_memory_descriptor_t * desc
)
{
#ifdef CONFIG_MM_DEBUG
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
#endif
}

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
#endif

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
	map->desc = kmalloc((map->desc_count) * sizeof(*map->desc));
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

/* firmware/efistub.h */
int efistub_memmap_and_exit(void)
{
	efi_memory_descriptor_t * efi_memmap = NULL;
	efi_uintn map_size = 0;
	efi_status_t status;
	efi_uintn map_key;
	efi_uintn desc_size;
	uint32_t desc_version;
	int ret;

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
	efi_memmap = kmalloc(map_size);
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

	/* Convert memory map */
	ret = register_memmap(efi_memmap, desc_size, map_size);
	kfree(efi_memmap);
	return ret;
}
