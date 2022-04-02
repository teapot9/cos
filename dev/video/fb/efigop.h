#ifndef DEV_VIDEO_FB_EFIGOP_H
#define DEV_VIDEO_FB_EFIGOP_H

#include <stddef.h>
#include <firmware/efiapi/console.h>

struct gop {
	unsigned default_mode;
	size_t mode_info_size;
	efi_graphics_output_protocol_mode_t mode;
	efi_graphics_output_mode_information_t info;
	size_t framebuffer_mapping_size;
};

#endif // DEV_VIDEO_FB_EFIGOP_H
