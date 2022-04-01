include $(SRC_ROOT)/Makefile.flags

bootloader-y = init.o paging.o physical.o ptable.o vkernel.o kapi.o
bootloader-$(CONFIG_MM_DEBUG) += debug.o
obj-y = $(bootloader-y)

include $(SRC_ROOT)/Makefile.rules
