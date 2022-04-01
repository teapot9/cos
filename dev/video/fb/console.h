#ifndef DRIVERS_VIDEO_FB_CONSOLE_H
#define DRIVERS_VIDEO_FB_CONSOLE_H

#include <device.h>

struct fbcon {
	const struct font * font;
	unsigned int x;
	unsigned int y;
	const char * kmsg_ptr;
};

int fbcon_reg(const struct device * fb);

#endif // DRIVERS_VIDEO_FB_CONSOLE_H
