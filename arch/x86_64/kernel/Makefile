include $(SRC_ROOT)/scripts/make/flags.mk

CFLAGS += -mfsgsbase

obj-y = halt.o idt.o gdt.o isr.o cpu.o isr_wrapper.o int.o
boot-y = halt.o int.o

include $(SRC_ROOT)/scripts/make/rules.mk
