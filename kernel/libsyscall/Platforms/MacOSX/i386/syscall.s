.globl accept$NOCANCEL$UNIX2003
accept$NOCANCEL$UNIX2003 = __accept_nocancel
.globl accept$UNIX2003
accept$UNIX2003 = __accept
.globl aio_suspend
aio_suspend = __aio_suspend_nocancel
.globl aio_suspend$NOCANCEL$UNIX2003
aio_suspend$NOCANCEL$UNIX2003 = __aio_suspend_nocancel
.globl aio_suspend$UNIX2003
aio_suspend$UNIX2003 = __aio_suspend
.globl bind$UNIX2003
bind$UNIX2003 = __bind
.globl close
close = __close_nocancel
.globl close$NOCANCEL$UNIX2003
close$NOCANCEL$UNIX2003 = __close_nocancel
.globl close$UNIX2003
close$UNIX2003 = __close
.globl chmod
chmod = __chmod
.globl connect$NOCANCEL$UNIX2003
connect$NOCANCEL$UNIX2003 = __connect_nocancel
.globl connect$UNIX2003
connect$UNIX2003 = __connect
.globl fcntl
fcntl = __fcntl_nocancel
.globl fcntl$NOCANCEL$UNIX2003
fcntl$NOCANCEL$UNIX2003 = __fcntl_nocancel
.globl fcntl$UNIX2003
fcntl$UNIX2003 = __fcntl
.globl fstat$INODE64
fstat$INODE64 = __fstat64
.globl fstatat$INODE64
fstatat$INODE64 = __fstatat64
.globl fstatfs$INODE64
fstatfs$INODE64 = __fstatfs64
.globl fsync
fsync = __fsync_nocancel
.globl fsync$NOCANCEL$UNIX2003
fsync$NOCANCEL$UNIX2003 = __fsync_nocancel
.globl fsync$UNIX2003
fsync$UNIX2003 = __fsync
.globl getattrlist$UNIX2003
getattrlist$UNIX2003 = __getattrlist
.globl getfsstat$INODE64
getfsstat$INODE64 = __getfsstat64
.globl getpeername$UNIX2003
getpeername$UNIX2003 = __getpeername
.globl getsockname$UNIX2003
getsockname$UNIX2003 = __getsockname
.globl lchown$UNIX2003
lchown$UNIX2003 = __lchown
.globl listen$UNIX2003
listen$UNIX2003 = __listen
.globl lstat$INODE64
lstat$INODE64 = __lstat64
.globl mmap
mmap = __mmap
.globl mprotect$UNIX2003
mprotect$UNIX2003 = __mprotect
.globl msgctl$UNIX2003
msgctl$UNIX2003 = __msgctl
.globl msgrcv
msgrcv = __msgrcv_nocancel
.globl msgrcv$NOCANCEL$UNIX2003
msgrcv$NOCANCEL$UNIX2003 = __msgrcv_nocancel
.globl msgrcv$UNIX2003
msgrcv$UNIX2003 = __msgrcv
.globl msgsnd
msgsnd = __msgsnd_nocancel
.globl msgsnd$NOCANCEL$UNIX2003
msgsnd$NOCANCEL$UNIX2003 = __msgsnd_nocancel
.globl msgsnd$UNIX2003
msgsnd$UNIX2003 = __msgsnd
.globl msgsys
msgsys = __msgsys
.globl msync$NOCANCEL$UNIX2003
msync$NOCANCEL$UNIX2003 = __msync_nocancel
.globl msync$UNIX2003
msync$UNIX2003 = __msync
.globl open$NOCANCEL$UNIX2003
open$NOCANCEL$UNIX2003 = __open_nocancel
.globl open$UNIX2003
open$UNIX2003 = __open
.globl openat$NOCANCEL
openat$NOCANCEL = __openat_nocancel
.globl openat
openat = __openat
.globl poll
poll = __poll_nocancel
.globl poll$NOCANCEL$UNIX2003
poll$NOCANCEL$UNIX2003 = __poll_nocancel
.globl poll$UNIX2003
poll$UNIX2003 = __poll
.globl pread
pread = __pread_nocancel
.globl pread$NOCANCEL$UNIX2003
pread$NOCANCEL$UNIX2003 = __pread_nocancel
.globl pread$UNIX2003
pread$UNIX2003 = __pread
.globl pwrite
pwrite = __pwrite_nocancel
.globl pwrite$NOCANCEL$UNIX2003
pwrite$NOCANCEL$UNIX2003 = __pwrite_nocancel
.globl pwrite$UNIX2003
pwrite$UNIX2003 = __pwrite
.globl read
read = __read_nocancel
.globl read$NOCANCEL$UNIX2003
read$NOCANCEL$UNIX2003 = __read_nocancel
.globl read$UNIX2003
read$UNIX2003 = __read
.globl readv
readv = __readv_nocancel
.globl readv$NOCANCEL$UNIX2003
readv$NOCANCEL$UNIX2003 = __readv_nocancel
.globl readv$UNIX2003
readv$UNIX2003 = __readv
.globl recvfrom$NOCANCEL$UNIX2003
recvfrom$NOCANCEL$UNIX2003 = __recvfrom_nocancel
.globl recvfrom$UNIX2003
recvfrom$UNIX2003 = __recvfrom
.globl recvmsg$NOCANCEL$UNIX2003
recvmsg$NOCANCEL$UNIX2003 = __recvmsg_nocancel
.globl recvmsg$UNIX2003
recvmsg$UNIX2003 = __recvmsg
.globl select$DARWIN_EXTSN
select$DARWIN_EXTSN = __select
.globl select$DARWIN_EXTSN$NOCANCEL
select$DARWIN_EXTSN$NOCANCEL = __select_nocancel
.globl sem_open
sem_open = __sem_open
.globl sem_wait
sem_wait = __sem_wait_nocancel
.globl sem_wait$NOCANCEL$UNIX2003
sem_wait$NOCANCEL$UNIX2003 = __sem_wait_nocancel
.globl sem_wait$UNIX2003
sem_wait$UNIX2003 = __sem_wait
.globl semctl$UNIX2003
semctl$UNIX2003 = __semctl
.globl semsys
semsys = __semsys
.globl sendmsg$NOCANCEL$UNIX2003
sendmsg$NOCANCEL$UNIX2003 = __sendmsg_nocancel
.globl sendmsg$UNIX2003
sendmsg$UNIX2003 = __sendmsg
.globl sendto$NOCANCEL$UNIX2003
sendto$NOCANCEL$UNIX2003 = __sendto_nocancel
.globl sendto$UNIX2003
sendto$UNIX2003 = __sendto
.globl setattrlist$UNIX2003
setattrlist$UNIX2003 = __setattrlist
.globl setpgrp
setpgrp = __setpgid
.globl setregid$UNIX2003
setregid$UNIX2003 = __setregid
.globl setreuid$UNIX2003
setreuid$UNIX2003 = __setreuid
.globl shmctl$UNIX2003
shmctl$UNIX2003 = __shmctl
.globl shmsys
shmsys = __shmsys
.globl shm_open
shm_open = __shm_open
.globl socketpair$UNIX2003
socketpair$UNIX2003 = __socketpair
.globl stat$INODE64
stat$INODE64 = __stat64
.globl statfs$INODE64
statfs$INODE64 = __statfs64
.globl waitid
waitid = __waitid_nocancel
.globl waitid$NOCANCEL$UNIX2003
waitid$NOCANCEL$UNIX2003 = __waitid_nocancel
.globl waitid$UNIX2003
waitid$UNIX2003 = __waitid
.globl write
write = __write_nocancel
.globl write$NOCANCEL$UNIX2003
write$NOCANCEL$UNIX2003 = __write_nocancel
.globl write$UNIX2003
write$UNIX2003 = __write
.globl writev
writev = __writev_nocancel
.globl writev$NOCANCEL$UNIX2003
writev$NOCANCEL$UNIX2003 = __writev_nocancel
.globl writev$UNIX2003
writev$UNIX2003 = __writev
.globl ioctl
ioctl = __ioctl
.globl sigaltstack
sigaltstack = __sigaltstack
.globl fchmod
fchmod = __fchmod
.globl setrlimit
setrlimit = __setrlimit
.globl getrlimit
getrlimit = __getrlimit
