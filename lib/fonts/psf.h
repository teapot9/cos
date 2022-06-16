#ifndef KERNEL_FONTS_PSF_H
#define KERNEL_FONTS_PSF_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cpp.h>

#define PSF1_MAGIC 0x0436
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define header_is_psf1(ptr) ((ptr)[0] == PSF1_MAGIC0 && (ptr)[1] == PSF1_MAGIC1)

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

struct _packed_ psf1_header {
	uint16_t magic;
	// uint8_t mode;
	bool mode_has_seq : 1;
	bool has_unicode_table : 1;
	bool mode_512 : 1;
	unsigned int _unused : 8 - 3;
	uint8_t charsize;     /* Character size */
};

static_assert(sizeof(struct psf1_header) == 4,
              "struct psf1_header must be 4 bytes");

#define PSF2_MAGIC 0x864AB572
#define PSF2_MAGIC0     0x72
#define PSF2_MAGIC1     0xb5
#define PSF2_MAGIC2     0x4a
#define PSF2_MAGIC3     0x86

#define header_is_psf2(ptr) ( \
	(ptr)[0] == PSF2_MAGIC0 && (ptr)[1] == PSF2_MAGIC1 \
	&& (ptr)[2] == PSF2_MAGIC2 && (ptr)[3] == PSF2_MAGIC3 \
	)

/* bits used in flags */
#define PSF2_HAS_UNICODE_TABLE 0x01

/* max version recognized so far */
#define PSF2_MAXVERSION	0

/* UTF8 separators */
#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

struct _packed_ psf2_header {
	uint32_t magic;
	uint32_t version;
	uint32_t headersize;    /* offset of bitmaps in file */
	// uint32_t flags;
	bool has_unicode_table : 1;
	unsigned int _unused : 32 - 1;
	uint32_t length;        /* number of glyphs */
	uint32_t charsize;      /* number of bytes for each character */
	uint32_t height, width; /* max dimensions of glyphs */
	/* charsize = height * ((width + 7) / 8) */
};

static_assert(sizeof(struct psf2_header) == 32,
              "struct psf2_header must be 32 bytes");

struct psf_font {
	enum psf_version {
		PSF_VERSION_1, PSF_VERSION_2,
	} version;
	const union psf_header {
		const struct psf1_header psf1;
		const struct psf2_header psf2;
	} * header;
	size_t size;
};

#define psf_font_free(font) ()

int psf_font_new(struct psf_font * font, const void * file, size_t len);

int psf_bitmap(bool * dst, const struct psf_font * font, uint32_t uc);

static inline size_t psf_char_width(const struct psf_font * font)
{
	switch (font->version) {
	case PSF_VERSION_1:
		return 8;
	case PSF_VERSION_2:
		return font->header->psf2.width;
	default:
		return 0;
	}
}

static inline size_t psf_char_height(const struct psf_font * font)
{
	switch (font->version) {
	case PSF_VERSION_1:
		return font->header->psf1.charsize;
	case PSF_VERSION_2:
		return font->header->psf2.height;
	default:
		return 0;
	}
}

static inline bool psf_test_magic(const void * file, size_t len)
{
	return (
		(len >= 2 && *((uint16_t *) file) == PSF1_MAGIC)
		|| (len >= 4 && *((uint32_t *) file) == PSF2_MAGIC)
	);
}

#endif // KERNEL_FONTS_PSF_H
