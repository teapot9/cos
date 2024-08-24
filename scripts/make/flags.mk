# Build flags

# Global variables

export KERNEL_NAME := kernel

export KERNEL_VERSION_MAJOR := 0
export KERNEL_VERSION_MINOR := 1
export KERNEL_VERSION_PATCH := 0

KERNEL_VERSION := $(KERNEL_VERSION_MAJOR).$(KERNEL_VERSION_MINOR).$(KERNEL_VERSION_PATCH)
export KERNEL_VERSION

# Compilation variables

export ARCH ?= $(shell uname -m)
export CC ?= cc
export CXX ?= c++
export LD ?= ld
export OBJCOPY ?= objcopy
export STRIP ?= strip
export AR ?= ar
export RANLIB ?= ranlib
export SED ?= sed
export MKDIR ?= mkdir

# Helper scripts

BIN2C = $(SRC_ROOT)/scripts/bin2c

# Define paths

RELAPATH := $(shell realpath --relative-to=$(SRC_ROOT) .)
SRC := $(abspath $(SRC_ROOT)/$(RELAPATH))
BUILD := $(abspath $(BUILD_ROOT)/$(RELAPATH))
SRC_ARCH := $(SRC_ROOT)/arch/$(ARCH)
BUILD_ARCH := $(BUILD_ROOT)/arch/$(ARCH)
BUILD_ARCH := $(SRC_ROOT)/arch/$(ARCH)

# Source config
ifndef no-load-kconfigs
include $(BUILD_ROOT)/include/config/auto.conf
endif

# Make flags

MAKEARGS += SRC_ROOT=$(SRC_ROOT) BUILD_ROOT=$(BUILD_ROOT) CONFIG=$(CONFIG) V=$(V)
ifeq ($(V),0)
	Q = @
	MAKEOPTS += --no-print-directory
else
	Q =
endif
ifeq ($(V),1)
	MAKEOPTS += --no-print-directory
endif
ifeq ($(CC),include-what-you-use)
	MAKEOPTS += -b -k
endif

# Build flags

CPPFLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CPPFLAGS += -I$(BUILD_ARCH)/include -I$(SRC_ARCH)/include
CPPFLAGS += -I$(BUILD_ROOT)/include -I$(SRC_ROOT)/include
CPPFLAGS += -iquote $(BUILD_ARCH)/$(RELAPATH) -iquote $(SRC_ARCH)/$(RELAPATH)
CPPFLAGS += -iquote $(BUILD_ROOT)/$(RELAPATH) -iquote $(SRC_ROOT)/$(RELAPATH)

CFLAGS += -nostdinc -std=c18

CXXFLAGS += -nostdinc++ -std=c++17
CXXFLAGS += -fno-rtti -fno-exceptions

BOOT_FLAGS += -mcmodel=small -fPIC -DBOOT

LDFLAGS += -znodefaultlib -nostdlib -znocombreloc
LDFLAGS += -unresolved-symbols=report-all --orphan-handling=warn
LDFLAGS += --warn-common --warn-alternate-em

common-flags :=
common-flags += -msse3
common-flags += -mno-red-zone -mabi=sysv
common-flags += -ffreestanding
common-flags += -mcmodel=kernel -fno-pie
common-flags += -Wall -Wextra -Wpedantic
common-flags += -Wtype-limits -Wstrict-prototypes -Wtype-limits
common-flags += -Wframe-larger-than=$(CONFIG_KERNEL_FRAME_SIZE)
common-flags += -mgeneral-regs-only

ifdef CONFIG_X86_64
	common-flags += -m64
endif
ifdef CONFIG_X86_32
	common-flags += -m32
endif

ifdef CONFIG_DEBUG
	common-flags += -g
endif

ifdef CONFIG_CC_OPTIMIZE_O0
	common-flags += -O0
endif
ifdef CONFIG_CC_OPTIMIZE_O1
	common-flags += -O1
endif
ifdef CONFIG_CC_OPTIMIZE_O2
	common-flags += -O2
endif
ifdef CONFIG_CC_OPTIMIZE_O3
	common-flags += -O3
endif
ifdef CONFIG_CC_OPTIMIZE_Os
	common-flags += -Os
endif

ifdef CONFIG_ASM_DEFAULT_INTEL
	common-flags += -masm=intel
endif
ifdef CONFIG_ASM_DEFAULT_ATT
	common-flags += -masm=att
endif

# TODO
ifdef CONFIG_STACK_PROTECTOR
	common-flags += -fstack-protector-all -mstack-protector-guard=global
else
	common-flags += -fno-stack-protector
endif

# TODO
ifdef CONFIG_STACK_USAGE
	common-flags += -fstack-usage
endif

# TODO
ifdef CONFIG_UBSAN
	common-flags += -fsanitize=undefined
ifdef CONFIG_CC_IS_CLANG
	# disable function sanitizer: requires RTTI
	common-flags += -fno-sanitize=function
endif
endif
ifdef CONFIG_UBSAN_IMPLICIT_CONVERSION
ifdef CONFIG_CC_IS_CLANG
	common-flags += -fsanitize=implicit-conversion
endif
endif

CFLAGS := $(common-flags) $(CFLAGS)
CXXFLAGS := $(common-flags) $(CXXFLAGS)
ASFLAGS := $(common-flags) $(ASFLAGS)
common-flags :=
