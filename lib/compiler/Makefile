include $(SRC_ROOT)/Makefile.flags

obj-y = cxx/
obj-$(CONFIG_DEBUG) += test/
obj-$(CONFIG_UBSAN) += ubsan.o
bootloader-$(CONFIG_UBSAN) += ubsan.o
obj-$(CONFIG_STACK_PROTECTOR) += stack_protector.o
bootloader-$(CONFIG_STACK_PROTECTOR) += stack_protector.o

include $(SRC_ROOT)/Makefile.rules
