#ifndef MM_MEMMAP_H
#define MM_MEMMAP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <list.h>
#include <memlist.h>
#include <task.h>
#include <kconfig.h>

/* Data types definitions */

/**
 * @brief Memory map
 */
struct memmap {
	/// Base memory list data
	struct memlist l;
};

/**
 * @brief Memory map block
 */
struct memmap_elt {
	/// Base memory list data
	struct memlist_elt l;
	/// Type of the memory block
	enum memory_type {
		MEMORY_TYPE_RESERVED,
		MEMORY_TYPE_EFI_SERVICES,
		MEMORY_TYPE_ACPI_RECLAIMABLE,
		MEMORY_TYPE_ACPI_NVS,
		MEMORY_TYPE_PERSISTENT,
		MEMORY_TYPE_USED,
		MEMORY_TYPE_FREE,
	} type;
	/// Owner of the memory block
	pid_t owner;
};

/* Global functions */

/**
 * @brief Create a new memory map
 * @return New memory map
 */
struct memmap memmap_new(void);

/**
 * @brief Free memory map from memory
 * @param map Memory map to free
 * @param free_ptr If true, `free(map)` will also be called
 */
static inline void memmap_free(struct memmap * map, bool free_ptr)
{
	memlist_free(&map->l, free_ptr);
	if (!free_ptr)
		*map = memmap_new();
}

/**
 * @brief Copy memory map
 * @param dst Pointer of the destination memmap
 * @param src Pointer of the source memmap
 * @return errno
 */
static inline int memmap_copy(struct memmap * dst, struct memmap * src)
{
	return list_copy((struct list *) dst, (struct list *) src,
	                 sizeof(struct memmap_elt));
}

#if IS_ENABLED(CONFIG_MM_DEBUG)
/**
 * @brief Dump memmap to console
 * @param map Memory map
 * @param prefix String to prepend to all printed lines
 */
void memmap_dump(struct memmap * map, const char * prefix);
#endif

/* Helpers */

/**
 * @brief Get string describing memory type
 * @param type Memory type
 * @return String
 */
const char * memmap_strtype(enum memory_type type);

/* Block functions */

/**
 * @brief Find memory block by address
 * @param m Memmap
 * @param addr Start of block
 * @param size Size of block
 * @param full Fail if the requested block is not inside a single memmap block
 * @return Pointer to the first memmap_elt overlapping with the requested block
 */
static inline struct memmap_elt * memmap_find_addr(
	struct memmap * m, void * addr, size_t size, bool full
)
{
	return (struct memmap_elt *) memlist_get(&m->l, addr, size, full);
}

/**
 * @brief Find memory block by size, type and owner
 * @param map Memory map
 * @param size Size of the block
 * @param align Alignment of the block
 * @param type Type of block
 * @param owner Owner of block
 * @return First matching block (note: start address may not be aligned)
 */
struct memmap_elt * memmap_find_size(
	struct memmap * map, size_t size, size_t align,
	enum memory_type type, pid_t owner
);

/**
 * @brief Get the type of a memory block
 * @param dst Where the type will be stored
 * @param map Memory map
 * @param addr Block start
 * @param size Block size
 * @return errno
 */
int memmap_type(
	enum memory_type * dst, struct memmap * map, void * addr, size_t size
);

/**
 * @brief Update memory map
 * @param map Memory map
 * @param start Block start
 * @param size Block size
 * @param type Block type
 * @param owner Block owner
 * @return errno
 *
 * This updates the memory map, splitting/merging existing memory block
 * informations as needed.
 */
int memmap_update(
	struct memmap * map, void * start, size_t size,
	enum memory_type type, pid_t owner
);

/**
 * @brief Undefine a memory block
 * @param map Memory map
 * @param start Block start
 * @param size Block size
 * @return errno
 */
int memmap_undef(
	struct memmap * map, void * start, size_t size
);

#ifdef __cplusplus
}
#endif
#endif // MM_MEMMAP_H
