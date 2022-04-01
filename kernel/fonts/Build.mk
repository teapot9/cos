OBJECTS += \
	fonts.o

#CONFIG_FONT_BUILTIN := $(patsubst '%',%,$(CONFIG_FONT_BUILTIN))
#$(foreach font, $(CONFIG_FONT_BUILTIN), \
#	$(eval OBJECTS += $(font).o) \
#)

ifdef CONFIG_FONT_BUILTIN_TERMINUS_8X16
	OBJECTS += font_ter_v16n.psf.o
endif

ifdef CONFIG_FONT_PSF
	OBJECTS += psf.o
endif
