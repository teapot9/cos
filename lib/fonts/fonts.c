#define pr_fmt(fmt) "fonts: " fmt

#include <fonts.h>
#include "fonts.h"

#include <errno.h>
#include <kconfig.h>

#include <mm.h>
#if IS_ENABLED(CONFIG_FONT_PSF)
#include "psf.h"
#endif

#if IS_ENABLED(CONFIG_FONT_PSF)
#define UNUSED_IF_NO_FONT
#else
#define UNUSED_IF_NO_FONT __attribute__((unused))
#endif

#if 1
#if IS_ENABLED(CONFIG_FONT_BUILTIN_TERMINUS_8X16)
#include "font_ter_v16n.psf.h"
#endif

/* public: fonts.h */
int font_load_default(const struct font ** dst_font)
{
#if IS_ENABLED(CONFIG_FONT_BUILTIN_TERMINUS_8X16)
	return font_new(dst_font, font_ter_v16n_psf, font_ter_v16n_psf_len);
#else
	*dst_font = NULL;
	return -ENOENT;
#endif
}
#else
/* public: fonts.h */
int font_load_default(const struct font ** dst_font)
{
	font_new(dst_font, _font_builtin_data, _font_builtin_size);
}
#endif

/* public: fonts.h */
int font_new(const struct font ** dst_font,
             UNUSED_IF_NO_FONT const void * file,
             UNUSED_IF_NO_FONT size_t len)
{
	if (dst_font == NULL)
		return -EINVAL;
	*dst_font = NULL;

	int ret;
	struct font * font = malloc(sizeof(*font));
	if (font == NULL)
		return -ENOMEM;

	if (false) {
#if IS_ENABLED(CONFIG_FONT_PSF)
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

/* public: fonts.h */
void font_free(const struct font * font)
{
	switch (font->type) {
#if IS_ENABLED(CONFIG_FONT_PSF)
	case FONT_TYPE_PSF:
		return;
#endif
	default:
		return;
	}
}

/* public: fonts.h */
int font_bitmap(UNUSED_IF_NO_FONT bool * dst,
                const struct font * font,
                UNUSED_IF_NO_FONT uint32_t uc)
{
	switch (font->type) {
#if IS_ENABLED(CONFIG_FONT_PSF)
	case FONT_TYPE_PSF:
		return psf_bitmap(dst, &font->info.psf, uc);
#endif
	default:
		return -EINVAL;
	}
}

/* public: fonts.h */
size_t font_width(const struct font * font)
{
	switch (font->type) {
#if IS_ENABLED(CONFIG_FONT_PSF)
	case FONT_TYPE_PSF:
		return psf_char_width(&font->info.psf);
#endif
	default:
		return 0;
	}
}

/* public: fonts.h */
size_t font_height(const struct font * font)
{
	switch (font->type) {
#if IS_ENABLED(CONFIG_FONT_PSF)
	case FONT_TYPE_PSF:
		return psf_char_height(&font->info.psf);
#endif
	default:
		return 0;
	}
}
