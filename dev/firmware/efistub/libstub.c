#include <firmware/efistub.h>
#include "efistub.h"

#include <print.h>
#include <unicode.h>

#define _str(x) #x
#define str(x) _str(x)
const char * const extra_cmdline = str(CONFIG_CMDLINE);

/* Return EFI cmdline in a dynamically allocated buffer */
static char * get_efi_cmdline(void)
{
	char * cmdline = NULL;
	size_t size;

	if (efistub_image_proto() == NULL)
		return NULL;

	if (efistub_image_proto()->load_options_size == 0
	    || efistub_image_proto()->load_options == NULL)
		return NULL;

	size = efistub_image_proto()->load_options_size * 2;
	cmdline = kmalloc(size);
	if (cmdline == NULL) {
		pr_err("Failed to allocate %zu bytes for the EFI "
		       "cmdline buffer", size);
		kfree(cmdline);
		return NULL;
	}

	if (utf16_to_utf(cmdline, efistub_image_proto()->load_options, size)
	    == 0) {
		pr_err("Failed to convert UTF-16 to UTF-8 UEFI cmdline\n", 0);
		kfree(cmdline);
		return NULL;
	}

	return cmdline;
}

/* firmware/efistub.h */
/* Return cmdline in a dynamically allocated buffer */
char * efistub_cmdline(void)
{
	size_t size = 0;
	size_t start = 0;
	char * efi_cmdline = get_efi_cmdline();

	if (efi_cmdline != NULL)
		size += strlen(efi_cmdline);
	if (size)
		size += sizeof(char); // space
	size += strlen(efi_cmdline);

	char * cmdline = kmalloc(size);
	if (cmdline == NULL) {
		pr_err("Failed to allocate %zu bytes for cmdline\n", 0);
		kfree(efi_cmdline);
		return NULL;
	}

#ifndef CONFIG_CMDLINE_OVERRIDE
	if (*extra_cmdline) {
		strncpy(cmdline + start, extra_cmdline, size - start);
		start = strlen(cmdline);
		cmdline[start] = ' ';
		cmdline[++start] = 0;
	}
#endif
	if (efi_cmdline != NULL) {
		strncpy(cmdline + start, efi_cmdline, size - start);
		start = strlen(cmdline);
	}
#ifdef CONFIG_CMDLINE_OVERRIDE
	if (efi_cmdline != NULL && *efi_cmdline) {
		cmdline[start] = ' ';
		cmdline[++start] = 0;
	}
	strncpy(cmdline + start, extra_cmdline, size - start);
	start = strlen(cmdline);
#endif

	kfree(efi_cmdline);
	return cmdline;
}

