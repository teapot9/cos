include $(SRC_ROOT)/Makefile.flags

bootloader-y = vmm.o page.o debug.o
bootloader-$(CONFIG_M_DEBUG) += debug.o
obj-y = $(bootloader-y)

include $(SRC_ROOT)/Makefile.rules
