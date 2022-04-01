ifdef CONFIG_VIDEO_FB
	OBJECTS += main.o
	OBJECTS += console.o
	OBJECTS += fb.o
endif

ifdef CONFIG_VIDEO_EFIGOP
	OBJECTS += efigop.o
endif
