#define pr_fmt(fmt) "fb: " fmt

#include "console.h"

#include <stddef.h>
#include <errno.h>

#include <device.h>
#include <console.h>
#include <fonts.h>
#include <video/fb.h>
#include <printk.h>
#include <alloc.h>
#include "fb.h"

static size_t total_fbcon_count = 0;

static void fbcon_clear(const struct device * dev);
static void fbcon_update(const struct device * dev);
static void fbcon_disable(const struct device * dev);
static int fbcon_enable(const struct device * dev);

static int _fbcon_reg(const struct device * dev)
{
	int err;

	err = console_reg(dev, fbcon_update, fbcon_clear, fbcon_enable,
			  fbcon_disable);
	if (err)
		return err;

	return 0;
}

static void _fbcon_unreg(const struct device * dev)
{
	kfree(dev->driver_data);
}

int fbcon_reg(const struct device * fb)
{
	int err;

	struct fbcon * fbcon = malloc(sizeof(*fbcon));
	if (fbcon == NULL)
		return -ENOMEM;

	if ((err = font_load_default(&fbcon->font))) {
		kfree(fbcon);
		return err;
	}
	fbcon->x = 0;
	fbcon->y = 0;
	fbcon->kmsg_ptr = NULL;

	err = device_create(NULL, &fb_mod, fb, "graphics", "con", _fbcon_reg,
	                    _fbcon_unreg, fbcon, "fbcon%zu", total_fbcon_count);
	if (err) {
		kfree(fbcon);
		return err;
	}

	total_fbcon_count++;
	return 0;
}

static int fbcon_newline(struct fbcon * con, const struct fb * fb)
{
	size_t font_h = font_height(con->font);

	con->x = 0;

	if ((con->y + 2) * font_h * fb->bytes_per_scanline >= fb->fb_size)
		return fb_move_up(fb, font_h);

	con->y++;
	return 0;
}

static int fbcon_nextc(struct fbcon * con, const struct fb * fb)
{
	size_t font_w = font_width(con->font);

	con->x++;
	if ((con->x + 1) * font_w * fb->bytes_per_pixel
	    > fb->bytes_per_scanline)
		return fbcon_newline(con, fb);
	return 0;
}

static int fbcon_putc(struct fbcon * con, const struct fb * fb, uint32_t c)
{
	int ret;
	size_t font_w = font_width(con->font);
	size_t font_h = font_height(con->font);

	if (c == '\n')
		return fbcon_newline(con, fb);

	bool * bitmap = malloc(font_w * font_h * sizeof(*bitmap));
	if (bitmap == NULL)
		return -ENOMEM;

	if ((ret = font_bitmap(bitmap, con->font, c))) {
		kfree(bitmap);
		return ret;
	}

	for (size_t y = 0; y < font_h; y++) {
		for (size_t x = 0; x < font_w; x++) {
			unsigned int c = (bitmap[y * font_w + x]) ? -1 : 0;
			if ((ret = fb_plot(
					fb, font_w * con->x + x,
					font_h * con->y + y, c, c, c
			)))
				return ret;
		}
	}

	kfree(bitmap);
	return fbcon_nextc(con, fb);
}

static int fbcon_puts(struct fbcon * con, const struct fb * fb, const char * s)
{
	int ret;
	while (*s) {
		if ((ret = fbcon_putc(con, fb, *s)))
			return ret;
		s++;
	}
	return 0;
}

static int fbcon_enable(_unused_ const struct device * dev)
{
	return 0;
}

static void fbcon_disable(const struct device * dev)
{
	fbcon_clear(dev);
}

static void fbcon_update(const struct device * dev)
{
	const struct fb * fb = dev->parent->driver_data;
	struct fbcon * con = dev->driver_data;
	const char * newpos = kmsg_next(con->kmsg_ptr);

	while (newpos != NULL) {
		fbcon_puts(con, fb, kmsg_get_str(newpos));
		con->kmsg_ptr = newpos;
		newpos = kmsg_next(con->kmsg_ptr);
	}
}

static void fbcon_clear(const struct device * dev)
{
	const struct fb * fb = dev->parent->driver_data;
	struct fbcon * con = dev->driver_data;

	con->kmsg_ptr = NULL;
	con->x = 0;
	con->y = 0;
	fb_clear(fb);
}
