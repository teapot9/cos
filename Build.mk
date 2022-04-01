ifdef CONFIG_DEBUG
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

ifdef CONFIG_IS_X64
	CFLAGS += -m64
	CPPFLAGS += -I$(SRC)/arch/x86/include
endif
ifdef CONFIG_IS_X86
	CFLAGS += -m32
endif

ifdef CONFIG_ASM_DEFAULT_INTEL
	CFLAGS += -masm=intel
endif
ifdef CONFIG_ASM_DEFAULT_ATT
	CFLAGS += -masm=att
endif
