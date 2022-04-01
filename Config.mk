# Dependency management for configuration

ifeq ($(ARCH), x86_64)
$(call config, CONFIG_IS_X64=y)
else ifeq ($(ARCH), x86)
$(call config, CONFIG_IS_X86=y)
endif

ifdef CONFIG_VIDEO_EFIGOP
$(call config, CONFIG_BOOT_EFI_STUB=y)
$(call config, CONFIG_VIDEO_FB=y)
endif

ifdef CONFIG_BOOT_EFI_STUB
$(call config, CONFIG_FIRMWARE_EFI=y)
endif
