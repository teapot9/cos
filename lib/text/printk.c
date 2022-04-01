#define pr_fmt(fmt) "text: " fmt

#include <print.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <console.h>
#include <string.h>

#define STR_BUFSIZE 128
#define KMSG_BUFSIZE 1024

static char printk_kmsg[KMSG_BUFSIZE];
static size_t printk_kmsg_start = 0;
static size_t printk_kmsg_end = 0;

/* public: print.h */
size_t printk(const char * fmt, ...)
{
	va_list ap;
	size_t ret = 0;

	va_start(ap, fmt);
	ret = vprintk(fmt, ap);
	va_end(ap);
	return ret;
}

#define kmsg_pos_add(written) \
	do { \
		size_t start1 = printk_kmsg_start; \
		size_t end1 = printk_kmsg_end; \
		bool has_cycled = false; \
		\
		printk_kmsg_end += written; \
		if (printk_kmsg_end >= KMSG_BUFSIZE) { \
			printk_kmsg_end = printk_kmsg_end - KMSG_BUFSIZE; \
			has_cycled = true; \
		} \
		if ((start1 < end1 && printk_kmsg_start < printk_kmsg_end \
		     && has_cycled) \
		    || (start1 > end1 && printk_kmsg_start < printk_kmsg_end) \
		    || (start1 > end1 && printk_kmsg_start > printk_kmsg_end \
			&& has_cycled) \
		    || (printk_kmsg_start == printk_kmsg_end)) \
			printk_kmsg_start = printk_kmsg_end + 1; \
		if (printk_kmsg_start >= KMSG_BUFSIZE) \
			printk_kmsg_start = printk_kmsg_start - KMSG_BUFSIZE; \
	} while (0)

/* public: print.h */
size_t vprintk(const char * fmt, va_list ap)
{
	const char * fmt_msg = NULL;
	char buffer[STR_BUFSIZE];
	size_t ret = 0;

	if (fmt[0] == '<' && fmt[2] == '>') {
		printk_kmsg[printk_kmsg_end] = fmt[0];
		kmsg_pos_add(1);
		printk_kmsg[printk_kmsg_end] = fmt[1];
		kmsg_pos_add(1);
		printk_kmsg[printk_kmsg_end] = fmt[2];
		kmsg_pos_add(1);
		// Don't print syslog level to console
		fmt_msg = fmt + 3;
	} else {
		fmt_msg = fmt;
	}

	ret = vsprintf(buffer, fmt_msg, ap);
	if (!ret)
		return ret;

	_Static_assert(KMSG_BUFSIZE >= STR_BUFSIZE, "KMSG_BUFSIZE too small");
	ret = strncpy(printk_kmsg + printk_kmsg_end, buffer,
	              KMSG_BUFSIZE - printk_kmsg_end);
	kmsg_pos_add(ret + 1);

	if (buffer[ret] && buffer[ret + 1]) {
		ret = strncpy(printk_kmsg + printk_kmsg_end, buffer + ret,
		              KMSG_BUFSIZE - printk_kmsg_end);
		kmsg_pos_add(ret + 1);
	}

	console_update();
	return ret;
}

#define is_pos_invalid(pos) ( \
	 (pos >= KMSG_BUFSIZE) \
	 || (pos < 0) \
	 || (printk_kmsg_start < printk_kmsg_end \
	     && pos >= printk_kmsg_end) \
	 || (printk_kmsg_start > printk_kmsg_end \
	     && pos >= printk_kmsg_end \
	     && pos < printk_kmsg_start) \
	 || (printk_kmsg_start == printk_kmsg_end) \
	)

#define is_ptr_invalid(ptr) ( \
	ptr == NULL || is_pos_invalid((size_t) (ptr - printk_kmsg)) \
	)

/* public: print.h */
const char * kmsg_next(const char * ptr)
{
	size_t pos = ptr - printk_kmsg;
	size_t newpos = 0;

	if (ptr == NULL) {
		int i = printk_kmsg_start;
		while (i < KMSG_BUFSIZE
		       && printk_kmsg[i] != 0
		       && printk_kmsg[i + 1] != '<')
			i++;
		return printk_kmsg + i + 1;
	}
	if (is_pos_invalid(pos))
		return NULL;

	pos = kmsg_get_str(printk_kmsg + pos) - printk_kmsg;
	newpos = pos + strlen(printk_kmsg + pos) + 1;

	if (newpos >= KMSG_BUFSIZE)
		newpos = newpos - KMSG_BUFSIZE;
	if (is_pos_invalid(newpos))
		return NULL;
	return printk_kmsg + newpos;
}

/* public: print.h */
const char * kmsg_get_str(const char * ptr)
{
	size_t pos = ptr - printk_kmsg;

	if (is_ptr_invalid(ptr))
		return NULL;

	if (printk_kmsg[pos] == '<') {
		pos += 3;
		if (pos >= KMSG_BUFSIZE)
			pos = pos - KMSG_BUFSIZE;
	}
	return printk_kmsg + pos;
}
