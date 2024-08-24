#ifndef __GDT_H
#define __GDT_H

enum gdt_type {
	GDT_KERN_CS = 1,
	GDT_KERN_DS = 2,
	GDT_USER_CS = 3,
	GDT_USER_DS = 4,
	GDT_TSS = 5,
};

struct tss;

unsigned int gdt_segment(enum gdt_type);
void tss_set_kstack(struct tss * tss, void * kstack);

#endif // __GDT_H
