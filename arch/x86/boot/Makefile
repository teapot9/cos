include $(SRC_ROOT)/Makefile.flags

obj-$(CONFIG_BOOT_EFISTUB) += entry_efi.o
clean-y += $(COS_KERNEL).efi $(COS_KERNEL).efi.debug

EFIARCH := efi-app-$(ARCH)

include $(SRC_ROOT)/Makefile.rules

.PHONY: image
image: $(BUILD)/$(COS_KERNEL).efi

$(BUILD)/$(COS_KERNEL).efi: $(BUILD)/$(COS_KERNEL).efi.debug
	$(STRIP) --strip-debug -o $@ $<

$(BUILD)/$(COS_KERNEL).efi.debug: $(BUILD_ROOT)/$(COS_KERNEL).so
	$(OBJCOPY) --target $(EFIARCH) --subsystem 10 $< $@
