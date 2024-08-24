include $(SRC_ROOT)/scripts/make/flags.mk

obj-$(CONFIG_EFISTUB) += main.o exit.o libstub.o
obj-$(CONFIG_EFISTUB_CONSOLE) += console.o

boot-y += $(obj-y)

include $(SRC_ROOT)/scripts/make/rules.mk
