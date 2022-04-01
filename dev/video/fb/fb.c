#include <video/fb.h>
#include "fb.h"

#include <errno.h>

#include <device.h>
#include <mm.h>
#include <string.h>
#include "console.h"

static size_t total_fb_count = 0;
static const union pixel_mask pixel_mask_zero = {0, 0, 0};

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

static int _fb_reg(const struct device * dev)
{
	int err;

	err = fbcon_reg(dev);
	if (err)
		return err;

	return 0;
}

static void _fb_unreg(const struct device * dev)
{
	kfree(dev->driver_data);
}

static int fb_reg(
	const struct device * backend,
	void * base, size_t size,
	size_t bytes_per_pixel, size_t bytes_per_scanline,
	enum pixel_format fmt,
	union pixel_mask mask
)
{
	int err;

	struct fb * fb = kmalloc(sizeof(*fb));
	if (fb == NULL)
		return -ENOMEM;

	fb->fb_base = base;
	fb->fb_size = size;
	fb->bytes_per_pixel = bytes_per_pixel;
	fb->bytes_per_scanline = bytes_per_scanline;
	fb->pixel_format = fmt;
	fb->pixel_mask = mask;

	err = device_create(NULL, &fb_mod, backend, "graphics", "fb", _fb_reg,
			    _fb_unreg, fb, "fb%zu", total_fb_count);
	if (err) {
		kfree(fb);
		return err;
	}

	total_fb_count++;
	return 0;
}

/* public: video/fb.h */
int fb_reg_bgr32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline)
{
	return fb_reg(backend, base, size, bytes_per_pixel, bytes_per_scanline,
	              PIXEL_FORMAT_32BGR, pixel_mask_zero);
}

/* public: video/fb.h */
int fb_reg_rgb32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline)
{
	return fb_reg(backend, base, size, bytes_per_pixel, bytes_per_scanline,
	              PIXEL_FORMAT_32RGB, pixel_mask_zero);
}

/* public: video/fb.h */
int fb_reg_mask32(const struct device * backend,
                  void * base, size_t size, size_t bytes_per_pixel,
		  size_t bytes_per_scanline, uint32_t r, uint32_t g, uint32_t b)
{
	union pixel_mask mask = {.m32.r = r, .m32.g = g, .m32.b = b};
	return fb_reg(backend, base, size, bytes_per_pixel, bytes_per_scanline,
	              PIXEL_FORMAT_32MASK, mask);
}

/* public: video/fb.h */
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

/* public: video/fb.h */
void fb_clear(const struct fb * fb)
{
	switch (fb->pixel_format) {
	case PIXEL_FORMAT_32BGR:
	case PIXEL_FORMAT_32RGB:
	case PIXEL_FORMAT_32MASK:
		memset(fb->fb_base, 0, fb->fb_size);
	}
}

/* public: video/fb.h */
int fb_move_up(const struct fb * fb, unsigned int scanlines)
{
	size_t offset = fb->bytes_per_scanline * scanlines;
	memcpy(fb->fb_base, (uint8_t *) fb->fb_base + offset,
	       fb->fb_size - offset - 1);
	memset((uint8_t *) fb->fb_base + fb->fb_size - offset + 1, 0, offset);
	return 0;
}
