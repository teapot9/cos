include $(SRC_ROOT)/Makefile.flags

CFLAGS += -mfsgsbase

obj-y = halt.o idt.o gdt.o isr.o cpu.o isr_wrapper.o int.o
bootloader-y = halt.o int.o

include $(SRC_ROOT)/Makefile.rules
