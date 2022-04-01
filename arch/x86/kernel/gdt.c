#include "gdt.h"

#include <errno.h>
#include <stddef.h>

#include <string.h>
#include <mm.h>
#include <print.h>

static int init_tss(struct tss * tss, void * kernel_stack)
{
	if (tss == NULL)
		return -EINVAL;

	memset(tss, 0, sizeof(*tss));
	tss->rsp0 = (uint64_t) kernel_stack;
	tss->rsp1 = (uint64_t) NULL;
	tss->rsp2 = (uint64_t) NULL;
	tss->iopb_offset = sizeof(*tss);

	return 0;
}

static struct gdt_desc new_desc(
	void * base, size_t limit,
	union desc_access access, union desc_flags flags
)
{
	return (struct gdt_desc) {
		.limit_low = limit & 0xFFFF,
		.base_low = (unsigned long) base & 0xFFFFFF,
		.access = access,
		.limit_high = (limit >> 16) & 0xF,
		.flags = flags,
		.base_high = ((unsigned long) base >> 24) & 0xFF,
	};
}

static int init_gdt(struct gdt * gdt, struct tss * tss)
{
	if (gdt == NULL || tss == NULL)
		return -EINVAL;

	// Problem? try rw=true exec=true
	//memset(gdt, 0, sizeof(*gdt));
	union desc_access access = {0};
	union desc_flags flags = {0};
	gdt->null = new_desc(0, 0, access, flags);

	flags.gdt = (struct gdt_desc_flags) {
		._reserved0 = 0,
		.l = true,
		.sz = false,
		.gr = false,
	};
	access.gdt = (struct gdt_desc_access) {
		.ac = false,
		.rw = false,
		.dc = false,
		.ex = true,
		.s = true,
		.privl = 0,
		.pr = true,
	};
	gdt->kernel_code = new_desc(NULL, 0, access, flags);

	access.gdt.rw = true;
	access.gdt.ex = false;
	gdt->kernel_data = new_desc(NULL, 0, access, flags);

	access.gdt.privl = 3;
	gdt->user_data = new_desc(NULL, 0, access, flags);

	access.gdt.rw = false;
	access.gdt.ex = true;
	gdt->user_code = new_desc(NULL, 0, access, flags);

	access.tss = (struct tss_desc_access) {
		._one0 = -1,
		.busy = false,
		._zero0 = 0,
		._one1 = -1,
		._zero1 = 0,
		.privl = 0,
		.pr = true,
	};
	flags.tss = (struct tss_desc_flags) {
		.avl = false,
		._zero0 = 0,
		.gr = false,
	};
	gdt->tss = new_desc(tss, sizeof(*tss), access, flags);

	return 0;
}

int gdt_create_load(struct gdt * gdt, struct tss * tss, void * kstack)
{
	int err;
	err = init_tss(tss, kstack);
	if (err)
		return err;
	err = init_gdt(gdt, tss);
	if (err)
		return err;

	pr_debug("loading new GDT [%p]\n", gdt);
	struct gdtr gdtr = {
		.offset = gdt,
		.size = sizeof(*gdt),
	};
	lgdt(gdtr);

	return 0;
}
