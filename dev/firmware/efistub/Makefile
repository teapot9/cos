include $(SRC_ROOT)/Makefile.flags

obj-$(CONFIG_EFISTUB) += main.o exit.o libstub.o
obj-$(CONFIG_EFISTUB_CONSOLE) += console.o

bootloader-y += $(obj-y)

include $(SRC_ROOT)/Makefile.rules
