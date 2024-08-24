include $(SRC_ROOT)/scripts/make/flags.mk

sections = ../sections.boot.lds

obj-$(CONFIG_EFISTUB) += entry_efi_s2.o entry_efi_wrapper.o
boot-$(CONFIG_EFISTUB) += entry_efi_s1.o entry_efi_wrapper.o
clean-y += $(KERNEL_NAME).efi $(KERNEL_NAME).efi.debug $(KERNEL_NAME).so $(KERNEL_NAME).elf.o $(sections)

EFIARCH := efi-app-$(ARCH)
CPPFLAGS += -DKERNEL_NAME=$(KERNEL_NAME)
CFLAGS += -fomit-frame-pointer

include $(SRC_ROOT)/scripts/make/rules.mk

.PHONY: image
image: $(BUILD)/$(KERNEL_NAME).efi

$(BUILD)/$(KERNEL_NAME).efi: $(BUILD)/$(KERNEL_NAME).efi.debug
	$(Q)$(STRIP) --strip-debug -o $@ $<

$(BUILD)/$(KERNEL_NAME).efi.debug: $(BUILD)/$(KERNEL_NAME).so
	$(Q)$(OBJCOPY) --target $(EFIARCH) --subsystem 10 $< $@

$(BUILD)/$(KERNEL_NAME).so: $(BUILD_ROOT)/boot.o $(BUILD)/$(KERNEL_NAME).elf.o
	$(Q)$(SED) -e 's/@@IMAGE_BASE@@/0x1000/g' $(SRC)/../sections.lds.in >$(BUILD)/$(sections)
	$(Q)$(LD) -dT $(BUILD)/$(sections) $(LDFLAGS) -Bsymbolic -shared -entry=entry_efi_wrapper_s1 -o $@ $^

$(BUILD)/$(KERNEL_NAME).elf.o: $(BUILD_ROOT)/$(KERNEL_NAME).elf
	$(Q)cd $(dir $<) && $(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $(notdir $<) $@
