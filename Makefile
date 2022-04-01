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

obj-y = arch/ dev/ kernel/ lib/ mm/
clean-y += $(COS_KERNEL).so
$(info cleany = $(clean-y))

.PHONY: all
all: image

.PHONY: image
image: $(BUILD)/$(COS_KERNEL).so
	$(Q)$(MAKE) -C arch/$(ARCH)/boot SRC_ROOT=$(SRC_ROOT) BUILD_ROOT=$(BUILD_ROOT) image

$(BUILD)/$(COS_KERNEL).so: $(BUILD)/modules.o
	$(LD) $(LDFLAGS) -o $@ $^

include $(SRC_ROOT)/Makefile.rules

MCONF ?= mconf

.PHONY: menuconfig
menuconfig:
	$(MCONF) Kconfig

.PHONY: rebuild
rebuild:
	$(Q)$(MAKE) clean
	$(Q)$(MAKE) all
