#define pr_fmt(fmt) "efigop: " fmt

#include "efigop.h"
#include <platform_setup.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include "fb.h"
#include <mm.h>
#include <mm/early.h>
#include <print.h>
#include <firmware/efistub.h>
#include <firmware/efiapi/console.h>
#include <string.h>
#include <video/fb.h>
#include <console.h>

static int gop_reg(const struct device * dev)
{
	int ret;
	struct gop * gop = dev->driver_data;

	void * base = (void *) gop->mode.frame_buffer_base;
	size_t size = gop->mode.frame_buffer_size;
	gop->framebuffer_mapping_size = size;
	base = mmap(0, base, &gop->framebuffer_mapping_size,
	            0, true, false, false);
	if (base == NULL) {
		pr_err("Failed to map EFI framebuffer memory\n", 0);
		return -ENOMEM;
	}

	size_t bpp = 4;
	size_t bps = bpp * gop->mode.info->pixels_per_scanline;
	switch (gop->mode.info->pixel_format) {
	case EFI_PIXEL_RED_GREEN_BLUE_RESERVED_8BIT_PER_COLOR:
		ret = fb_reg_rgb32(dev, base, size, bpp, bps);
		break;
	case EFI_PIXEL_BLUE_GREEN_RED_RESERVED_8BIT_PER_COLOR:
		ret = fb_reg_bgr32(dev, base, size, bpp, bps);
		break;
	case EFI_PIXEL_BIT_MASK:
		ret = fb_reg_mask32(
			dev, base, size, bpp, bps,
			gop->mode.info->pixel_information.red_mask,
			gop->mode.info->pixel_information.green_mask,
			gop->mode.info->pixel_information.blue_mask
		);
		break;
	default:
		pr_err("Unsupported mode for fb driver\n", 0);
		return -ENOTSUP;
	}
	if (ret)
		return ret;

	pr_debug("Initialized: %d modes, current mode: %d\n",
	         gop->mode.max_mode, gop->mode.mode);
	return 0;
}

static void gop_unreg(const struct device * dev)
{
	kfree(dev->driver_data);
}

/* public: platform_setup.h */
int gop_init(struct efigop_bdata * data)
{
	int err;

	struct gop * gop = kmalloc(sizeof(*gop));
	memcpy(gop, &data->gop, sizeof(*gop));
	gop->mode.info = &gop->info;

	/* Register device */
	// TODO: parent = efi boot services? firmware?
	err = device_create(NULL, &fb_mod, NULL, "graphics", "fb", gop_reg,
	                    gop_unreg, gop, "efigop");
	if (err)
		return err;
	return 0;
}
