#define pr_fmt(fmt) "efigop: " fmt

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include <mm.h>
#include <print.h>
#include <firmware/efistub.h>
#include <firmware/efiapi/console.h>
#include <string.h>
#include <video/fb.h>
#include "fb.h"
#include "console.h"
#include <console.h>

// Modes information
static unsigned int mode_default = 0;
static size_t size_of_mode_info = 0;
static efi_graphics_output_protocol_mode_t mode_current_base = {0};
static efi_graphics_output_mode_information_t mode_current_info = {0};
static const efi_graphics_output_mode_information_t * saved_modes = NULL;

static efi_graphics_output_mode_information_t * save_modes(
	efi_graphics_output_protocol_t * gop
)
{
	efi_status_t status;
	efi_graphics_output_mode_information_t * info = NULL;
	efi_graphics_output_mode_information_t * modes =
		kmalloc(mode_current_base.max_mode * size_of_mode_info);

	if (modes == NULL) {
		pr_err("Cannot allocate memory for GOP modes list, "
		       "%d bytes needed", mode_current_base.max_mode * sizeof(*modes));
		return NULL;
	}

	for (efi_uintn i = 0; i < mode_current_base.max_mode; i++) {
		efi_graphics_output_mode_information_t * modeptr =
			(void *) ((uint8_t *) modes + i * size_of_mode_info);

		status = gop->query_mode(gop, i, &size_of_mode_info, &info);
		if (status != EFI_SUCCESS) {
			memset(modeptr, 0, sizeof(*modeptr));
		}

		*modeptr = *info;
	}

	return modes;
}

static void print_current_mode(void)
{
	pr_debug(
		"Current mode %d: width %d, height %d, "
		"pixel format 0x%x, "
		"pixels per scanline %d, framebuffer base 0x%x, "
		"framebuffer size %d\n",
		mode_current_base.mode,
		mode_current_info.horizontal_resolution,
		mode_current_info.vertical_resolution,
		mode_current_info.pixel_format,
		mode_current_info.pixels_per_scanline,
		mode_current_base.frame_buffer_base,
		mode_current_base.frame_buffer_size
	);
}

static inline void update_current_mode(
	efi_graphics_output_protocol_t * gop
)
{
	mode_current_base = *gop->mode;
	mode_current_info = *gop->mode->info;
	mode_current_base.info = &mode_current_info;
}

static int set_mode(efi_graphics_output_protocol_t * gop, int mode)
{
	int ret = 0;
	efi_status_t status;
	efi_graphics_output_blt_pixel_t black = {0, 0, 0, 0};

	pr_debug("Setting mode to %d\n", mode);

	status = gop->set_mode(gop, mode);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to set mode to %d (error %d)\n", mode, status);
		ret = -1;
	} else {
		update_current_mode(gop);
	}

	status = gop->blt(gop, &black, EFI_BLT_VIDEO_FILL, 0, 0, 0, 0,
			  mode_current_info.horizontal_resolution,
			  mode_current_info.vertical_resolution, 0);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to clear the screen (error %d)\n", status);
		ret = -1;
	}

	print_current_mode();
	return ret;
}

static int gop_reg(const struct device * dev)
{
	int ret;

	void * base = (void *) mode_current_base.frame_buffer_base;
	size_t size = mode_current_base.frame_buffer_size;
	size_t bpp = 4;
	size_t bps = bpp * mode_current_info.pixels_per_scanline;
	switch (mode_current_info.pixel_format) {
	case EFI_PIXEL_RED_GREEN_BLUE_RESERVED_8BIT_PER_COLOR:
		ret = fb_reg_rgb32(dev, base, size, bpp, bps);
		break;
	case EFI_PIXEL_BLUE_GREEN_RED_RESERVED_8BIT_PER_COLOR:
		ret = fb_reg_bgr32(dev, base, size, bpp, bps);
		break;
	case EFI_PIXEL_BIT_MASK:
		ret = fb_reg_mask32(
			dev, base, size, bpp, bps,
			mode_current_info.pixel_information.red_mask,
			mode_current_info.pixel_information.green_mask,
			mode_current_info.pixel_information.blue_mask
		);
		break;
	default:
		pr_err("Unsupported mode for fb driver\n", 0);
		return -ENOTSUP;
	}
	if (ret)
		return ret;

	pr_debug("Initialized: %d modes, current mode: %d\n",
	         mode_current_base.max_mode, mode_current_base.mode);
	return 0;
}

static void gop_unreg(__attribute__((__unused__)) const struct device * dev)
{
}

static int gop_init(void)
{
	efi_status_t status;
	int err;
	efi_graphics_output_protocol_t * gop = NULL;
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	/* Check support */
	status = efistub_system_table()->boot_services->locate_protocol(
		&gop_guid, NULL, (void **) &gop
	);
	if (status != EFI_SUCCESS)
		return -ENOTSUP;
	if (gop == NULL)
		return -ENOTSUP;

	/* Initialize and get modes info */
	efi_graphics_output_mode_information_t * info = NULL;
	status = gop->query_mode(gop, 0, &size_of_mode_info, &info);
	if (status != EFI_SUCCESS)
		return -ENOTSUP;
	update_current_mode(gop);
	mode_default = mode_current_base.mode;

	/* Get mode list */
	saved_modes = save_modes(gop);

	/* Set mode */
	set_mode(gop, mode_default);
	console_reset();

	/* Register device */
	// TODO: parent = efi boot services? firmware?
	err = device_create(NULL, &fb_mod, NULL, "graphics", "fb", gop_reg,
			    gop_unreg, NULL, "efigop");
	if (err)
		return err;

	return 0;
}
module_init(gop_init, early);
