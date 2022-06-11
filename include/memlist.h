/**
 * @file memlist.h
 * @brief Memory list data structure
 */

#ifndef MEMLIST_H
#define MEMLIST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>

struct memlist_elt;

/**
 * @brief Memory list
 */
struct memlist {
	/// Base list data
	struct list l;
	/// Real element size (contains at least a struct memlist_elt)
	size_t elt_size;
	/// Callback to check if memory list elements can be merged
	bool (*compat)(struct memlist_elt * a, struct memlist_elt * b);
};

/**
 * @brief Memory list element
 */
struct memlist_elt {
	/// Base list data
	struct list_head l;
	/// Memory block start address
	void * addr;
	/// Memory block size
	size_t size;
};

/**
 * @brief Create new memory list
 * @param compat_fcn `compat` callback
 * @param size `elt_size`
 * @return struct memlist
 */
#define memlist_new(compat_fcn, size) (struct memlist) { \
		.l = list_new(), \
		.compat = compat_fcn, \
		.elt_size = size, \
	}

/**
 * @brief Create a basic memory list
 * @return struct memlist
 */
#define memlist_new_default() memlist_new(NULL, sizeof(struct memlist_elt))

/**
 * @brief Free memory list
 * @param list Memory list pointer
 * @param free_ptr If true, `free(list)` will be called; if false, `*list`
 * 	will be set to an empty list
 */
static inline void memlist_free(struct memlist * list, bool free_ptr)
{
	list_free_all(&list->l, free_ptr);
	if (!free_ptr)
		*list = memlist_new_default();
}

/**
 * @brief Copy a memory list
 * @param dst Destination list
 * @param src Source
 * @return errno
 */
static inline int memlist_copy(struct memlist * dst, struct memlist * src)
{
	return list_copy((struct list *) dst, (struct list *) src,
	                 sizeof(struct memlist_elt));
}

/**
 * @brief Add an element
 * @param l List
 * @param elt Element to add
 * @param strict Fail if a collision with an existing element occurs
 * @return errno
 */
int memlist_add_elt(struct memlist * l, struct memlist_elt * elt, bool strict);

/**
 * @brief Add an element
 * @param l List
 * @param addr Memory address
 * @param size Memory size
 * @param strict Fail if a collision with an existing element occurs
 * @return errno
 */
int memlist_add(struct memlist * l, void * addr, size_t size, bool strict);

/**
 * @brief Remove an element
 * @param l List
 * @param elt Element to remove
 * @param strict Fail if the element is not within a single existing element
 * @return errno
 */
int memlist_del_elt(struct memlist * l, struct memlist_elt * elt, bool strict);

/**
 * @brief Remove an element
 * @param l List
 * @param addr Memory address
 * @param size Memory size
 * @param strict Fail if the element is not within a single existing element
 * @return errno
 */
int memlist_del(struct memlist * l, void * addr, size_t size, bool strict);

/**
 * @brief Find memory block by address
 * @param l List
 * @param addr Searched address
 * @return Found memory block
 */
struct memlist_elt * memlist_get_ptr(struct memlist * l, void * addr);

/**
 * @brief Find memory block by addres and size
 * @param l List
 * @param addr Block address
 * @param size Block size
 * @param full Fail if the requested memory block is overlapping with multiple
 * 	blocks in the list
 * @return Found memory block
 */
struct memlist_elt * memlist_get(
	struct memlist * l, void * addr, size_t size, bool full
);

/**
 * @brief Find memory block by size
 * @param l List
 * @param size Searched block size
 * @param align Memory alignment (used for size calculation)
 * @return Found memory block (at least size + (start % align) bytes)
 */
struct memlist_elt * memlist_search(
	struct memlist * l, size_t size, size_t align
);

/**
 * @brief Address space size of memory list (from 0)
 * @param l List
 * @return last_block.addr + last_block.size
 */
size_t memlist_virtual_size(struct memlist * l);

#ifdef __cplusplus
}
#endif
#endif // MEMLIST_H
