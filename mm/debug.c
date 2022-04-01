#define pr_fmt(fmt) "mm: " fmt

#include "pmm.h"

#include <stdint.h>

#include <mm.h>
#include <print.h>

static void print_memmap_desc(struct memmap_desc * desc)
{
	const char * type;
	switch (desc->type) {
	case MEMORY_TYPE_RESERVED:
		type = "MEMORY_TYPE_RESERVED";
		break;
	case MEMORY_TYPE_KERNEL_CODE:
		type = "MEMORY_TYPE_KERNEL_CODE";
		break;
	case MEMORY_TYPE_KERNEL_DATA:
		type = "MEMORY_TYPE_KERNEL_DATA";
		break;
	case MEMORY_TYPE_EFI_SERVICES:
		type = "MEMORY_TYPE_EFI_SERVICES";
		break;
	case MEMORY_TYPE_AVAILABLE:
		type = "MEMORY_TYPE_AVAILABLE";
		break;
	case MEMORY_TYPE_BAD:
		type = "MEMORY_TYPE_BAD";
		break;
	case MEMORY_TYPE_ACPI_RECLAIMABLE:
		type = "MEMORY_TYPE_ACPI_RECLAIMABLE";
		break;
	case MEMORY_TYPE_ACPI_NVS:
		type = "MEMORY_TYPE_ACPI_NVS";
		break;
	case MEMORY_TYPE_HARDWARE:
		type = "MEMORY_TYPE_HARDWARE";
		break;
	case MEMORY_TYPE_PERSISTENT:
		type = "MEMORY_TYPE_PERSISTENT";
		break;
	default:
		pr_err("Unknown descriptor type: %d\n", desc->type);
		return;
	}
	pr_debug("descriptor: [%p,%p] -> %p (%zu): %s\n",
	         desc->phy_start,
	         (uint8_t *) desc->phy_start + desc->size,
	         desc->virt_start,
		 desc->size,
	         type);
}

void print_memmap(struct memmap map)
{
	for (size_t i = 0; i < map.desc_count; i++)
		print_memmap_desc(&map.desc[i]);
}
