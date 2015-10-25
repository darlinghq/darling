#include "syscalls.h"

#include "unistd/write.h"
#include "unistd/read.h"
#include "mman.h"

#include "unistd/sync.h"
#include "unistd/fsync.h"
#include "unistd/fdatasync.h"
#include "unistd/dup.h"
#include "unistd/dup2.h"
#include "unistd/exit.h"
#include "unistd/close.h"
#include "unistd/fchdir.h"
#include "unistd/fchown.h"
#include "unistd/fchmod.h"
#include "unistd/setgid.h"
#include "unistd/setegid.h"
#include "unistd/seteuid.h"
#include "unistd/setsid.h"
#include "unistd/setuid.h"
#include "unistd/getuid.h"
#include "unistd/geteuid.h"
#include "unistd/pread.h"
#include "unistd/pwrite.h"
#include "unistd/getpid.h"
#include "unistd/access.h"
#include "unistd/lseek.h"
#include "unistd/ftruncate.h"
#include "signal/kill.h"
#include "signal/sigaltstack.h"
#include "misc/ioctl.h"
#include "misc/getrlimit.h"
#include "misc/thread_selfid.h"
#include "misc/sysctl.h"
#include "fcntl/open.h"
#include "network/socket.h"
#include "network/connect.h"
#include "dirent/getdirentries.h"
#include "stat/fstat.h"
#include "stat/stat.h"
#include "time/gettimeofday.h"
#include "wqueue/bsdthread_register.h"

void* __bsd_syscall_table[512] = {
	[1] = sys_exit,
	[3] = sys_read,
	[4] = sys_write,
	[5] = sys_open,
	[6] = sys_close,
	[13] = sys_fchdir,
	[20] = sys_getpid,
	[23] = sys_setuid,
	[24] = sys_getuid,
	[25] = sys_geteuid,
	[33] = sys_access,
	[36] = sys_sync,
	[37] = sys_kill,
	[41] = sys_dup,
	[53] = sys_sigaltstack,
	[54] = sys_ioctl,
	[73] = sys_munmap,
	[74] = sys_mprotect,
	[78] = sys_mincore,
	[90] = sys_dup2,
	[95] = sys_fsync,
	[97] = sys_socket,
	[98] = sys_connect,
	[116] = sys_gettimeofday,
	[123] = sys_fchown,
	[124] = sys_fchmod,
	[147] = sys_setsid,
	[153] = sys_pread,
	[154] = sys_pwrite,
	[181] = sys_setgid,
	[182] = sys_setegid,
	[183] = sys_seteuid,
	[187] = sys_fdatasync,
	[188] = sys_stat,
	[189] = sys_fstat,
	[194] = sys_getrlimit,
	[196] = sys_getdirentries,
	[197] = sys_mmap,
	[199] = sys_lseek,
	[201] = sys_ftruncate,
	[202] = sys_sysctl,
	[338] = sys_stat64,
	[339] = sys_fstat64,
	[344] = sys_getdirentries64,
	[366] = sys_bsdthread_register,
	[372] = sys_thread_selfid,
	[396] = sys_read_nocancel,
	[397] = sys_write_nocancel,
	[398] = sys_open_nocancel,
	[399] = sys_close_nocancel,
	[408] = sys_fsync_nocancel,
	[414] = sys_pread_nocancel,
	[415] = sys_pwrite_nocancel,
};

