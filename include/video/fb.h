/**
 * @file video/fb.h
 * @brief Video framebuffer device
 */

#ifndef __VIDEO_FB_H
#define __VIDEO_FB_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <device.h>

/// Framebuffer device
struct fb;

/**
 * @brief Register framebuffer blue-green-red 32 bit
 * @param backend Parent video device
 * @param base Framebuffer address start
 * @param size Framebuffer size (bytes)
 * @param bytes_per_pixel bytes per pixel (should be 4)
 * @param bytes_per_scanline bytes per line
 * @return errno
 */
int fb_reg_bgr32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline);

/**
 * @brief Register framebuffer red-green-blue 32 bit
 * @param backend Parent video device
 * @param base Framebuffer address start
 * @param size Framebuffer size (bytes)
 * @param bytes_per_pixel bytes per pixel (should be 4)
 * @param bytes_per_scanline bytes per line
 * @return errno
 */
int fb_reg_rgb32(const struct device * backend,
                 void * base, size_t size, size_t bytes_per_pixel,
                 size_t bytes_per_scanline);

/**
 * @brief Register framebuffer mask-based color 32 bit
 * @param backend Parent video device
 * @param base Framebuffer address start
 * @param size Framebuffer size (bytes)
 * @param bytes_per_pixel bytes per pixel (should be 4)
 * @param bytes_per_scanline bytes per line
 * @return errno
 */
int fb_reg_mask32(const struct device * backend,
                  void * base, size_t size, size_t bytes_per_pixel,
		  size_t bytes_per_scanline,
		  uint32_t r, uint32_t g, uint32_t b);

/**
 * @brief Clear a framebuffer
 * @param fb Framebuffer device
 */
void fb_clear(const struct fb * fb);

/**
 * @brief Plot a pixel
 * @param fb Framebuffer device
 * @param x X coordinate
 * @param y Y coordinate
 * @param r Red color
 * @param g Green color
 * @param b Blue color
 * @return errno
 */
int fb_plot(const struct fb * fb, unsigned int x, unsigned int y,
            unsigned int r, unsigned int g, unsigned int b);

/**
 * @brief Move all lines up
 * @param fb Framebuffer device
 * @param scanlines By how many lines to move up
 * @return errno
 */
int fb_move_up(const struct fb * fb, unsigned int scanlines);

#ifdef __cplusplus
}
#endif
#endif // __VIDEO_FB_H
