#define pr_fmt(fmt) "text: " fmt

#include <print.h>

#include <stdarg.h>
#include <stdint.h>

#include <string.h>
#include <mm.h>
#include <strtox.h>

#define STR_BUFSIZE 256

static const char base8[] = "01234567";
static const char base10[] = "0123456789";
static const char base16l[] = "0123456789abcdef";
static const char base16u[] = "0123456789ABCDEF";

static size_t uint_to_str(char * dst, uintmax_t i,
			  const char * digits, unsigned int base)
{
	char buffer[STR_BUFSIZE];
	size_t pos = 0;

	do {
		buffer[pos++] = digits[i % base];
		i /= base;
	} while (i);

	if (dst != NULL) {
		for (size_t k = 0; k < pos; k++)
			dst[k] = buffer[pos - k - 1];
	}
	return pos;
}

/* public: print.h */
size_t asprintf(char ** dst, const char * fmt, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, fmt);
	ret = vasprintf(dst, fmt, ap);
	va_end(ap);
	return ret;
}

/* public: print.h */
size_t vasprintf(char ** dst, const char * fmt, va_list ap)
{
	va_list aq;

	if (dst == NULL)
		return vsnprintf(NULL, 0, fmt, ap);

	va_copy(aq, ap);
	size_t len = vsprintf(NULL, fmt, aq) + 1;
	va_end(aq);

	*dst = malloc(len);
	if (*dst == NULL)
		return 0;

	return vsnprintf(*dst, len, fmt, ap);
}

/* public: print.h */
size_t sprintf(char * dst, const char * fmt, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, fmt);
	ret = vsprintf(dst, fmt, ap);
	va_end(ap);
	return ret;
}

/* public: print.h */
size_t snprintf(char * dst, size_t len, const char * fmt, ...)
{
	va_list ap;
	size_t ret;

	va_start(ap, fmt);
	ret = vsnprintf(dst, len, fmt, ap);
	va_end(ap);
	return ret;
}

/* public: print.h */
size_t vsprintf(char * dst, const char * fmt, va_list ap)
{
	return vsnprintf(dst, SIZE_MAX, fmt, ap);
}

#define format_integer_s(str, base_str, base) \
	do { \
		pos += strlen(str); \
		if (var < 0) { \
			if (dst != NULL) \
				dst[written] = '-'; \
			written++; \
			var = -var; \
		} \
		char * _dst = dst == NULL ? NULL : dst + written; \
		written += uint_to_str( \
			_dst, var, base_str, base \
		); \
	} while (0)

#define format_integer_u(str, base_str, base) \
	do { \
		pos += strlen(str); \
		char * _dst = dst == NULL ? NULL : dst + written; \
		written += uint_to_str( \
			_dst, var, base_str, base \
		); \
	} while (0)

