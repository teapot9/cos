include $(SRC_ROOT)/scripts/make/flags.mk

boot-y = init.o paging.o physical.o ptable.o vkernel.o kapi.o
boot-$(CONFIG_MM_DEBUG) += debug.o
obj-y = $(boot-y)

include $(SRC_ROOT)/scripts/make/rules.mk
