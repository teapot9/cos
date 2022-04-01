#ifndef BOOT_H
#define BOOT_H

struct memmap_descriptor {
	enum {
		MEMMAP_TYPE_RESERVED,
		MEMMAP_TYPE_AVAILABLE,
		MEMMAP_TYPE_FIRMWARE,
		MEMMAP_TYPE_ACPI,
		MEMMAP_TYPE_MAPPED_IO,
		MEMMAP_TYPE_PERSISTENT,
		MEMMAP_TYPE_UNUSABLE,
	} type;
	void * physical_start;
	void * virtual_start;
	size_t size;
};

struct memmap {
	size_t size;
	struct memmap_descriptor * descriptors;
};

#endif // BOOT_H
