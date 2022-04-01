ifdef CONFIG_BOOT_EFI_STUB
	OBJECTS += entry_efi.o
	EFIARCH = efi-app-$(ARCH)
	LDFLAGS += -dT arch/x86/sections_efi.lds
	LDFLAGS += -entry=entry_efi -shared
	TARGET = $(KERNEL).efi
ifdef CONFIG_DEBUG
	TARGET += $(KERNEL).efi.debug
endif
endif

ifdef CONFIG_BOOT_GRUB
	OBJECTS += entry_grub.o
ifndef CONFIG_BOOT_EFI_STUB
	LDFLAGS += -entry=entry_grub
endif
endif


