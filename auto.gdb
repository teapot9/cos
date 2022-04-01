#b efistub_memmap_and_exit
set tmp=1
#b serial_init
b dobreak
b kernel_main
#b kernel/sched/main.c:18
#b kthread_new
#b thread_init
#b spintest
b thread_kill
#b dotest
#b isr_handler
c
