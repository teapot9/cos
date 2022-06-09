#include <cmdline.h>

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include <alloc.h>

const char * kernel_cmdline = "";
static const char * const extra_cmdline = CONFIG_CMDLINE;

/* setup.h */
int cmdline_init(const char * (* firmware_cmdline)(void))
{
	static const char * const empty = "";
	int err = 0;

	/* Get cmdlines */
	const char * firmware = firmware_cmdline();
	if (firmware == NULL)
		firmware = empty;
	size_t firmware_sz = strlen(firmware);
	const char * extra = extra_cmdline;
	size_t extra_sz = strlen(extra);
	const char * sep = firmware_sz && extra_sz ? " " : empty;
	size_t sep_sz = firmware_sz && extra_sz ? 1 : 0;

	/* Allocate final cmdline */
	size_t final_sz = firmware_sz + sep_sz + extra_sz + 1;
	char * final = malloc(final_sz * sizeof(*final));
	if (final == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	/* Set cmdline order */
	const char * first, * last;
	size_t first_sz, last_sz;
#if IS_ENABLED(CONFIG_CMDLINE_OVERRIDE)
	first = extra;
	first_sz = extra_sz;
	last = firmware;
	last_sz = firmware_sz;
#else
	first = firmware;
	first_sz = firmware_sz;
	last = extra;
	last_sz = extra_sz;
#endif

	/* Write cmdline */
	size_t pos = 0;
	strncpy(final + pos, first, first_sz);
	pos += first_sz;
	strncpy(final + pos, sep, sep_sz);
	pos += sep_sz;
	strncpy(final + pos, last, last_sz);
	pos += last_sz;
	final[pos] = 0;

exit:
	if (err) {
		if (final != NULL) {
			free(final);
			final = NULL;
		}
	} else {
		kernel_cmdline = final;
	}
	if (firmware != NULL)
		kfree(firmware);
	return err;
}
