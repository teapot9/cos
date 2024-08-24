include $(SRC_ROOT)/scripts/make/flags.mk

boot-y = firmware/ tty/
boot-$(CONFIG_VIDEO) += video/

obj-y = firmware/ tty/ clock/
obj-$(CONFIG_VIDEO) += video/
obj-$(CONFIG_X86) += irq/

include $(SRC_ROOT)/scripts/make/rules.mk
