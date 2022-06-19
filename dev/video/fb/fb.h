#ifndef _DEV_VIDEO_FB_FB_H
#define _DEV_VIDEO_FB_FB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <module.h>

struct fb {
	void * fb_base;
	size_t fb_size;
	size_t bytes_per_pixel;
	size_t bytes_per_scanline;
	enum pixel_format {
		PIXEL_FORMAT_32BGR, PIXEL_FORMAT_32RGB,
		PIXEL_FORMAT_32MASK,
	} pixel_format;
	union pixel_mask {
		struct pixel_mask_32 {
			uint32_t r;
			uint32_t g;
			uint32_t b;
		} m32;
	} pixel_mask;
};

extern const struct module fb_mod;

#endif // _DEV_VIDEO_FB_FB_H
