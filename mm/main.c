#include <mm.h>
#include "kmm.h"
#include "vmm.h"
#include "pmm.h"

#include <stdbool.h>

bool mm_is_init = false;

void mm_init_early(void)
{
	if (kmm_early_init())
		return;
	mm_is_init = true;
}

void mm_init(struct memmap map) {
	pmm_init(map);
	vmm_init(map);
}
