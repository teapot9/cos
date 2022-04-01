#include <video/fb.h>

#include <stdbool.h>

#include "efigop.h"

bool fb_is_init = false;

void fb_init(void)
{
	efigop_init();
	fb_is_init = true;
}
