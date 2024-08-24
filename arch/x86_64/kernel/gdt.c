#include "gdt.h"

#include <errno.h>
#include <stddef.h>

#include <string.h>
#include <alloc.h>
#include <printk.h>

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

static void desc_base_limit(
	struct gdt_desc * desc, void * base, size_t limit
)
{
	desc->limit_low = limit & 0xFFFF;
	desc->base_low = (unsigned long) base & 0xFFFFFF;
	desc->limit_high = (limit >> 16) & 0xF;
	desc->base_high = ((unsigned long) base >> 24) & 0xFF;
}

static void tss_base_limit(
	struct tss_desc * desc, void * base, size_t limit
)
{
	desc_base_limit(&desc->gdt, base, limit);
	desc->base_higher = ((unsigned long) base >> 32) & 0xFFFFFFFF;
	desc->_zero0 = 0;
}

static int init_gdt(struct gdt * gdt, struct tss * tss)
{
	if (gdt == NULL || tss == NULL)
		return -EINVAL;

	// Problem? try rw=true exec=true
	gdt->null = (struct gdt_desc) {0};
	desc_base_limit(&gdt->null, 0, 0);

	gdt->kernel_code.avl = false;
	gdt->kernel_code.l = true;
	gdt->kernel_code.sz = false;
	gdt->kernel_code.gr = false;
	gdt->kernel_code.ac = false;
	gdt->kernel_code.rw_busy = false;
	gdt->kernel_code.dc = false;
	gdt->kernel_code.ex = true;
	gdt->kernel_code.s = true;
	gdt->kernel_code.privl = 0;
	gdt->kernel_code.pr = true;
	desc_base_limit(&gdt->kernel_code, NULL, 0);

	gdt->kernel_data = gdt->kernel_code;
	gdt->kernel_data.rw_busy = true;
	gdt->kernel_data.ex = false;
	desc_base_limit(&gdt->kernel_data, NULL, 0);

	gdt->user_code = gdt->kernel_code;
	gdt->user_code.privl = 3;
	desc_base_limit(&gdt->user_code, NULL, 0);

	gdt->user_data = gdt->kernel_data;
	gdt->user_data.privl = 3;
	desc_base_limit(&gdt->user_data, NULL, 0);

	gdt->tss.gdt.avl = false;
	gdt->tss.gdt.l = true;
	gdt->tss.gdt.sz = false;
	gdt->tss.gdt.gr = false;
	gdt->tss.gdt.ac = true;
	gdt->tss.gdt.rw_busy = false;
	gdt->tss.gdt.dc = false;
	gdt->tss.gdt.ex = true;
	gdt->tss.gdt.s = false;
	gdt->tss.gdt.privl = 0;
	gdt->tss.gdt.pr = true;
	tss_base_limit(&gdt->tss, tss, sizeof(*tss));

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

	set_cs(gdt_segment(GDT_KERN_CS));
	set_ds(gdt_segment(GDT_KERN_DS));
	set_ss(gdt_segment(GDT_KERN_DS));
	set_es(gdt_segment(GDT_KERN_DS));
	set_fs(gdt_segment(GDT_KERN_DS));
	set_gs(gdt_segment(GDT_KERN_DS));

	ltr(gdt_segment(GDT_TSS));

	return 0;
}

/* public: gdt.h */
unsigned int gdt_segment(enum gdt_type type)
{
	return type * sizeof(struct gdt_desc);
}

/* public: gdt.h */
void tss_set_kstack(struct tss * tss, void * kstack)
{
	tss->rsp0 = (uint64_t) kstack;
}
