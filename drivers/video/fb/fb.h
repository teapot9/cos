#ifndef DRIVERS_VIDEO_FB_FB_H
#define DRIVERS_VIDEO_FB_FB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/// TODO: driver initialized
extern bool fb_is_init;

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

int fb_add(const struct fb ** ref, struct fb fb);

int fb_remove(const struct fb * ref);

#endif // DRIVERS_VIDEO_FB_FB_H
