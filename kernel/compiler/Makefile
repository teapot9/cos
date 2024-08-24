include $(SRC_ROOT)/scripts/make/flags.mk

obj-y = cxx/
obj-$(CONFIG_DEBUG) += test/
obj-$(CONFIG_UBSAN) += ubsan.o
boot-$(CONFIG_UBSAN) += ubsan.o
obj-$(CONFIG_STACK_PROTECTOR) += stack_protector.o
boot-$(CONFIG_STACK_PROTECTOR) += stack_protector.o

include $(SRC_ROOT)/scripts/make/rules.mk
