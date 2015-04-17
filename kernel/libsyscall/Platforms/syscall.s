.globl __sandbox_me
__sandbox_me = __mac_execve
.globl __sandbox_mm
__sandbox_mm = __mac_mount
.globl __sandbox_ms
__sandbox_ms = __mac_syscall
.globl __sandbox_msp
__sandbox_msp = __mac_set_proc
.globl _exit
_exit = __exit
.globl accessx_np
accessx_np = __access_extended
.globl getsgroups_np
getsgroups_np = __getsgroups
.globl getwgroups_np
getwgroups_np = __getwgroups
.globl posix_madvise
posix_madvise = __madvise
.globl pthread_getugid_np
pthread_getugid_np = __gettid
.globl pthread_setugid_np
pthread_setugid_np = __settid
.globl setsgroups_np
setsgroups_np = __setsgroups
.globl setwgroups_np
setwgroups_np = __setwgroups
.globl wait4
wait4 = __wait4_nocancel
