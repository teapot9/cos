# Root makefile

ROOT := $(shell dirname $(abspath $(firstword $(MAKEFILE_LIST))))
ifdef SRC
	SRC_ROOT := $(SRC)
else
	SRC_ROOT := $(ROOT)
endif
ifdef BUILD
	BUILD_ROOT := $(BUILD)
else
	BUILD_ROOT := $(ROOT)
endif

#SRC_ROOT := $(ROOT)
#BUILD_ROOT := $(ROOT)

V = 1
ifdef V
	Q = @
else
	Q =
endif

include $(SRC_ROOT)/Makefile.flags

KCONFIG ?= .config

sections = $(BUILD)/arch/$(ARCH)/sections.lds
obj-y = arch/ dev/ kernel/ lib/ mm/
bootloader-y = $(obj-y)
clean-y += $(COS_KERNEL).elf $(sections)
clean-y += $(wildcard include/config/*)
clean-y += include/generated/autoconf.h


.PHONY: allx
allx: image all
ifeq ($(CONFIG_STACK_USAGE), y)
ifneq ($(CONFIG_STACK_USAGE_WARN), 0)
	./scripts/checkstack $(CONFIG_STACK_USAGE_WARN)
endif
endif

.PHONY: image
image: $(BUILD)/$(COS_KERNEL).elf $(BUILD)/bootloader.o
	$(Q)$(MAKE) -C arch/$(ARCH)/boot SRC_ROOT=$(SRC_ROOT) BUILD_ROOT=$(BUILD_ROOT) image

$(BUILD)/$(COS_KERNEL).elf: $(BUILD)/modules.o
	$(SED) -e 's/@@IMAGE_BASE@@/0xffffffff80000000/g' $(SRC)/arch/$(ARCH)/sections.lds.in >$(sections)
	$(LD) -dT $(sections) $(LDFLAGS) -static -entry=entry_efi_wrapper_s2 -o $@ $^
#	$(LD) -dT $(sections) $(LDFLAGS) -pie -static -entry=entry_efi_s2 -o $@ $^

include $(SRC_ROOT)/Makefile.rules

MCONF ?= mconf
CONF ?= conf

include $(BUILD)/include/config/auto.conf.cmd

%.var:
	@echo $($*)

.PHONY: noop
noop:
	$(NOOP)

.PHONY: menuconfig
menuconfig:
	$(MCONF) Kconfig

.PHONY: syncconfig
syncconfig:
	$(CONF) --syncconfig Kconfig

.PHONY: rebuild
rebuild:
	$(Q)$(MAKE) clean
	$(Q)$(MAKE) all

$(BUILD)/include/generated/autoconf.h $(BUILD)/include/config/auto.conf $(BUILD)/include/config/auto.conf.cmd: $(KCONFIG)
	$(CONF) --syncconfig Kconfig

.PHONY: FORCE
FORCE:
