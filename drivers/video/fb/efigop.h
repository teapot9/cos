#ifndef DRIVERS_VIDEO_FB_EFIGIP_H
#define DRIVERS_VIDEO_FB_EFIGIP_H

//struct efigop_fb;

/// TODO: initialize the efigop driver
int efigop_init(void);

//int efigop_plot(struct efigop_fb * fb, unsigned int x, unsigned int y,
//                unsigned int r, unsigned int g, unsigned int b);

void efigop_disable_console(void);
void efigop_disable(void);
#endif // DRIVERS_VIDEO_FB_EFIGIP_H
