#include <fonts.h>
#include "fonts.h"

#include <errno.h>

#include <mm.h>
#ifdef CONFIG_FONT_PSF
#include "psf.h"
#endif

#if defined(CONFIG_FONT_PSF)
#define UNUSED_IF_NO_FONT
#else
#define UNUSED_IF_NO_FONT __attribute__((unused))
#endif

#ifdef CONFIG_FONT_BUILTIN_TERMINUS_8X16
#include "font_ter_v16n.psf.h"
#endif

int font_load_default(const struct font ** dst_font)
{
#if defined(CONFIG_FONT_BUILTIN_TERMINUS_8X16)
	return font_new(dst_font, font_ter_v16n_psf, font_ter_v16n_psf_len);
#else
	if (dst_font != NULL)
		*dst_font = NULL;
	return -ENOENT;
#endif
}

int font_new(const struct font ** dst_font,
             UNUSED_IF_NO_FONT const void * file,
             UNUSED_IF_NO_FONT size_t len)
{
	if (dst_font == NULL)
		return -EINVAL;
	*dst_font = NULL;

	int ret;
	struct font * font = kmalloc(sizeof(*font));
	if (font == NULL)
		return -ENOMEM;

	if (false) {
#ifdef CONFIG_FONT_PSF
	} else if (psf_test_magic(file, len)) {
		font->type = FONT_TYPE_PSF;
		if ((ret = psf_font_new(&font->info.psf, file, len)))
			goto return_ret_free_font;
#endif
	} else {
		ret = -EINVAL;
		goto return_ret_free_font;
	}

	*dst_font = font;
	return 0;

return_ret_free_font:
	kfree(font);
	return ret;
}

void font_free(const struct font * font)
{
	switch (font->type) {
#ifdef CONFIG_FONT_PSF
	case FONT_TYPE_PSF:
		return;
#endif
	default:
		return;
	}
}

int font_bitmap(UNUSED_IF_NO_FONT bool * dst,
                const struct font * font,
                UNUSED_IF_NO_FONT uint32_t uc)
{
	switch (font->type) {
#ifdef CONFIG_FONT_PSF
	case FONT_TYPE_PSF:
		return psf_bitmap(dst, &font->info.psf, uc);
#endif
	default:
		return -EINVAL;
	}
}

size_t font_width(const struct font * font)
{
	switch (font->type) {
#ifdef CONFIG_FONT_PSF
	case FONT_TYPE_PSF:
		return psf_char_width(&font->info.psf);
#endif
	default:
		return 0;
	}
}

size_t font_height(const struct font * font)
{
	switch (font->type) {
#ifdef CONFIG_FONT_PSF
	case FONT_TYPE_PSF:
		return psf_char_height(&font->info.psf);
#endif
	default:
		return 0;
	}
}
