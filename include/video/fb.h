#ifndef VIDEO_FB_H
#define VIDEO_FB_H

#include <stddef.h>
#include <stdint.h>

#include <device.h>

struct fb;
struct fbcon;

int fb_reg_bgr32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline);
int fb_reg_rgb32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline);
int fb_reg_mask32(const struct device * backend,
                  void * base, size_t size, size_t bytes_per_pixel,
		  size_t bytes_per_scanline,
		  uint32_t r, uint32_t g, uint32_t b);

void fb_clear(const struct fb * fb);

int fb_plot(const struct fb * fb, unsigned int x, unsigned int y,
            unsigned int r, unsigned int g, unsigned int b);

int fb_move_up(const struct fb * fb, unsigned int scanlines);

#endif // VIDEO_FB_H
