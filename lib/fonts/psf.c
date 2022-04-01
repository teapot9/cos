#include "psf.h"

#include <errno.h>
#include <stdbool.h>

#include <mm.h>
#include <unicode.h>

static inline bool psf_eof(const struct psf_font * font,
                           const void * ptr, size_t len)
{
	return !(
		ptr >= (const void *) font->header
		&& (uint8_t *) ptr + len
		< (uint8_t *) font->header + font->size
	);
}

static inline bool psf_has_unicode(const struct psf_font * font)
{
	if (font->version == PSF_VERSION_1)
		return font->header->psf1.has_unicode_table;
	else if (font->version == PSF_VERSION_2)
		return font->header->psf2.has_unicode_table;
	else
		return 0;
}

static inline bool psf_unicode_is_term(const struct psf_font * font,
				       const uint8_t * seq)
{
	if (font->version == PSF_VERSION_1 && !psf_eof(font, seq, 1))
		return *((uint16_t *) seq) == PSF1_SEPARATOR;
	else if (font->version == PSF_VERSION_2 && !psf_eof(font, seq, 0))
		return *seq == PSF2_SEPARATOR;
	else
		return true;
}

static inline bool psf_unicode_is_seq(const struct psf_font * font,
				      const uint8_t * seq)
{
	if (font->version == PSF_VERSION_1 && !psf_eof(font, seq, 1))
		return *((uint16_t *) seq) == PSF1_STARTSEQ;
	else if (font->version == PSF_VERSION_2 && !psf_eof(font, seq, 0))
		return *seq == PSF2_STARTSEQ;
	else
		return false;
}

static int psf_set_glyph(bool * dst, const struct psf_font * font,
			 uint32_t glyph)
{
	uint8_t * bitmap = ((uint8_t *) font->header);
	size_t nb_bits;

	if (font->version == PSF_VERSION_1) {
		bitmap += sizeof(font->header->psf1)
			+ glyph * font->header->psf1.charsize;
		if (psf_eof(font, bitmap, font->header->psf1.charsize))
			return -EINVAL;
		nb_bits = 8 * font->header->psf1.charsize;
	} else if (font->version == PSF_VERSION_2) {
		bitmap += font->header->psf2.headersize
			+ glyph * font->header->psf2.charsize;
		if (psf_eof(font, bitmap, font->header->psf2.charsize))
			return -EINVAL;
		nb_bits = font->header->psf2.height * font->header->psf2.width;
	} else {
		return -EINVAL;
	}

	for (size_t i = 0; i < nb_bits; i++) {
		if (bitmap[i / 8] & (1 << (7 - i % 8)))
			dst[i] = true;
	}
	return 0;
}

static size_t psf_unicode_get_uc(const struct psf_font * font,
                                 const uint8_t * seq, uint32_t * out)
{
	if (font->version == PSF_VERSION_1) {
		if (psf_eof(font, seq, 1))
			return -EACCES;
		if (out != NULL)
			*out = *((uint16_t *) seq);
		return 2;
	} else if (font->version == PSF_VERSION_2) {
		uint32_t uc;
		size_t read = utf8_to_unicode(seq, &uc);

		if (psf_eof(font, seq, read))
			return -EINVAL;
		if (out != NULL)
			*out = uc;
		return read;
	} else {
		return -EINVAL;
	}
}

static const uint8_t * psf_unicode_find_seq(const struct psf_font * font,
					    uint32_t uc)
{
	uint8_t * seq = ((uint8_t *) font->header);
	uint32_t current;

	if (font->version == PSF_VERSION_1) {
		seq += sizeof(font->header->psf1);
		if (font->header->psf1.mode_512)
			seq += 512 * font->header->psf1.charsize;
		else
			seq += 256 * font->header->psf1.charsize;
	} else if (font->version == PSF_VERSION_2) {
		seq += font->header->psf2.headersize
			+ font->header->psf2.length
			* font->header->psf2.charsize;
	} else {
		return NULL;
	}
	if (psf_eof(font, seq, 0))
		return NULL;

	seq += psf_unicode_get_uc(font, seq, &current);
	while (!psf_eof(font, seq, 0) && current != uc) {
		if (psf_unicode_is_seq(font, seq)) {
			while (!psf_unicode_is_term(font, seq))
				seq += psf_unicode_get_uc(font, seq, NULL);
		}
		if (psf_unicode_is_term(font, seq))
			seq += psf_unicode_get_uc(font, seq, NULL);
		seq += psf_unicode_get_uc(font, seq, &current);
	}

	while (!psf_eof(font, seq, 0)
	       && !psf_unicode_is_seq(font, seq)
	       && !psf_unicode_is_term(font, seq)) {
		seq += psf_unicode_get_uc(font, seq, NULL);
	}

	if (psf_eof(font, seq, 0) || current != uc)
		return NULL;
	return seq;
}

static void psf_init_dest(bool * dst, const struct psf_font * font)
{
	size_t nb_bits;

	if (font->version == PSF_VERSION_1)
		nb_bits = 8 * font->header->psf1.charsize;
	else if (font->version == PSF_VERSION_2)
		nb_bits = font->header->psf2.height * font->header->psf2.width;
	else
		nb_bits = 0;

	for (size_t i = 0; i < nb_bits; i++)
		dst[i] = false;
}

int psf_bitmap(bool * dst, const struct psf_font * font, uint32_t uc)
{
	int ret;

	psf_init_dest(dst, font);

	if (psf_has_unicode(font)) {
		const uint8_t * seq = psf_unicode_find_seq(font, uc);
		if (seq == NULL || psf_eof(font, seq, 0))
			return -EINVAL;

		if (psf_unicode_is_term(font, seq)) {
			return psf_set_glyph(dst, font, uc);
		} else if (psf_unicode_is_seq(font, seq)) {
			seq += psf_unicode_get_uc(font, seq, NULL);
			while (!psf_eof(font, seq, 0)
			       && !psf_unicode_is_term(font, seq)) {
				seq += psf_unicode_get_uc(font, seq, &uc);
				if ((ret = psf_set_glyph(dst, font, uc)))
					return ret;
			}
		} else {
			return -EINVAL;
		}
	} else {
		return psf_set_glyph(dst, font, uc);
	}
	return 0;
}

int psf_font_new(struct psf_font * font, const void * file, size_t len)
{
	if (file == NULL || font == NULL)
		return -EINVAL;

	font->size = len;
	font->header = file;
	if (len >= 2 && font->header->psf1.magic == PSF1_MAGIC) {
		font->version = PSF_VERSION_1;
		if (len <= sizeof(font->header->psf1))
			goto return_error;
	} else if (len >= 4 && font->header->psf2.magic == PSF2_MAGIC) {
		font->version = PSF_VERSION_2;
		if (len <= sizeof(font->header->psf2))
			goto return_error;
	} else {
		goto return_error;
	}

	return 0;

return_error:
	font->size = 0;
	font->header = NULL;
	return -EINVAL;
}
