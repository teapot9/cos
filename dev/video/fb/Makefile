include $(SRC_ROOT)/scripts/make/flags.mk

obj-y += console.o fb.o main.o
obj-$(CONFIG_FB_EFIGOP) += efigop.o

boot-$(CONFIG_FB_EFIGOP) += efigop_boot.o

include $(SRC_ROOT)/scripts/make/rules.mk
