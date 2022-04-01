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

sections = $(BUILD)/arch/$(ARCH)/sections.lds
obj-y = arch/ dev/ kernel/ lib/ mm/
bootloader-y = $(obj-y)
clean-y += $(COS_KERNEL).elf $(sections)


.PHONY: all
all: image

.PHONY: image
image: $(BUILD)/$(COS_KERNEL).elf
	$(Q)$(MAKE) -C arch/$(ARCH)/boot SRC_ROOT=$(SRC_ROOT) BUILD_ROOT=$(BUILD_ROOT) image

$(BUILD)/$(COS_KERNEL).elf: $(BUILD)/modules.o
	$(SED) -e 's/@@IMAGE_BASE@@/0xffffffff80000000/g' $(SRC)/arch/$(ARCH)/sections.lds.in >$(sections)
	$(LD) -dT $(sections) $(LDFLAGS) -pie -static -entry=entry_efi_wrapper_s2 -o $@ $^

include $(SRC_ROOT)/Makefile.rules

MCONF ?= mconf

.PHONY: menuconfig
menuconfig:
	$(MCONF) Kconfig

.PHONY: rebuild
rebuild:
	$(Q)$(MAKE) clean
	$(Q)$(MAKE) all
