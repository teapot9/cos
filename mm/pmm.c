#include <mm.h>
#include "pmm.h"

#include <stdint.h>

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

static struct pmem_block * first_free = NULL;

static struct pmem_block * find_prev_free_block(void * ptr)
{
	struct pmem_block * prev = NULL;
	struct pmem_block * cur = first_free;

	while (cur != NULL && (uint8_t *) cur < (uint8_t *) ptr) {
		prev = cur;
		cur = cur->next;
	}
	return prev;
}

static void free_block(void * start, size_t size)
{
	struct pmem_block * prev = find_prev_free_block(start);
	struct pmem_block * cur = start;
	cur->size = size;
	if (prev != NULL) {
		cur->next = prev->next;
		prev->next = cur;
	} else {
		cur->next = NULL;
		first_free = cur;
	}
}

void pmm_init(struct memmap map)
{
	for (size_t i = 0; i < map.desc_count; i++) {
		switch (map.desc[i].type) {
		case MEMORY_TYPE_AVAILABLE:;
			void * start = map.desc[i].phy_start;
			size_t size = map.desc[i].size;
			if (start == NULL) {
				start = (uint8_t *) start + 4096;
				size += 4096;
			}
			free_block(start, size);
			break;
		default:;
		}
	}
}

void * pmalloc(size_t size, size_t align)
{
	struct pmem_block * cur = first_free;
	while (cur != NULL) {
		uint8_t * ptr = (uint8_t *) cur;
		size_t mod = (size_t) ptr % align;
		uint8_t * aligned = ptr - mod + (mod ? align : 0);
		size_t align_diff = aligned - ptr;
		if (cur->size - (align_diff) < size)
			continue;
		if (cur->size > size + align_diff) {
			// Create free block following the one we alloc
			struct pmem_block * new =
				(void *) (ptr + align_diff + size);
			new->size = cur->size - size - align_diff;
			new->next = cur->next;
			cur->next = new;
		}
		if (aligned - ptr) {
			// We still have free memory before the block we alloc
			cur->size = aligned - ptr;
		} else {
			// No free memory due to alignment: remove cur block
			struct pmem_block * prev =
				find_prev_free_block((void *) cur);
			if (prev != NULL)
				prev->next = cur->next;
			else
				first_free = cur->next;
		}
		pr_debug("Allocated %zu B at %p\n", size, aligned);
		return aligned;
	}
	pr_alert("Failed to find %zu B of physical memory (aligned %zu)\n",
	         size, align);
	return NULL;
}

void * pzalloc(size_t size, size_t align)
{
	void * alloc = pmalloc(size, align);
	if (alloc == NULL)
		return alloc;

	uint8_t * mem = alloc;
	for (size_t i = 0; i < size; i++)
		mem[i] = 0;

	return alloc;
}

void pfree(void * ptr, size_t size)
{
	free_block(ptr, size);
	pr_debug("Freed %zu B at %p\n", size, ptr);
}
