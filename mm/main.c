#include <mm.h>
#include "kmm.h"
#include "vmm.h"
#include "pmm.h"

#include <stdbool.h>

/* public: mm.h */
void mm_init_early(void)
{
	if (kmm_early_init())
		return;
}

/* public: mm.h */
void mm_init(struct memmap map) {
	pmm_init(map);
	vmm_init(map);
}
