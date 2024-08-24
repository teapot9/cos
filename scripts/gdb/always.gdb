source -v scripts/gdb/helpers.py

bt
break _kbreak
break isr_handler if frame->interrupt < 32
break panic
break halt
