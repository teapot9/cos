/**
 * @file list.h
 * @brief Linked list
 */

#ifndef __LIST_H
#define __LIST_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

#include <spinlock.h>
#include <cpp.h>
#include <assert.h>

/**
 * @brief Linked list
 *
 * A base list does not contains any data, it should be "sub-classed" by
 * creating data structures containing a `struct list_head` as their
 * first component. See `memlist.h` for an example.
 */
struct list {
	/// First element
	struct list_head * first;
	/// Last element
	struct list_head * last;
	/// Internal lock
	struct spinlock lock;
};

#include <lock.h>

/**
 * @brief List data header
 */
struct list_head {
	/// Next element pointer
	struct list_head * next;
	/// Previous element pointer
	struct list_head * prev;
};

/**
 * @brief Initialize a list
 * @return struct list
 */
#define list_new() (struct list) \
	{.first = NULL, .last = NULL, .lock = spinlock_init()}

/**
 * @brief Free a list
 * @param l List
 * @param free_list If true, `free(l)` will be called
 */
void list_free_all(struct list * l, bool free_list);

/**
 * @brief Copy a list
 * @param dst Destination list
 * @param l Source list
 * @param elt_size Size of a list element
 * @return errno
 */
int list_copy(struct list * dst, struct list * l, size_t elt_size);

/**
 * @brief Add an element
 * @param l List
 * @param index Index of the new element
 * @param elt Element to add
 */
void list_add(struct list * l, size_t index, struct list_head * elt);

/**
 * @brief Remove an element
 * @param l List
 * @param elt Element to remove
 * @param free If true, `free(elt)` will be called
 */
void list_del(struct list * l, struct list_head * elt, bool free);

/**
 * @brief Add an element to the end of the list
 * @param l List
 * @param elt Element to add
 */
void list_append(struct list * l, struct list_head * elt);

/**
 * @brief Add an element to the start of the list
 * @param l List
 * @param elt Element to add
 */
void list_push(struct list * l, struct list_head * elt);

/**
 * @brief Add an element after a provided one
 * @param l List
 * @param prev Element preceding the new one
 * @param elt Element to add after `prev`
 */
void list_add_after(struct list * l, struct list_head * prev,
                    struct list_head * elt);

/**
 * @brief Add an element before a provided one
 * @param l List
 * @param next Element succeeding the new one
 * @param elt Element to add before `next`
 */
void list_add_before(struct list * l, struct list_head * next,
		     struct list_head * elt);

/**
 * @brief For loop iterating over a list
 * @param elt Iterator
 * @param start Starting element
 * @return for (elt = start; elt != NULL; elt = elt->next)
 */
#define list_foreach(elt, start) \
	static_assert(same_type(start, struct list_head *), \
	              "start should be a struct list_head *"); \
	for (elt = (typeof(elt)) start; elt != NULL; \
	     elt = (typeof(elt)) ((struct list_head *) elt)->next)

#ifdef __cplusplus
}
#endif
#endif // __LIST_H
