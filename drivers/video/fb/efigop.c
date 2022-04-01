#define pr_fmt(fmt) "efigop: " fmt

#include "efigop.h"

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

static bool efigop_is_available = false;
static bool efigop_is_console = false;

// Modes information
static unsigned int mode_default = 0;
static size_t size_of_mode_info = 0;
static efi_graphics_output_protocol_mode_t mode_current_base = {0};
static efi_graphics_output_mode_information_t mode_current_info = {0};
static const efi_graphics_output_mode_information_t * modes = NULL;

// Framebuffer and console information
static const struct fb * efigop_fb = NULL;
static struct fbcon * efigop_con = NULL;

static efi_graphics_output_mode_information_t * efigop_save_modes(
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
			continue;
		}

		*modeptr = *info;
	}

	return modes;
}

static void efigop_print_current_mode(void)
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

static inline void efigop_update_current_mode(
	efi_graphics_output_protocol_t * gop
)
{
	mode_current_base = *gop->mode;
	mode_current_info = *gop->mode->info;
	mode_current_base.info = &mode_current_info;
}

static int efigop_set_mode(efi_graphics_output_protocol_t * gop, int mode)
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
		efigop_update_current_mode(gop);
	}

	status = gop->blt(gop, &black, EFI_BLT_VIDEO_FILL, 0, 0, 0, 0,
			  mode_current_info.horizontal_resolution,
			  mode_current_info.vertical_resolution, 0);
	if (status != EFI_SUCCESS) {
		pr_err("Failed to clear the screen (error %d)\n", status);
		ret = -1;
	}

	efigop_print_current_mode();
	return ret;
}

int efigop_init(void)
{
	efi_status_t status;
	int ret;
	efi_graphics_output_protocol_t * gop = NULL;
	efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	bool enable_efigop_console = false;

	efistub_init();
	if (efistub_system_table == NULL)
		return -EINVAL;

	/* Check support */
	status = efistub_system_table->boot_services->locate_protocol(
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
	efigop_update_current_mode(gop);
	mode_default = mode_current_base.mode;

	/* Get mode list */
	modes = efigop_save_modes(gop);

	/* Disable efi console */
	if (efistub_console_is_init) {
		enable_efigop_console = true;
		efistub_console_disable();
	}

	/* Set mode */
	efigop_set_mode(gop, mode_default);

	/* Register efigop */
	struct fb efigop;
	efigop.fb_base = (void *) mode_current_base.frame_buffer_base;
	efigop.fb_size = mode_current_base.frame_buffer_size;
	efigop.bytes_per_pixel = 4;
	efigop.bytes_per_scanline = 4 * mode_current_info.pixels_per_scanline;
	switch (mode_current_info.pixel_format) {
	case EFI_PIXEL_RED_GREEN_BLUE_RESERVED_8BIT_PER_COLOR:
		efigop.pixel_format = PIXEL_FORMAT_32RGB;
		break;
	case EFI_PIXEL_BLUE_GREEN_RED_RESERVED_8BIT_PER_COLOR:
		efigop.pixel_format = PIXEL_FORMAT_32BGR;
		break;
	case EFI_PIXEL_BIT_MASK:
		efigop.pixel_format = PIXEL_FORMAT_32MASK;
		efigop.pixel_mask.m32.r =
			mode_current_info.pixel_information.red_mask;
		efigop.pixel_mask.m32.g =
			mode_current_info.pixel_information.green_mask;
		efigop.pixel_mask.m32.b =
			mode_current_info.pixel_information.blue_mask;
		break;
	default:
		pr_crit("Unsupported mode for fb driver\n", 0);
	}
	if ((ret = fb_add(&efigop_fb, efigop)))
		pr_err("Could not register efigop framebuffer (error %d)\n",
		       ret);

	/* Register console */
	if (enable_efigop_console) {
		if ((ret = fbcon_add(&efigop_con, efigop_fb)))
			pr_err("Could not register efigop as console (error %d)"
			       "\n", ret);
		else
			efigop_is_console = true;
	}

	pr_debug("Initialized: %d modes, current mode: %d\n",
	         mode_current_base.max_mode, mode_current_base.mode);
	efigop_is_available = true;
	return 0;
}

void efigop_disable_console(void)
{
	console_remove(efigop_con);
	efigop_is_console = false;
}

void efigop_disable(void)
{
	efigop_disable_console();
	fb_remove(efigop_fb);
	efigop_is_available = false;
}
