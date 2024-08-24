# Kernel main makefile

# Early variables definitions
MAKEFILE_DIR := $(shell dirname $(abspath $(firstword $(MAKEFILE_LIST))))
export SRC_ROOT := $(abspath $(if $(INPUT),$(INPUT),$(MAKEFILE_DIR)))
export BUILD_ROOT := $(abspath $(if $(OUTPUT),$(OUTPUT),$(MAKEFILE_DIR)/build))
export CONFIG := $(abspath $(BUILD_ROOT)/.config)
V ?= 1

# Default target
.PHONY: _all
_all: image all

# Set no-load-kconfigs for targets that work on .config
ifneq ($(filter menuconfig syncconfig,$(MAKECMDGOALS)),)
no-load-kconfigs := 1
endif

# Error message for missing .config file
ifndef no-load-kconfigs
$(CONFIG):
	$(error config file not found, please run "make menuconfig" to build one)
endif

# Load flags
include $(SRC_ROOT)/scripts/make/flags.mk

# Define build targets
sections = arch/$(ARCH)/sections.lds
obj-y := arch/ dev/ kernel/ lib/ mm/
boot-y := $(obj-y)
clean-y += $(KERNEL_NAME).elf $(sections) tags include/generated/ include/config/

# Image target
.PHONY: image
image: $(BUILD)/$(KERNEL_NAME).elf $(BUILD)/boot.o
	$(Q)$(MAKE) $(MAKEOPTS) -C $(SRC_ARCH)/boot $(MAKEARGS) image

# Kernel ELF image
.PHONY: $(BUILD)/$(KERNEL_NAME).elf
$(BUILD)/$(KERNEL_NAME).elf: $(BUILD)/kernel.o
	$(Q)$(SED) -e 's/@@IMAGE_BASE@@/0xffffffff80000000/g' $(SRC_ARCH)/sections.lds.in >$(BUILD)/$(sections)
	$(Q)$(LD) -dT $(BUILD)/$(sections) $(LDFLAGS) -static -entry=entry_efi_wrapper_s2 -o $@ $^

# Build tags file

.PHONY: tags
tags:
	$(Q)$(SRC_ROOT)/scripts/tags $(SRC_ROOT) >$(BUILD_ROOT)/tags

# include-what-you-use rule

IWYU_FLAGS += -Xiwyu --error_always -Xiwyu --check_also=*.h -Xiwyu --verbose=2 -w
.PHONY: iwyu
iwyu:
	$(Q)env CFLAGS="$(IWYU_FLAGS)" CXXFLAGS="$(IWYU_FLAGS)" $(MAKE) -B -k CC=include-what-you-use CXX=include-what-you-use

# Syntax check rule

SYNTAX_FLAGS += -fsyntax-only
.PHONY: syntax
syntax:
	$(Q)env CFLAGS="$(SYNTAX_FLAGS)" CXXFLAGS="$(SYNTAX_FLAGS)" $(MAKE) -B -k

# Define custom rules

.PHONY: rebuild
rebuild:
	$(Q)$(MAKE) clean
	$(Q)$(MAKE) all

%.var:
	@echo $($*)

# Kconfig rules

ifndef no-load-kconfigs
include $(BUILD_ROOT)/include/config/auto.conf.cmd
endif

export KCONFIG_CONFIG := $(CONFIG)
export KCONFIG_AUTOCONFIG := $(BUILD_ROOT)/include/config/auto.conf
export KCONFIG_AUTOHEADER := $(BUILD_ROOT)/include/generated/autoconf.h
export KCONFIG_RUSTCCFG := $(BUILD_ROOT)/include/generated/rustc_cfg

MCONF ?= mconf
CONF ?= conf

.PHONY: menuconfig
menuconfig: $(BUILD)
	$(Q)$(MCONF) Kconfig

.PHONY: syncconfig
syncconfig:
	$(Q)$(CONF) --syncconfig Kconfig

# Load default rules
include $(SRC_ROOT)/scripts/make/rules.mk
