.globl accept$NOCANCEL
accept$NOCANCEL = __accept_nocancel
.globl aio_suspend$NOCANCEL
aio_suspend$NOCANCEL = __aio_suspend_nocancel
.globl close$NOCANCEL
close$NOCANCEL = __close_nocancel
.globl connect$NOCANCEL
connect$NOCANCEL = __connect_nocancel
.globl fstat$INODE64
fstat$INODE64 = __fstat64
.globl fstatat$INODE64
fstatat$INODE64 = __fstatat64
.globl fstatfs$INODE64
fstatfs$INODE64 = __fstatfs64
.globl fsync$NOCANCEL
fsync$NOCANCEL = __fsync_nocancel
.globl getfsstat$INODE64
getfsstat$INODE64 = __getfsstat64
.globl lstat$INODE64
lstat$INODE64 = __lstat64
.globl msgrcv$NOCANCEL
msgrcv$NOCANCEL = __msgrcv_nocancel
.globl msgsnd$NOCANCEL
msgsnd$NOCANCEL = __msgsnd_nocancel
.globl msgsys
msgsys = __msgsys
.globl msync$NOCANCEL
msync$NOCANCEL = __msync_nocancel
.globl open$NOCANCEL
open$NOCANCEL = __open_nocancel
.globl openat$NOCANCEL
openat$NOCANCEL = __openat_nocancel
.globl poll$NOCANCEL
poll$NOCANCEL = __poll_nocancel
.globl pread$NOCANCEL
pread$NOCANCEL = __pread_nocancel
.globl pwrite$NOCANCEL
pwrite$NOCANCEL = __pwrite_nocancel
.globl read$NOCANCEL
read$NOCANCEL = __read_nocancel
.globl readv$NOCANCEL
readv$NOCANCEL = __readv_nocancel
.globl recvfrom$NOCANCEL
recvfrom$NOCANCEL = __recvfrom_nocancel
.globl recvmsg$NOCANCEL
recvmsg$NOCANCEL = __recvmsg_nocancel
.globl select$DARWIN_EXTSN
select$DARWIN_EXTSN = __select
.globl select$DARWIN_EXTSN$NOCANCEL
select$DARWIN_EXTSN$NOCANCEL = __select_nocancel
.globl sem_wait$NOCANCEL
sem_wait$NOCANCEL = __sem_wait_nocancel
.globl semsys
semsys = __semsys
.globl sendmsg$NOCANCEL
sendmsg$NOCANCEL = __sendmsg_nocancel
.globl sendto$NOCANCEL
sendto$NOCANCEL = __sendto_nocancel
.globl stat$INODE64
stat$INODE64 = __stat64
.globl statfs$INODE64
statfs$INODE64 = __statfs64
.globl waitid$NOCANCEL
waitid$NOCANCEL = __waitid_nocancel
.globl write$NOCANCEL
write$NOCANCEL = __write_nocancel
.globl writev$NOCANCEL
writev$NOCANCEL = __writev_nocancel
.globl accept
accept = __accept
.globl bind
bind = __bind
.globl connect
connect = __connect
.globl getattrlist
getattrlist = __getattrlist
.globl getpeername
getpeername = __getpeername
.globl getsockname
getsockname = __getsockname
.globl lchown
lchown = __lchown
.globl listen
listen = __listen
.globl mprotect
mprotect = __mprotect
.globl msgctl
msgctl = __msgctl
.globl msync
msync = __msync
.globl open
open = __open
.globl openat
openat = __openat
.globl recvfrom
recvfrom = __recvfrom
.globl recvmsg
recvmsg = __recvmsg
.globl semctl
semctl = __semctl
.globl sem_open
sem_open = __sem_open
.globl sendmsg
sendmsg = __sendmsg
.globl sendto
sendto = __sendto
.globl setattrlist
setattrlist = __setattrlist
.globl setregid
setregid = __setregid
.globl setreuid
setreuid = __setreuid
.globl shmctl
shmctl = __shmctl
.globl shmsys
shmsys = __shmsys
.globl shm_open
shm_open = __shm_open
.globl socketpair
socketpair = __socketpair
