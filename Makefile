# cos makefile

# Base tools

ifndef ROOT_DIR
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
endif

ifndef config
define config
	$(eval $(1))
	$(eval CPPFLAGS += -D$(1))
endef
endif

# Environment

BUILD ?= .
SRC ?= .

ARCH ?= $(shell uname -m)
CC ?= cc
LD ?= ld
OBJCOPY ?= objcopy
STRIP ?= strip

KERNEL = cos
TARGET = $(KERNEL).elf
ALL_OBJECTS =

CFLAGS += -ffreestanding -fpic -fPIC -fpie -fno-stack-protector
CFLAGS += -mno-red-zone -mabi=sysv
CFLAGS += -Wall -Wextra -Wpedantic -std=c18
CFLAGS += -Wtype-limits -Wstrict-prototypes -Wtype-limits
CFLAGS += -Iinclude
ifeq ($(CC), gcc)
	CFLAGS += -mgeneral-regs-only
endif
LDFLAGS += -znodefaultlib -nostdlib -znocombreloc -Bsymbolic
LDFLAGS += -unresolved-symbols=report-all
LDFLAGS += --warn-common --warn-alternate-em

BIN2C = $(SRC)/scripts/bin2c

# Global config

include .config
CPPFLAGS += $(shell $(SRC)/scripts/get_config_cppflags)
#CONFIG = CONFIG_DEBUG CONFIG_BOOT_EFI_STUB CONFIG_BOOT_GRUB CONFIG_ASM_DEFAULT_INTEL CONFIG_VIDEO_EFIGOP
#$(foreach cfg, $(CONFIG), $(call config, $(cfg)))

include Config.mk

# Get object list

SUBDIRS = arch drivers init kernel mm
BUILD_MK = Build.mk $(shell find $(SUBDIRS) -name Build.mk)
$(foreach mk, $(BUILD_MK), \
	$(info include $(mk)) \
	$(eval OBJECTS = ) \
	$(eval include $(mk)) \
	$(eval ALL_OBJECTS += $(addprefix $(dir $(mk)), $(OBJECTS))) \
	)
OBJECTS =

.phony: all clean

# Special build rules

all: $(BUILD)/$(TARGET)

clean:
	rm -f $(addprefix $(BUILD)/, $(ALL_OBJECTS) $(KERNEL).so $(KERNEL).efi $(KERNEL).elf)

# Unique build rules

$(SRC)/kernel/fonts/font_ter_v16n.psf:
	:

$(BUILD)/kernel/fonts/font_ter_v16n.psf.c: $(SRC)/kernel/fonts/font_ter_v16n.psf $(BUILD)/kernel/fonts/font_ter_v16n.psf.h
	$(BIN2C) $< $@

$(BUILD)/kernel/fonts/font_ter_v16n.psf.h: $(SRC)/kernel/fonts/font_ter_v16n.psf
	$(BIN2C) $< $@

# Default build rules

$(BUILD)/%.i: $(SRC)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -E -o $@ $^

$(BUILD)/%.i: $(BUILD)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -E -o $@ $^

$(BUILD)/%.s: $(SRC)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -S -o $@ $^

$(BUILD)/%.s: $(BUILD)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -S -o $@ $^

$(BUILD)/%.s: $(BUILD)/%.i
	$(CC) $(CFLAGS) -S -o $@ $^

$(BUILD)/%.s: $(SRC)/%.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -E -o $@ $^

$(BUILD)/%.s: $(BUILD)/%.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -E -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(BUILD)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(BUILD)/%.i
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(SRC)/%.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(BUILD)/%.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

$(BUILD)/%.o: $(BUILD)/%.s
	$(CC) $(CFLAGS) -c -o $@ $^

# Kernel image build rules

$(BUILD)/$(KERNEL).elf: $(addprefix $(BUILD)/, $(ALL_OBJECTS))
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/$(KERNEL).so: $(addprefix $(BUILD)/, $(ALL_OBJECTS))
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILD)/$(KERNEL).efi: $(BUILD)/$(KERNEL).efi.debug
	$(STRIP) --strip-debug -o $@ $<

$(BUILD)/$(KERNEL).efi.debug: $(BUILD)/$(KERNEL).so
	$(OBJCOPY) --target $(EFIARCH) --subsystem 10 $< $@
