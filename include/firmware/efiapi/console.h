#ifndef FIRMWARE_EFIAPI_CONSOLE_H
#define FIRMWARE_EFIAPI_CONSOLE_H

#include <firmware/efiapi/boot.h>
#include <firmware/efiapi/efiapi.h>

#define EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID \
	{0x387477c1, 0x69c7, 0x11d2, \
	 {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

struct _efi_simple_text_input_protocol;
typedef struct _efi_simple_text_input_protocol efi_simple_text_input_protocol_t;

typedef struct {
	uint16_t scan_code;
	uint16_t unicode_char;
} efi_input_key_t;

typedef efi_status_t (EFIABI * efi_input_reset_t) (
	efi_simple_text_input_protocol_t * this,
	efi_bool extended_verification
);

typedef efi_status_t (EFIABI * efi_input_read_key_t) (
	efi_simple_text_input_protocol_t * this,
	efi_input_key_t * key
);

struct _efi_simple_text_input_protocol {
	efi_input_reset_t reset;
	efi_input_read_key_t read_key_stroke;
	efi_event_t wait_for_key;
};

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
	{0x387477c2, 0x69c7, 0x11d2, \
	 {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

struct _efi_simple_text_output_protocol;
typedef struct _efi_simple_text_output_protocol
	efi_simple_text_output_protocol_t;

typedef struct {
	int32_t max_mode;
	int32_t mode;
	int32_t attribute;
	int32_t cursor_column;
	int32_t cursor_row;
	efi_bool cursor_visible;
} efi_simple_text_output_mode;

typedef efi_status_t (EFIABI * efi_text_reset_t) (
	efi_simple_text_output_protocol_t * this,
	efi_bool extended_verification
);

typedef efi_status_t (EFIABI * efi_text_string_t) (
	efi_simple_text_output_protocol_t * this,
	uint16_t * string
);

typedef efi_status_t (EFIABI * efi_text_test_string_t) (
	efi_simple_text_input_protocol_t * this,
	uint16_t * string
);

typedef efi_status_t (EFIABI * efi_text_query_mode_t) (
	efi_simple_text_output_protocol_t * this,
	efi_uintn mode_number,
	efi_uintn * columns,
	efi_uintn * rows
);

typedef efi_status_t (EFIABI * efi_text_set_mode_t) (
	efi_simple_text_output_protocol_t * this,
	efi_uintn mode_number
);

#define EFI_BLACK 0x00
#define EFI_BLUE 0x01
#define EFI_GREEN 0x02
#define EFI_CYAN (EFI_BLUE | EFI_GREEN)
#define EFI_RED 0x04
#define EFI_MAGENTA (EFI_BLUE | EFI_RED)
#define EFI_BROWN (EFI_RED | EFI_GREEN)
#define EFI_LIGHTGRAY (EFI_RED | EFI_GREEN | EFI_BLUE)
#define EFI_BRIGHT 0x08
#define EFI_DARKGRAY (EFI_BLACK | EFI_BRIGHT)
#define EFI_LIGHTBLUE (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE (EFI_LIGHTGRAY | EFI_BRIGHT)
#define EFI_BACKGROUND_BLACK (EFI_BLACK << 4)
#define EFI_BACKGROUND_BLUE (EFI_BLUE << 4)
#define EFI_BACKGROUND_GREEN (EFI_GREEN << 4)
#define EFI_BACKGROUND_CYAN (EFI_CYAN << 4)
#define EFI_BACKGROUND_RED (EFI_RED << 4)
#define EFI_BACKGROUND_MAGENTA (EFI_MAGENTA << 4)
#define EFI_BACKGROUND_BROWN (EFI_BROWN << 4)
#define EFI_BACKGROUND_LIGHTGRAY (EFI_LIGHTGRAY << 4)
#define EFI_TEXT_ATTR(foreground, background) \
	((foreground) | ((background) << 4))

typedef efi_status_t (EFIABI * efi_text_set_attribute_t) (
	efi_simple_text_output_protocol_t * this,
	efi_uintn attribute
);

typedef efi_status_t (EFIABI * efi_text_clear_screen_t) (
	efi_simple_text_output_protocol_t * this
);

typedef efi_status_t (EFIABI * efi_text_set_cursor_position_t) (
	efi_simple_text_output_protocol_t * this,
	efi_uintn column,
	efi_uintn row
);

typedef efi_status_t (EFIABI * efi_text_enable_cursor_t) (
	efi_simple_text_output_protocol_t * this,
	efi_bool visible
);

struct _efi_simple_text_output_protocol {
	efi_text_reset_t reset;
	efi_text_string_t output_string;
	efi_text_test_string_t test_string;
	efi_text_query_mode_t query_mode;
	efi_text_set_mode_t set_mode;
	efi_text_set_attribute_t set_attribute;
	efi_text_clear_screen_t clear_screen;
	efi_text_set_cursor_position_t set_cursor_position;
	efi_text_enable_cursor_t enable_cursor;
	efi_simple_text_output_mode * mode;
};

/* GOP: graphics output protocol */

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
	{0x9042a9de, 0x23dc, 0x4a38, \
	 {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}

struct _efi_graphics_output_protocol;
typedef struct _efi_graphics_output_protocol efi_graphics_output_protocol_t;

typedef struct {
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t _reserved;
} efi_pixel_bitmask_t;

typedef struct {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t _reserved;
} efi_graphics_output_blt_pixel_t;

typedef enum {
	EFI_BLT_VIDEO_FILL,
	EFI_BLT_VIDEO_TO_BLT_BUFFER,
	EFI_BLT_BUFFER_TO_VIDEO,
	EFI_BLT_VIDEO_TO_VIDEO,
	EFI_GRAPHICS_OUTPUT_BLT_OPERATION_MAX,
} efi_graphics_output_blt_operation_t;

typedef enum {
	EFI_PIXEL_RED_GREEN_BLUE_RESERVED_8BIT_PER_COLOR,
	EFI_PIXEL_BLUE_GREEN_RED_RESERVED_8BIT_PER_COLOR,
	EFI_PIXEL_BIT_MASK,
	EFI_PIXEL_BLT_ONLY,
	EFI_PIXEL_FORMAT_MAX,
} efi_graphics_pixel_format_t;

typedef struct {
	uint32_t version;
	uint32_t horizontal_resolution;
	uint32_t vertical_resolution;
	efi_graphics_pixel_format_t pixel_format;
	efi_pixel_bitmask_t pixel_information;
	uint32_t pixels_per_scanline;
} efi_graphics_output_mode_information_t;

typedef struct {
	uint32_t max_mode;
	uint32_t mode;
	efi_graphics_output_mode_information_t * info;
	efi_uintn size_of_info;
	efi_physical_address_t frame_buffer_base;
	efi_uintn frame_buffer_size;
} efi_graphics_output_protocol_mode_t;

typedef efi_status_t (EFIABI * efi_graphics_output_protocol_query_mode_t) (
	efi_graphics_output_protocol_t * this,
	uint32_t mode_number,
	efi_uintn * size_of_info,
	efi_graphics_output_mode_information_t ** info
);

typedef efi_status_t (EFIABI * efi_graphics_output_protocol_set_mode_t) (
	efi_graphics_output_protocol_t * this,
	uint32_t mode_number
);

typedef efi_status_t (EFIABI * efi_graphics_output_protocol_blt_t) (
	efi_graphics_output_protocol_t * this,
	efi_graphics_output_blt_pixel_t * blt_buffer,
	efi_graphics_output_blt_operation_t blt_operation,
	efi_uintn source_x,
	efi_uintn source_y,
	efi_uintn destination_x,
	efi_uintn destination_y,
	efi_uintn width,
	efi_uintn height,
	efi_uintn delta
);

struct _efi_graphics_output_protocol {
	efi_graphics_output_protocol_query_mode_t query_mode;
	efi_graphics_output_protocol_set_mode_t set_mode;
	efi_graphics_output_protocol_blt_t blt;
	efi_graphics_output_protocol_mode_t * mode;
};

#endif // FIRMWARE_EFIAPI_CONSOLE_H
