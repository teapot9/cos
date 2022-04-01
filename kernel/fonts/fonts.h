#ifndef KERNEL_FONTS_FONTS_H
#define KERNEL_FONTS_FONTS_H

#ifdef CONFIG_FONT_PSF
#include "psf.h"
#endif

struct font {
	enum font_type {
		UNKNOWN,
#ifdef CONFIG_FONT_PSF
		FONT_TYPE_PSF,
#endif
	} type;
	union font_info {
		void * none;
#ifdef CONFIG_FONT_PSF
		struct psf_font psf;
#endif
	} info;
};

#endif // KERNEL_FONTS_FONTS_H
