#ifndef _LIB_FONTS_FONTS_H
#define _LIB_FONTS_FONTS_H

#include <kconfig.h>

#if IS_ENABLED(CONFIG_FONT_PSF)
#include "psf.h"
#endif

struct font {
	enum font_type {
		UNKNOWN,
#if IS_ENABLED(CONFIG_FONT_PSF)
		FONT_TYPE_PSF,
#endif
	} type;
	union font_info {
		void * none;
#if IS_ENABLED(CONFIG_FONT_PSF)
		struct psf_font psf;
#endif
	} info;
};

#endif // _LIB_FONTS_FONTS_H