/* public: print.h */
size_t vsnprintf(char * dst, size_t len, const char * fmt, va_list ap)
{
	size_t written = 0;
	size_t pos = 0;

	while (written < len - 1 && fmt[pos]) {
		if (fmt[pos] != '%') {
			if (dst != NULL)
				dst[written] = fmt[pos];
			written++;
			pos++;
			continue;
		}
		pos++;

		int precision_len = 0;
		unsigned long long precision = 0;
		if (fmt[pos] == '.') {
			pos++;
			precision_len = kstrtoull(fmt + pos, 10, &precision);
			if (precision_len > 0)
				pos += precision_len;
		}

		if (!strncmp(fmt + pos, "d", strlen("d"))) {
			int var = va_arg(ap, int);

			format_integer_s("d", base10, 10);
		} else if (!strncmp(fmt + pos, "hd", strlen("hd"))) {
			short int var = va_arg(ap, int);

			format_integer_s("hd", base10, 10);
		} else if (!strncmp(fmt + pos, "hhd", strlen("hhd"))) {
			char var = va_arg(ap, int);

			format_integer_s("hhd", base10, 10);
		} else if (!strncmp(fmt + pos, "ld", strlen("ld"))) {
			long int var = va_arg(ap, long int);

			format_integer_s("ld", base10, 10);
		} else if (!strncmp(fmt + pos, "lld", strlen("lld"))) {
			long long int var = va_arg(ap, long long int);

			format_integer_s("lld", base10, 10);
		} else if (!strncmp(fmt + pos, "jd", strlen("jd"))) {
			intmax_t var = va_arg(ap, intmax_t);

			format_integer_s("jd", base10, 10);
		} else if (!strncmp(fmt + pos, "zd", strlen("zd"))) {
			size_t var = va_arg(ap, size_t);

			format_integer_u("zd", base10, 10);
		} else if (!strncmp(fmt + pos, "td", strlen("td"))) {
			ptrdiff_t var = va_arg(ap, ptrdiff_t);

			format_integer_s("td", base10, 10);
		} else if (!strncmp(fmt + pos, "o", strlen("o"))) {
			unsigned int var = va_arg(ap, unsigned int);

			format_integer_u("o", base8, 8);
		} else if (!strncmp(fmt + pos, "ho", strlen("ho"))) {
			unsigned short int var = va_arg(ap, unsigned int);

			format_integer_u("ho", base8, 8);
		} else if (!strncmp(fmt + pos, "hho", strlen("hho"))) {
			unsigned char var = va_arg(ap, unsigned int);

			format_integer_u("hho", base8, 8);
		} else if (!strncmp(fmt + pos, "lo", strlen("lo"))) {
			unsigned long int var = va_arg(ap, unsigned long int);

			format_integer_u("lo", base8, 8);
		} else if (!strncmp(fmt + pos, "llo", strlen("llo"))) {
			unsigned long long int var = va_arg(
				ap, unsigned long long int
			);

			format_integer_u("llo", base8, 8);
		} else if (!strncmp(fmt + pos, "jo", strlen("jo"))) {
			uintmax_t var = va_arg(ap, uintmax_t);

			format_integer_u("jo", base8, 8);
		} else if (!strncmp(fmt + pos, "zo", strlen("zo"))) {
			size_t var = va_arg(ap, size_t);

			format_integer_u("zo", base8, 8);
		} else if (!strncmp(fmt + pos, "to", strlen("to"))) {
			ptrdiff_t var = va_arg(ap, ptrdiff_t);

			format_integer_u("to", base8, 8);
		} else if (!strncmp(fmt + pos, "u", strlen("u"))) {
			unsigned int var = va_arg(ap, unsigned int);

			format_integer_u("u", base10, 10);
		} else if (!strncmp(fmt + pos, "hu", strlen("hu"))) {
			unsigned short int var = va_arg(ap, unsigned int);

			format_integer_u("hu", base10, 10);
		} else if (!strncmp(fmt + pos, "hhu", strlen("hhu"))) {
			unsigned char var = va_arg(ap, unsigned int);

			format_integer_u("hhu", base10, 10);
		} else if (!strncmp(fmt + pos, "lu", strlen("lu"))) {
			unsigned long int var = va_arg(ap, unsigned long int);

			format_integer_u("lu", base10, 10);
		} else if (!strncmp(fmt + pos, "llu", strlen("llu"))) {
			unsigned long long int var = va_arg(
				ap, unsigned long long int
			);

			format_integer_u("llu", base10, 10);
		} else if (!strncmp(fmt + pos, "ju", strlen("ju"))) {
			uintmax_t var = va_arg(ap, uintmax_t);

			format_integer_u("ju", base10, 10);
		} else if (!strncmp(fmt + pos, "zu", strlen("zu"))) {
			size_t var = va_arg(ap, size_t);

			format_integer_u("zu", base10, 10);
		} else if (!strncmp(fmt + pos, "tu", strlen("tu"))) {
			ptrdiff_t var = va_arg(ap, ptrdiff_t);

			format_integer_u("tu", base10, 10);
		} else if (!strncmp(fmt + pos, "x", strlen("x"))) {
			unsigned int var = va_arg(ap, unsigned int);

			format_integer_u("x", base16l, 16);
		} else if (!strncmp(fmt + pos, "hx", strlen("hx"))) {
			unsigned short int var = va_arg(ap, unsigned int);

			format_integer_u("hx", base16l, 16);
		} else if (!strncmp(fmt + pos, "hhx", strlen("hhx"))) {
			unsigned char var = va_arg(ap, unsigned int);

			format_integer_u("hhx", base16l, 16);
		} else if (!strncmp(fmt + pos, "lx", strlen("lx"))) {
			unsigned long int var = va_arg(ap, unsigned long int);

			format_integer_u("lx", base16l, 16);
		} else if (!strncmp(fmt + pos, "llx", strlen("llx"))) {
			unsigned long long int var = va_arg(
				ap, unsigned long long int
			);

			format_integer_u("llx", base16l, 16);
		} else if (!strncmp(fmt + pos, "jx", strlen("jx"))) {
			uintmax_t var = va_arg(ap, uintmax_t);

			format_integer_u("jx", base16l, 16);
		} else if (!strncmp(fmt + pos, "zx", strlen("zx"))) {
			size_t var = va_arg(ap, size_t);

			format_integer_u("zx", base16l, 16);
		} else if (!strncmp(fmt + pos, "tx", strlen("tx"))) {
			ptrdiff_t var = va_arg(ap, ptrdiff_t);

			format_integer_u("tx", base16l, 16);
		} else if (!strncmp(fmt + pos, "X", strlen("X"))) {
			unsigned int var = va_arg(ap, unsigned int);

			format_integer_u("X", base16u, 16);
		} else if (!strncmp(fmt + pos, "hX", strlen("hX"))) {
			unsigned short int var = va_arg(ap, unsigned int);

			format_integer_u("hX", base16u, 16);
		} else if (!strncmp(fmt + pos, "hhX", strlen("hhX"))) {
			unsigned char var = va_arg(ap, unsigned int);

			format_integer_u("hhX", base16u, 16);
		} else if (!strncmp(fmt + pos, "lX", strlen("lX"))) {
			unsigned long int var = va_arg(ap, unsigned long int);

			format_integer_u("lX", base16u, 16);
		} else if (!strncmp(fmt + pos, "llX", strlen("llX"))) {
			unsigned long long int var = va_arg(
				ap, unsigned long long int
			);

			format_integer_u("llX", base16u, 16);
		} else if (!strncmp(fmt + pos, "jX", strlen("jX"))) {
			uintmax_t var = va_arg(ap, uintmax_t);

			format_integer_u("jX", base16u, 16);
		} else if (!strncmp(fmt + pos, "zX", strlen("zX"))) {
			size_t var = va_arg(ap, size_t);

			format_integer_u("zX", base16u, 16);
		} else if (!strncmp(fmt + pos, "tX", strlen("tX"))) {
			ptrdiff_t var = va_arg(ap, ptrdiff_t);

			format_integer_u("tX", base16u, 16);
		} else if (!strncmp(fmt + pos, "c", strlen("c"))) {
			unsigned char var = va_arg(ap, int);

			pos += strlen("c");
			if (dst != NULL)
				dst[written] = var;
			written++;
		} else if (!strncmp(fmt + pos, "s", strlen("s"))) {
			const char * var = va_arg(ap, const char *);
			size_t max = len - written;
			if (precision_len > 0 && precision < max)
				max = precision + 1;

			pos += strlen("s");
			written += strncpy(dst + written, var, max);
		} else if (!strncmp(fmt + pos, "p", strlen("p"))) {
			uintmax_t var = (uintmax_t) va_arg(ap, void *);

			if (dst != NULL) {
				dst[written] = '0';
				dst[written + 1] = 'x';
			}
			written += 2;
			// format_integer_u("s", base16l, 16);
			pos += strlen("s");
			char * _dst = dst == NULL ? NULL : dst + written;
			written += uint_to_str(
				_dst, var, base16l, 16
			);
		} else if (!strncmp(fmt + pos, "%", strlen("%"))) {
			pos += strlen("%");
			if (dst != NULL)
				dst[written] = '%';
			written++;
		} else {
			if (dst != NULL)
				dst[written] = '%';
			written++;
		}
	}

	if (dst != NULL && written < len)
		dst[written] = 0;
	return written;
}
