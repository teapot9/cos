/**
 * @file fonts.h
 * @brief Font library
 */

#ifndef __FONTS_H
#define __FONTS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct font;

/**
 * @brief Load built-in font
 * @param dst_font Pointer where the font will be loaded
 * @return errno
 */
int font_load_default(const struct font ** dst_font);

/**
 * @brief Load a font
 * @param dst_font Pointer where the font will be loaded
 * @param file Font file start
 * @param len Font file size
 * @return errno
 */
int font_new(const struct font ** dst_font, const void * file, size_t len);

/**
 * @brief Free a font
 * @param font Font
 */
void font_free(const struct font * font);

/**
 * @brief Get bitmap of a character
 * @param dst Bitmap array
 * @param font Font
 * @param uc Unicode character code
 * @return errno
 */
int font_bitmap(bool * dst, const struct font * font, uint32_t uc);

/**
 * @brief Get font width
 * @param font Font
 * @return Width in pixels
 */
size_t font_width(const struct font * font);

/**
 * @brief Get font height
 * @param font Font
 * @return Height in pixels
 */
size_t font_height(const struct font * font);

#ifdef __cplusplus
}
#endif
#endif // __FONTS_H
