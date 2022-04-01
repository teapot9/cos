#ifndef DRIVERS_VIDEO_FB_CONSOLE_H
#define DRIVERS_VIDEO_FB_CONSOLE_H

struct fbcon {
	const struct fb * fb;
	const struct font * font;
	unsigned int x;
	unsigned int y;
	const char * kmsg_ptr;
};

void fbcon_update(struct fbcon * con);

void fbcon_reset(struct fbcon * con);

int fbcon_add(struct fbcon ** dst_con, const struct fb * fb);

#endif // DRIVERS_VIDEO_FB_CONSOLE_H
