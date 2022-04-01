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
#include <print.h>

static void print_current_mode(struct gop * gop)
{
	pr_debug(
		"Current mode %d: width %d, height %d, "
		"pixel format 0x%x, "
		"pixels per scanline %d, framebuffer base 0x%x, "
		"framebuffer size %d\n",
		gop->mode.mode,
		gop->mode.info->horizontal_resolution,
		gop->mode.info->vertical_resolution,
		gop->mode.info->pixel_format,
		gop->mode.info->pixels_per_scanline,
		gop->mode.frame_buffer_base,
		gop->mode.frame_buffer_size
	);
}

static inline void update_current_mode(
	struct gop * gop, efi_graphics_output_protocol_t * gop_prot
)
{
	gop->mode = *gop_prot->mode;
	gop->info = *gop_prot->mode->info;
	gop->mode.info = &gop->info;
}

static int set_mode(
	struct gop * gop, efi_graphics_output_protocol_t * gop_prot, int mode
)
{
	int ret = 0;
	efi_status_t status;
	efi_graphics_output_blt_pixel_t black = {0, 0, 0, 0};

	pr_debug("Setting mode to %d\n", mode);

	status = gop_prot->set_mode(gop_prot, mode);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to set mode to %d (error %d)\n", mode, status);
		ret = -1;
	} else {
		update_current_mode(gop, gop_prot);
	}

	status = gop_prot->blt(gop_prot, &black, EFI_BLT_VIDEO_FILL, 0, 0, 0, 0,
			  gop->mode.info->horizontal_resolution,
			  gop->mode.info->vertical_resolution, 0);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to clear the screen (error %d)\n", status);
		ret = -1;
	}

	print_current_mode(gop);
	return ret;
}

/* public: platform_setup.h */
int gop_init(struct efigop_bdata ** data)
{
	efi_status_t status;
	efi_graphics_output_protocol_t * gop_prot = NULL;
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

	if (data == NULL)
		return -EINVAL;
	*data = kmalloc(sizeof(**data));
	if (*data == NULL)
		return -ENOMEM;
	struct gop * gop = &(*data)->gop;

	/* Check support */
	status = efistub_system_table()->boot_services->locate_protocol(
		&gop_guid, NULL, (void **) &gop
	);
	if (status != EFI_SUCCESS || gop_prot == NULL) {
		kfree(*data);
		*data = NULL;
		return -ENOTSUP;
	}

	/* Initialize and get modes info */
	efi_graphics_output_mode_information_t * info = NULL;
	status = gop_prot->query_mode(gop_prot, 0, &gop->mode_info_size, &info);
	if (status != EFI_SUCCESS) {
		kfree(*data);
		*data = NULL;
		return -ENOTSUP;
	}
	update_current_mode(gop, gop_prot);
	gop->default_mode = gop->mode.mode;

	/* Set mode */
	set_mode(gop, gop_prot, gop->default_mode);
	console_reset();

	return 0;
}
