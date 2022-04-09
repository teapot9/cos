@break entry_efi_s2
@break kernel_main
@break isr_handler if frame->interrupt < 32
#@break do_call
b console.c:181




# Reload symbols to kernel offset (0xffff...)
@break entry_efi_wrapper_s2
*symbol-file
*symbol-file -readnow cos.elf
*cont

# Break as soon as possible, get out of the infinite loop, continue
@break entry_efi_s1
@bt
@set tmp=1
@cont
