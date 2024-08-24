include $(SRC_ROOT)/scripts/make/flags.mk

obj-y += main.o serial.o device.o
obj-$(CONFIG_SERIAL_CONSOLE) += console.o

boot-y += main.o serial.o device.o console.o

include $(SRC_ROOT)/scripts/make/rules.mk
