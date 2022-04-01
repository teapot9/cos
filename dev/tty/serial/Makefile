include $(SRC_ROOT)/Makefile.flags

obj-y += main.o serial.o device.o
obj-$(CONFIG_SERIAL_CONSOLE) += console.o

bootloader-y += main.o serial.o device.o console.o

include $(SRC_ROOT)/Makefile.rules
