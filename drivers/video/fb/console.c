#include "console.h"

#include <stddef.h>
#include <errno.h>

#include <console.h>
#include <fonts.h>
#include <video/fb.h>
#include <print.h>
#include <mm.h>
#include "fb.h"

static int fbcon_newline(struct fbcon * con)
{
	size_t font_h = font_height(con->font);

	con->x = 0;

	if ((con->y + 2) * font_h * con->fb->bytes_per_scanline
	    >= con->fb->fb_size)
		return fb_move_up(con->fb, font_h);

	con->y++;
	return 0;
}

static int fbcon_nextc(struct fbcon * con)
{
	size_t font_w = font_width(con->font);

	con->x++;
	if ((con->x + 1) * font_w * con->fb->bytes_per_pixel
	    > con->fb->bytes_per_scanline)
		return fbcon_newline(con);
	return 0;
}

int fbcon_putc(struct fbcon * con, uint32_t c)
{
	int ret;
	size_t font_w = font_width(con->font);
	size_t font_h = font_height(con->font);

	if (c == '\n')
		return fbcon_newline(con);

	bool * bitmap = kmalloc(font_w * font_h * sizeof(*bitmap));
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
					con->fb, font_w * con->x + x,
					font_h * con->y + y, c, c, c
					)))
				return ret;
		}
	}

	kfree(bitmap);
	return fbcon_nextc(con);
}

int fbcon_puts(struct fbcon * con, const char * s)
{
	int ret;
	while (*s) {
		if ((ret = fbcon_putc(con, *s)))
			return ret;
		s++;
	}
	return 0;
}

void fbcon_update(struct fbcon * con)
{
	const char * newpos = kmsg_next(con->kmsg_ptr);

	while (newpos != NULL) {
		fbcon_puts(con, kmsg_get_str(newpos));
		con->kmsg_ptr = newpos;
		newpos = kmsg_next(con->kmsg_ptr);
	}
}

static void _fbcon_update(void * id)
{
	fbcon_update(id);
}

void fbcon_reset(struct fbcon * con)
{
	con->kmsg_ptr = NULL;
	con->x = 0;
	con->y = 0;
	fb_clear(con->fb);
}

static void _fbcon_reset(void * id)
{
	fbcon_reset(id);
}

int fbcon_add(struct fbcon ** dst_con, const struct fb * fb)
{
	int ret;
	if (dst_con == NULL)
		return -EINVAL;
	*dst_con = NULL;

	struct fbcon * con = kmalloc(sizeof(*con));
	if (con == NULL)
		return -ENOMEM;

	con->fb = fb;
	if ((ret = font_load_default(&con->font))) {
		kfree(con);
		return ret;
	}
	fbcon_reset(con);

	if ((ret = console_add(con, _fbcon_update, _fbcon_reset))) {
		kfree(con);
		return ret;
	}

	*dst_con = con;
	return 0;
}
