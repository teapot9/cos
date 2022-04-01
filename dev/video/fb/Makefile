include $(SRC_ROOT)/Makefile.flags

obj-y += console.o fb.o main.o
obj-$(CONFIG_FB_EFIGOP) += efigop.o

bootloader-$(CONFIG_FB_EFIGOP) += efigop_boot.o

include $(SRC_ROOT)/Makefile.rules
