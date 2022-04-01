include $(SRC_ROOT)/Makefile.flags

sections = $(BUILD)/../sections.bootloader.lds

obj-$(CONFIG_BOOT_EFISTUB) += entry_efi_s2.o entry_efi_wrapper.o
bootloader-$(CONFIG_BOOT_EFISTUB) += entry_efi_s1.o entry_efi_wrapper.o
clean-y += $(COS_KERNEL).efi $(COS_KERNEL).efi.debug $(COS_KERNEL).so $(COS_KERNEL).elf.o $(sections)

EFIARCH := efi-app-$(ARCH)
CFLAGS += -fomit-frame-pointer

include $(SRC_ROOT)/Makefile.rules

.PHONY: image
image: $(BUILD)/$(COS_KERNEL).efi

$(BUILD)/$(COS_KERNEL).efi: $(BUILD)/$(COS_KERNEL).efi.debug
	$(STRIP) --strip-debug -o $@ $<

$(BUILD)/$(COS_KERNEL).efi.debug: $(BUILD)/$(COS_KERNEL).so
	$(OBJCOPY) --target $(EFIARCH) --subsystem 10 $< $@

$(BUILD)/$(COS_KERNEL).so: $(BUILD_ROOT)/bootloader.o $(BUILD)/$(COS_KERNEL).elf.o
	$(SED) -e 's/@@IMAGE_BASE@@/0x1000/g' $(SRC)/../sections.lds.in >$(sections)
	$(LD) -dT $(sections) $(LDFLAGS) -Bsymbolic -shared -entry=entry_efi_wrapper_s1 -o $@ $^

$(BUILD)/$(COS_KERNEL).elf.o: $(BUILD_ROOT)/$(COS_KERNEL).elf
	cd $(dir $<) && $(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $(notdir $<) $@
