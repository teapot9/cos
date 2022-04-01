#ifndef VIDEO_FB_H
#define VIDEO_FB_H

#include <stdint.h>

struct fb;
struct fbcon;

/// TODO: initialize the fb driver
void fb_init(void);

void fb_clear(const struct fb * fb);

int fb_plot(const struct fb * fb, unsigned int x, unsigned int y,
            unsigned int r, unsigned int g, unsigned int b);

int fb_move_up(const struct fb * fb, unsigned int scanlines);

int fbcon_putc(struct fbcon * con, uint32_t c);

int fbcon_puts(struct fbcon * con, const char * s);

#endif // VIDEO_FB_H
