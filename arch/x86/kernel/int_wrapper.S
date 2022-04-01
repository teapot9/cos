.globl int_wrapper
.align 4

int_wrapper:
	pushfq
	cld
	call int_handler
	popfq
	iretq
