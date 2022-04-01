#include <unicode.h>

size_t utf_to_unicode(const char * src, uint32_t * code)
{
	if (sizeof(char) == sizeof(uint8_t))
		return utf8_to_unicode((const uint8_t *) src, code);
	return 0;
}

size_t utf8_to_unicode(const uint8_t * src, uint32_t * code)
{
	if ((src[0] & 0x80) == 0x00) {
		*code = src[0];
		return 1;
	} else if ((src[0] & 0xE0) == 0xC0
	           && (src[1] & 0xC0) == 0x80) {
		*code = (src[0] & 0x1F) | (src[1] & 0x3F);
		return 2;
	} else if ((src[0] & 0xF0) == 0xE0
	           && (src[1] & 0xC0) == 0x80
	           && (src[2] & 0xC0) == 0x80) {
		*code = (src[0] & 0x0F) | (src[1] & 0x3F) | (src[2] & 0x3F);
		return 3;
	} else if ((src[0] & 0xF8) == 0xF0
		   && (src[1] & 0xC0) == 0x80
		   && (src[2] & 0xC0) == 0x80
		   && (src[3] & 0xC0) == 0x80) {
		*code = (src[0] & 0x7) | (src[1] & 0x3F)
			| (src[2] & 0x3F) | (src[3] & 0x3F);
		return 4;
	} else {
		return 0;
	}
}

size_t unicode_to_utf16(uint16_t * dst, uint32_t code, size_t len)
{
	uint16_t tmp;

	if (len >= 1 && (code <= 0xD7FF || (code >= 0xE000 && code <= 0xFFFF))) {
		dst[0] = code;
		return 1;
	} else if (len >= 2 && code >= 0x010000 && code <= 0x10FFFF) {
		tmp = code - 0x10000;
		dst[0] = 0xD800 | (tmp & 0x003FF);
		dst[1] = 0xDC00 | (tmp & 0xFFC00);
		return 2;
	} else {
		return 0;
	}
}

size_t utf_to_utf16(uint16_t * dst, const char * src, size_t len)
{
	if (sizeof(char) == sizeof(uint8_t))
		return utf8_to_utf16(dst, (const uint8_t *) src, len);
	return 0;
}

size_t utf8_to_utf16(uint16_t * dst, const uint8_t * src, size_t len)
{
	return utf8_to_utf16_eol(dst, src, len, NULL);
}

size_t utf_to_utf16_eol(uint16_t * dst, const char * src, size_t len, uint16_t * eol)
{
	if (sizeof(char) == sizeof(uint8_t))
		return utf8_to_utf16_eol(dst, (const uint8_t *) src, len, eol);
	return 0;
}

size_t utf8_to_utf16_eol(uint16_t * dst, const uint8_t * src, size_t len, uint16_t * eol)
{
	size_t written = 0;
	size_t read = 0;
	size_t size = 0;
	uint32_t unicode = 1;

	while (written < len - 1 && unicode) {
		size = utf8_to_unicode(src, &unicode);
		if (!size)
			return 0;
		src += size;
		if (unicode)
			read += size;

		if (unicode == '\n' && eol != NULL)
			size = utf16_copy(dst, eol, len - written);
		else
			size = unicode_to_utf16(dst, unicode, len - written);
		if (!size)
			return 0;
		dst += size;
		written += size;
	}

	if (written < len)
		*dst = 0;
	return read;
}

size_t utf16_to_unicode(const uint16_t * src, uint32_t * code)
{
	if (src[0] < 0xD7FF || src[0] >= 0xE000) {
		*code = src[0];
		return 1;
	} else if ((src[0] & 0xFC00) == 0xD800 && (src[1] & 0xFC00) == 0xDC00) {
		*code = ((src[0] & 0x03FF) << 16) | (src[1] & 0x03FF);
		return 2;
	} else {
		return 0;
	}
}

size_t unicode_to_utf8(uint8_t * dst, uint32_t code, size_t len)
{
	if (len >= 1 && code <= 0x007F) {
		dst[0] = code;
		return 1;
	} else if (len >= 2 && code >= 0x0080 && code <= 0x07FF) {
		dst[0] = 0xC0 | ((code & 0x07C0) >> 6);
		dst[1] = 0x80 | ((code & 0x003F) >> 0);
		return 2;
	} else if (len >= 3 && code >= 0x0800 && code <= 0xFFFF) {
		dst[0] = 0xE0 | ((code & 0xF000) >> 12);
		dst[1] = 0x80 | ((code & 0x0FC0) >> 6);
		dst[2] = 0x80 | ((code & 0x003F) >> 0);
		return 3;
	} else if (len >= 4 && code >= 0x10000 && code <= 0x10FFFF) {
		dst[0] = 0xF0 | ((code & 0x1C0000) >> 18);
		dst[1] = 0x80 | ((code & 0x03F000) >> 12);
		dst[2] = 0x80 | ((code & 0x000FC0) >> 6);
		dst[3] = 0x80 | ((code & 0x00003F) >> 0);
		return 4;
	} else {
		return 0;
	}
}

size_t utf16_to_utf8(uint8_t * dst, const uint16_t * src, size_t len)
{
	size_t written = 0;
	size_t read = 0;
	size_t size = 0;
	uint32_t unicode = 1;

	while (written * sizeof(*dst) < len - sizeof(*dst) && unicode) {
		size = utf16_to_unicode(src, &unicode);
		if (!size)
			return 0;
		src += size;
		if (unicode)
			read += size;

		size = unicode_to_utf8(dst, unicode,
		                       len - written * sizeof(*dst));
		if (!size)
			return 0;
		dst += size;
		written += size;
	}

	if (written * sizeof(*dst) <= len - sizeof(*dst))
		*dst = 0;
	return read;
}

size_t utf16_to_utf(char * dst, const uint16_t * src, size_t len)
{
	if (sizeof(char) == sizeof(uint8_t))
		return utf16_to_utf8((uint8_t *) dst, src, len);
	return 0;
}

size_t utf16_copy(uint16_t * dst, const uint16_t * src, size_t len)
{
	size_t copied = 0;

	while (copied < len - 1 && src[copied]) {
		dst[copied] = src[copied];
		copied++;
	}
	if (copied < len)
		dst[copied] = 0;
	return copied;
}
