#include <video/fb.h>
#include "fb.h"

#include <errno.h>

#include <mm.h>
#include <string.h>

struct fb_list {
	struct fb_list * prec;
	struct fb_list * next;
	const struct fb fb;
};

struct fb_list * fb_list = NULL;

static void set_pixel_mask_32(uint32_t * dst, uint32_t color, uint32_t mask)
{
	for (size_t i = 0; i < 32; i++) {
		if (mask & (1 << i)) {
			if (color & 1)
				*(uint32_t *) dst |= (1 << i);
			else
				*(uint32_t *) dst &= ~(1 << i);
			color >>= 1;
		}
	}
}

int fb_add(const struct fb ** ref, struct fb fb)
{
	struct fb_list * elt = kmalloc(sizeof(*elt));
	if (elt == NULL)
		return -ENOMEM;
	*(struct fb *) &elt->fb = fb; // init const
	elt->prec = NULL;

	if (fb_list == NULL) {
		elt->next = NULL;
		fb_list = elt;
		*ref = &elt->fb;
		return 0;
	}

	elt->next = fb_list;
	fb_list->prec = elt;
	fb_list = elt;
	*ref = &elt->fb;
	return 0;
}

int fb_remove(const struct fb * ref)
{
	struct fb_list * cur = fb_list;

	while (cur != NULL) {
		if (&cur->fb != ref) {
			cur = cur->next;
			continue;
		}
		cur->prec->next = cur->next;
		cur->next->prec = cur->prec;
		kfree(cur);
	}
	return 0;
}

int fb_plot(const struct fb * fb, unsigned int x, unsigned int y,
            unsigned int r, unsigned int g, unsigned int b)
{
	if (fb == NULL || fb->fb_base == NULL)
		return -EINVAL;

	size_t offset = y * fb->bytes_per_scanline + x * fb->bytes_per_pixel;
	void * dst = (void *) ((uint8_t *) fb->fb_base + offset);
	if (offset + fb->bytes_per_pixel >= fb->fb_size)
		return -EINVAL;

	switch (fb->pixel_format) {
	case PIXEL_FORMAT_32BGR:
		*(uint32_t *) dst = ((b & 0xFF) << 0)
			| ((g & 0xFF) << 8)
			| ((r & 0xFF) << 16);
		break;
	case PIXEL_FORMAT_32RGB:
		*(uint32_t *) dst = ((r & 0xFF) << 0)
			| ((g & 0xFF) << 8)
			| ((b & 0xFF) << 16);
		break;
	case PIXEL_FORMAT_32MASK:
		set_pixel_mask_32(dst, r, fb->pixel_mask.m32.r);
		set_pixel_mask_32(dst, g, fb->pixel_mask.m32.g);
		set_pixel_mask_32(dst, b, fb->pixel_mask.m32.b);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

void fb_clear(const struct fb * fb)
{
	switch (fb->pixel_format) {
	case PIXEL_FORMAT_32BGR:
	case PIXEL_FORMAT_32RGB:
	case PIXEL_FORMAT_32MASK:
		memset(fb->fb_base, 0, fb->fb_size);
	}
}

int fb_move_up(const struct fb * fb, unsigned int scanlines)
{
	size_t offset = fb->bytes_per_scanline * scanlines;
	memcpy(fb->fb_base, (uint8_t *) fb->fb_base + offset,
	       fb->fb_size - offset - 1);
	memset((uint8_t *) fb->fb_base + fb->fb_size - offset + 1, 0, offset);
	return 0;
}

#include <asm.h>
#include <print.h>
int myfb(void) {
	pr_debug("myfb is here !\n", 0);
	return 22;
}

//typedef void (*initcall_t)(void);
static void * __attribute__((__section__(".discard.addressable"))) __attribute__((__used__)) ___stub = (void*) &myfb;
asm(intel(".section \".initcalltest\", \"a\"\n"
    "myname:\n"
    ".long myfb - .\n"
    ".previous\n"));
//static initcall_t myname __attribute__((__used__)) __attribute__((__section__(".initcalltest"))) = myfb;
