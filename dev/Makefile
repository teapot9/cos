include $(SRC_ROOT)/Makefile.flags

bootloader-y = firmware/ tty/
bootloader-$(CONFIG_VIDEO) += video/

obj-y = firmware/ tty/ clock/
obj-$(CONFIG_VIDEO) += video/
obj-$(CONFIG_X86) += irq/

include $(SRC_ROOT)/Makefile.rules
