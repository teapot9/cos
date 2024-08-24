include $(SRC_ROOT)/scripts/make/flags.mk

obj-$(CONFIG_FONT_BUILTIN_TERMINUS_8X16) += font_ter_v16n.psf.o
obj-y += fonts.o
obj-$(CONFIG_FONT_PSF) += psf.o

clean-y += font_ter_v16n.psf.c font_ter_v16n.psf.h

include $(SRC_ROOT)/scripts/make/rules.mk

$(SRC)/font_ter_v16n.psf:
	$(NOOP)

$(BUILD)/font_ter_v16n.psf.c: $(SRC)/font_ter_v16n.psf $(BUILD)/font_ter_v16n.psf.h
	$(BIN2C) $< $@

$(BUILD)/font_ter_v16n.psf.h: $(SRC)/font_ter_v16n.psf
	$(BIN2C) $< $@
