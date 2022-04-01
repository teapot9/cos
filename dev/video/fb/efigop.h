#ifndef DEV_VIDEO_FB_EFIGOP_H
#define DEV_VIDEO_FB_EFIGOP_H

#include <stddef.h>
#include <firmware/efiapi/console.h>

struct gop {
	unsigned default_mode;
	size_t mode_info_size;
	efi_graphics_output_protocol_mode_t mode;
	efi_graphics_output_mode_information_t info;
};

struct efigop_bdata {
	struct gop gop;
};

#endif // DEV_VIDEO_FB_EFIGOP_H