#ifndef FONT_H
#define FONT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct font;

int font_load_default(const struct font ** dst_font);

int font_new(const struct font ** dst_font, const void * file, size_t len);

void font_free(const struct font * font);

int font_bitmap(bool * dst, const struct font * font, uint32_t uc);

size_t font_width(const struct font * font);

size_t font_height(const struct font * font);

#ifdef __cplusplus
}
#endif
#endif // FONT_H
