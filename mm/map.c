#include <mm/map.h>
#include "map.h"

#include <errno.h>
#include <stdint.h>

#include <mm.h>
#include <mm/helper.h>

int memmap_add(struct memmap_list ** map, void * addr1, void * addr2, size_t size)
{
	if (map == NULL)
		return -EINVAL;

	struct memmap_list * new = kmalloc(sizeof(*new));
	if (new == NULL)
		return -ENOMEM;

	new->addr1 = addr1;
	new->addr2 = addr2;
	new->size = size;
	new->next = *map;
	*map = new;
	return 0;
}

void * memmap_translate(struct memmap_list ** map, void * addr)
{
	if (map == NULL)
		return NULL;
	struct memmap_list * cur = *map;

	while (cur != NULL) {
		if (is_inside(cur->addr1, cur->size, addr, 0))
			return (uint8_t *) cur->addr2 + (
				(uint8_t *) addr - (uint8_t *) cur->addr1
			);
		cur = cur->next;
	}
	return NULL;
}
