@break entry_efi_s2
@break kernel_main
@break isr_handler if frame->interrupt < 32
#@break do_call
#break kernel/init.c:146
#*print i
#*cont




@break entry_efi_wrapper_s2
*symbol-file
*symbol-file -readnow cos.elf
*cont

@break entry_efi_s1
@bt
@set tmp=1
@cont
