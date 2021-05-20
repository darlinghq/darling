#include <darling/emulation/simple.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

#include "xtracelib.h"
#include "bsd_trace.h"

static void print_errno(char* buf, int nr, uintptr_t rv);
static void print_errno_num(char* buf, int nr, uintptr_t rv);
static void print_errno_ptr(char* buf, int nr, uintptr_t rv);

static void print_args(char* buf, int nr, void* args[]);

// awk '/^[0-9]/ { if ($6 !~ "nosys") { split($6, a, "("); print "[" $1 "] = { \"" a[1] "\", print_args, print_errno }," } }'

static const struct calldef bsd_defs[600] = {
	[1] = { "exit", print_args, print_errno_num },
	[2] = { "fork", print_args, print_errno_num },
	[3] = { "read", print_args, print_errno_num },
	[4] = { "write", print_args, print_errno_num },
	[5] = { "open", print_args, print_errno_num },
	[6] = { "close", print_args, print_errno_num },
	[7] = { "wait4", print_args, print_errno_num },
	[9] = { "link", print_args, print_errno_num },
	[10] = { "unlink", print_args, print_errno_num },
	[12] = { "chdir", print_args, print_errno_num },
	[13] = { "fchdir", print_args, print_errno_num },
	[14] = { "mknod", print_args, print_errno_num },
	[15] = { "chmod", print_args, print_errno_num },
	[16] = { "chown", print_args, print_errno_num },
	[18] = { "getfsstat", print_args, print_errno_num },
	[20] = { "getpid", print_args, print_errno_num },
	[23] = { "setuid", print_args, print_errno_num },
	[24] = { "getuid", print_args, print_errno_num },
	[25] = { "geteuid", print_args, print_errno_num },
	[26] = { "ptrace", print_args, print_errno_num },
	[27] = { "recvmsg", print_args, print_errno_num },
	[28] = { "sendmsg", print_args, print_errno_num },
	[29] = { "recvfrom", print_args, print_errno_num },
	[30] = { "accept", print_args, print_errno_num },
	[31] = { "getpeername", print_args, print_errno_num },
	[32] = { "getsockname", print_args, print_errno_num },
	[33] = { "access", print_args, print_errno_num },
	[34] = { "chflags", print_args, print_errno_num },
	[35] = { "fchflags", print_args, print_errno_num },
	[36] = { "sync", print_args, print_errno_num },
	[37] = { "kill", print_args, print_errno_num },
	[39] = { "getppid", print_args, print_errno_num },
	[41] = { "dup", print_args, print_errno_num },
	[42] = { "pipe", print_args, print_errno_num },
	[43] = { "getegid", print_args, print_errno_num },
	[46] = { "sigaction", print_args, print_errno_num },
	[47] = { "getgid", print_args, print_errno_num },
	[48] = { "sigprocmask", print_args, print_errno_num },
	[49] = { "getlogin", print_args, print_errno_num },
	[50] = { "setlogin", print_args, print_errno_num },
	[51] = { "acct", print_args, print_errno_num },
	[52] = { "sigpending", print_args, print_errno_num },
	[53] = { "sigaltstack", print_args, print_errno_num },
	[54] = { "ioctl", print_args, print_errno_num },
	[55] = { "reboot", print_args, print_errno_num },
	[56] = { "revoke", print_args, print_errno_num },
	[57] = { "symlink", print_args, print_errno_num },
	[58] = { "readlink", print_args, print_errno_num },
	[59] = { "execve", print_args, print_errno_num },
	[60] = { "umask", print_args, print_errno_num },
	[61] = { "chroot", print_args, print_errno_num },
	[65] = { "msync", print_args, print_errno_num },
	[66] = { "vfork", print_args, print_errno_num },
	[73] = { "munmap", print_args, print_errno_num },
	[74] = { "mprotect", print_args, print_errno_num },
	[75] = { "madvise", print_args, print_errno_num },
	[78] = { "mincore", print_args, print_errno_num },
	[79] = { "getgroups", print_args, print_errno_num },
	[80] = { "setgroups", print_args, print_errno_num },
	[81] = { "getpgrp", print_args, print_errno_num },
	[82] = { "setpgid", print_args, print_errno_num },
	[83] = { "setitimer", print_args, print_errno_num },
	[85] = { "swapon", print_args, print_errno_num },
	[86] = { "getitimer", print_args, print_errno_num },
	[89] = { "getdtablesize", print_args, print_errno_num },
	[90] = { "dup2", print_args, print_errno_num },
	[92] = { "fcntl", print_args, print_errno_num },
	[93] = { "select", print_args, print_errno_num },
	[95] = { "fsync", print_args, print_errno_num },
	[96] = { "setpriority", print_args, print_errno_num },
	[97] = { "socket", print_args, print_errno_num },
	[98] = { "connect", print_args, print_errno_num },
	[100] = { "getpriority", print_args, print_errno_num },
	[104] = { "bind", print_args, print_errno_num },
	[105] = { "setsockopt", print_args, print_errno_num },
	[106] = { "listen", print_args, print_errno_num },
	[111] = { "sigsuspend", print_args, print_errno_num },
	[116] = { "gettimeofday", print_args, print_errno_num },
	[117] = { "getrusage", print_args, print_errno_num },
	[118] = { "getsockopt", print_args, print_errno_num },
	[120] = { "readv", print_args, print_errno_num },
	[121] = { "writev", print_args, print_errno_num },
	[122] = { "settimeofday", print_args, print_errno_num },
	[123] = { "fchown", print_args, print_errno_num },
	[124] = { "fchmod", print_args, print_errno_num },
	[126] = { "setreuid", print_args, print_errno_num },
	[127] = { "setregid", print_args, print_errno_num },
	[128] = { "rename", print_args, print_errno_num },
	[131] = { "flock", print_args, print_errno_num },
	[132] = { "mkfifo", print_args, print_errno_num },
	[133] = { "sendto", print_args, print_errno_num },
	[134] = { "shutdown", print_args, print_errno_num },
	[135] = { "socketpair", print_args, print_errno_num },
	[136] = { "mkdir", print_args, print_errno_num },
	[137] = { "rmdir", print_args, print_errno_num },
	[138] = { "utimes", print_args, print_errno_num },
	[139] = { "futimes", print_args, print_errno_num },
	[140] = { "adjtime", print_args, print_errno_num },
	[142] = { "gethostuuid", print_args, print_errno_num },
	[147] = { "setsid", print_args, print_errno_num },
	[151] = { "getpgid", print_args, print_errno_num },
	[152] = { "setprivexec", print_args, print_errno_num },
	[153] = { "pread", print_args, print_errno_num },
	[154] = { "pwrite", print_args, print_errno_num },
	[155] = { "nfssvc", print_args, print_errno_num },
	[157] = { "statfs", print_args, print_errno_num },
	[158] = { "fstatfs", print_args, print_errno_num },
	[159] = { "unmount", print_args, print_errno_num },
	[161] = { "getfh", print_args, print_errno_num },
	[165] = { "quotactl", print_args, print_errno_num },
	[167] = { "mount", print_args, print_errno_num },
	[169] = { "csops", print_args, print_errno_num },
	[170] = { "csops_audittoken", print_args, print_errno_num },
	[173] = { "waitid", print_args, print_errno_num },
	[177] = { "kdebug_typefilter", print_args, print_errno_num },
	[178] = { "kdebug_trace_string", print_args, print_errno_num },
	[179] = { "kdebug_trace64", print_args, print_errno_num },
	[180] = { "kdebug_trace", print_args, print_errno_num },
	[181] = { "setgid", print_args, print_errno_num },
	[182] = { "setegid", print_args, print_errno_num },
	[183] = { "seteuid", print_args, print_errno_num },
	[184] = { "sigreturn", print_args, print_errno_num },
	[187] = { "fdatasync", print_args, print_errno_num },
	[188] = { "stat", print_args, print_errno_num },
	[189] = { "fstat", print_args, print_errno_num },
	[190] = { "lstat", print_args, print_errno_num },
	[191] = { "pathconf", print_args, print_errno_num },
	[192] = { "fpathconf", print_args, print_errno_num },
	[194] = { "getrlimit", print_args, print_errno_num },
	[195] = { "setrlimit", print_args, print_errno_num },
	[196] = { "getdirentries", print_args, print_errno_num },
	[197] = { "mmap", print_args, print_errno_ptr },
	[199] = { "lseek", print_args, print_errno_num },
	[200] = { "truncate", print_args, print_errno_num },
	[201] = { "ftruncate", print_args, print_errno_num },
	[202] = { "sysctl", print_args, print_errno_num },
	[203] = { "mlock", print_args, print_errno_num },
	[204] = { "munlock", print_args, print_errno_num },
	[205] = { "undelete", print_args, print_errno_num },
	[216] = { "open_dprotected_np", print_args, print_errno_num },
	[220] = { "getattrlist", print_args, print_errno_num },
	[221] = { "setattrlist", print_args, print_errno_num },
	[222] = { "getdirentriesattr", print_args, print_errno_num },
	[223] = { "exchangedata", print_args, print_errno_num },
	[225] = { "searchfs", print_args, print_errno_num },
	[226] = { "delete", print_args, print_errno_num },
	[227] = { "copyfile", print_args, print_errno_num },
	[228] = { "fgetattrlist", print_args, print_errno_num },
	[229] = { "fsetattrlist", print_args, print_errno_num },
	[230] = { "poll", print_args, print_errno_num },
	[231] = { "watchevent", print_args, print_errno_num },
	[232] = { "waitevent", print_args, print_errno_num },
	[233] = { "modwatch", print_args, print_errno_num },
	[234] = { "getxattr", print_args, print_errno_num },
	[235] = { "fgetxattr", print_args, print_errno_num },
	[236] = { "setxattr", print_args, print_errno_num },
	[237] = { "fsetxattr", print_args, print_errno_num },
	[238] = { "removexattr", print_args, print_errno_num },
	[239] = { "fremovexattr", print_args, print_errno_num },
	[240] = { "listxattr", print_args, print_errno_num },
	[241] = { "flistxattr", print_args, print_errno_num },
	[242] = { "fsctl", print_args, print_errno_num },
	[243] = { "initgroups", print_args, print_errno_num },
	[244] = { "posix_spawn", print_args, print_errno_num },
	[245] = { "ffsctl", print_args, print_errno_num },
	[247] = { "nfsclnt", print_args, print_errno_num },
	[248] = { "fhopen", print_args, print_errno_num },
	[250] = { "minherit", print_args, print_errno_num },
	[251] = { "semsys", print_args, print_errno_num },
	[252] = { "msgsys", print_args, print_errno_num },
	[253] = { "shmsys", print_args, print_errno_num },
	[254] = { "semctl", print_args, print_errno_num },
	[255] = { "semget", print_args, print_errno_num },
	[256] = { "semop", print_args, print_errno_num },
	[258] = { "msgctl", print_args, print_errno_num },
	[259] = { "msgget", print_args, print_errno_num },
	[260] = { "msgsnd", print_args, print_errno_num },
	[261] = { "msgrcv", print_args, print_errno_num },
	[262] = { "shmat", print_args, print_errno_num },
	[263] = { "shmctl", print_args, print_errno_num },
	[264] = { "shmdt", print_args, print_errno_num },
	[265] = { "shmget", print_args, print_errno_num },
	[266] = { "shm_open", print_args, print_errno_num },
	[267] = { "shm_unlink", print_args, print_errno_num },
	[268] = { "sem_open", print_args, print_errno_num },
	[269] = { "sem_close", print_args, print_errno_num },
	[270] = { "sem_unlink", print_args, print_errno_num },
	[271] = { "sem_wait", print_args, print_errno_num },
	[272] = { "sem_trywait", print_args, print_errno_num },
	[273] = { "sem_post", print_args, print_errno_num },
	[274] = { "sysctlbyname", print_args, print_errno_num },
	[277] = { "open_extended", print_args, print_errno_num },
	[278] = { "umask_extended", print_args, print_errno_num },
	[279] = { "stat_extended", print_args, print_errno_num },
	[280] = { "lstat_extended", print_args, print_errno_num },
	[281] = { "fstat_extended", print_args, print_errno_num },
	[282] = { "chmod_extended", print_args, print_errno_num },
	[283] = { "fchmod_extended", print_args, print_errno_num },
	[284] = { "access_extended", print_args, print_errno_num },
	[285] = { "settid", print_args, print_errno_num },
	[286] = { "gettid", print_args, print_errno_num },
	[287] = { "setsgroups", print_args, print_errno_num },
	[288] = { "getsgroups", print_args, print_errno_num },
	[289] = { "setwgroups", print_args, print_errno_num },
	[290] = { "getwgroups", print_args, print_errno_num },
	[291] = { "mkfifo_extended", print_args, print_errno_num },
	[292] = { "mkdir_extended", print_args, print_errno_num },
	[293] = { "identitysvc", print_args, print_errno_num },
	[294] = { "shared_region_check_np", print_args, print_errno_num },
	[296] = { "vm_pressure_monitor", print_args, print_errno_num },
	[297] = { "psynch_rw_longrdlock", print_args, print_errno_num },
	[298] = { "psynch_rw_yieldwrlock", print_args, print_errno_num },
	[299] = { "psynch_rw_downgrade", print_args, print_errno_num },
	[300] = { "psynch_rw_upgrade", print_args, print_errno_num },
	[301] = { "psynch_mutexwait", print_args, print_errno_num },
	[302] = { "psynch_mutexdrop", print_args, print_errno_num },
	[303] = { "psynch_cvbroad", print_args, print_errno_num },
	[304] = { "psynch_cvsignal", print_args, print_errno_num },
	[305] = { "psynch_cvwait", print_args, print_errno_num },
	[306] = { "psynch_rw_rdlock", print_args, print_errno_num },
	[307] = { "psynch_rw_wrlock", print_args, print_errno_num },
	[308] = { "psynch_rw_unlock", print_args, print_errno_num },
	[309] = { "psynch_rw_unlock2", print_args, print_errno_num },
	[310] = { "getsid", print_args, print_errno_num },
	[311] = { "settid_with_pid", print_args, print_errno_num },
	[312] = { "psynch_cvclrprepost", print_args, print_errno_num },
	[313] = { "aio_fsync", print_args, print_errno_num },
	[314] = { "aio_return", print_args, print_errno_num },
	[315] = { "aio_suspend", print_args, print_errno_num },
	[316] = { "aio_cancel", print_args, print_errno_num },
	[317] = { "aio_error", print_args, print_errno_num },
	[318] = { "aio_read", print_args, print_errno_num },
	[319] = { "aio_write", print_args, print_errno_num },
	[320] = { "lio_listio", print_args, print_errno_num },
	[322] = { "iopolicysys", print_args, print_errno_num },
	[323] = { "process_policy", print_args, print_errno_num },
	[324] = { "mlockall", print_args, print_errno_num },
	[325] = { "munlockall", print_args, print_errno_num },
	[327] = { "issetugid", print_args, print_errno_num },
	[328] = { "__pthread_kill", print_args, print_errno_num },
	[329] = { "__pthread_sigmask", print_args, print_errno_num },
	[330] = { "__sigwait", print_args, print_errno_num },
	[331] = { "__disable_threadsignal", print_args, print_errno_num },
	[332] = { "__pthread_markcancel", print_args, print_errno_num },
	[333] = { "__pthread_canceled", print_args, print_errno_num },
	[334] = { "__semwait_signal", print_args, print_errno_num },
	[336] = { "proc_info", print_args, print_errno_num },
	[337] = { "sendfile", print_args, print_errno_num },
	[338] = { "stat64", print_args, print_errno_num },
	[339] = { "fstat64", print_args, print_errno_num },
	[340] = { "lstat64", print_args, print_errno_num },
	[341] = { "stat64_extended", print_args, print_errno_num },
	[342] = { "lstat64_extended", print_args, print_errno_num },
	[343] = { "fstat64_extended", print_args, print_errno_num },
	[344] = { "getdirentries64", print_args, print_errno_num },
	[345] = { "statfs64", print_args, print_errno_num },
	[346] = { "fstatfs64", print_args, print_errno_num },
	[347] = { "getfsstat64", print_args, print_errno_num },
	[348] = { "__pthread_chdir", print_args, print_errno_num },
	[349] = { "__pthread_fchdir", print_args, print_errno_num },
	[350] = { "audit", print_args, print_errno_num },
	[351] = { "auditon", print_args, print_errno_num },
	[353] = { "getauid", print_args, print_errno_num },
	[354] = { "setauid", print_args, print_errno_num },
	[357] = { "getaudit_addr", print_args, print_errno_num },
	[358] = { "setaudit_addr", print_args, print_errno_num },
	[359] = { "auditctl", print_args, print_errno_num },
	[360] = { "bsdthread_create", print_args, print_errno_num },
	[361] = { "bsdthread_terminate", print_args, print_errno_num },
	[362] = { "kqueue", print_args, print_errno_num },
	[363] = { "kevent", print_args, print_errno_num },
	[364] = { "lchown", print_args, print_errno_num },
	[366] = { "bsdthread_register", print_args, print_errno_num },
	[367] = { "workq_open", print_args, print_errno_num },
	[368] = { "workq_kernreturn", print_args, print_errno_num },
	[369] = { "kevent64", print_args, print_errno_num },
	[370] = { "__old_semwait_signal", print_args, print_errno_num },
	[371] = { "__old_semwait_signal_nocancel", print_args, print_errno_num },
	[372] = { "thread_selfid", print_args, print_errno_num },
	[373] = { "ledger", print_args, print_errno_num },
	[374] = { "kevent_qos", print_args, print_errno_num },
	[380] = { "__mac_execve", print_args, print_errno_num },
	[381] = { "__mac_syscall", print_args, print_errno_num },
	[382] = { "__mac_get_file", print_args, print_errno_num },
	[383] = { "__mac_set_file", print_args, print_errno_num },
	[384] = { "__mac_get_link", print_args, print_errno_num },
	[385] = { "__mac_set_link", print_args, print_errno_num },
	[386] = { "__mac_get_proc", print_args, print_errno_num },
	[387] = { "__mac_set_proc", print_args, print_errno_num },
	[388] = { "__mac_get_fd", print_args, print_errno_num },
	[389] = { "__mac_set_fd", print_args, print_errno_num },
	[390] = { "__mac_get_pid", print_args, print_errno_num },
	[394] = { "pselect", print_args, print_errno_num },
	[395] = { "pselect_nocancel", print_args, print_errno_num },
	[396] = { "read_nocancel", print_args, print_errno_num },
	[397] = { "write_nocancel", print_args, print_errno_num },
	[398] = { "open_nocancel", print_args, print_errno_num },
	[399] = { "close_nocancel", print_args, print_errno_num },
	[400] = { "wait4_nocancel", print_args, print_errno_num },
	[401] = { "recvmsg_nocancel", print_args, print_errno_num },
	[402] = { "sendmsg_nocancel", print_args, print_errno_num },
	[403] = { "recvfrom_nocancel", print_args, print_errno_num },
	[404] = { "accept_nocancel", print_args, print_errno_num },
	[405] = { "msync_nocancel", print_args, print_errno_num },
	[406] = { "fcntl_nocancel", print_args, print_errno_num },
	[407] = { "select_nocancel", print_args, print_errno_num },
	[408] = { "fsync_nocancel", print_args, print_errno_num },
	[409] = { "connect_nocancel", print_args, print_errno_num },
	[410] = { "sigsuspend_nocancel", print_args, print_errno_num },
	[411] = { "readv_nocancel", print_args, print_errno_num },
	[412] = { "writev_nocancel", print_args, print_errno_num },
	[413] = { "sendto_nocancel", print_args, print_errno_num },
	[414] = { "pread_nocancel", print_args, print_errno_num },
	[415] = { "pwrite_nocancel", print_args, print_errno_num },
	[416] = { "waitid_nocancel", print_args, print_errno_num },
	[417] = { "poll_nocancel", print_args, print_errno_num },
	[418] = { "msgsnd_nocancel", print_args, print_errno_num },
	[419] = { "msgrcv_nocancel", print_args, print_errno_num },
	[420] = { "sem_wait_nocancel", print_args, print_errno_num },
	[421] = { "aio_suspend_nocancel", print_args, print_errno_num },
	[422] = { "__sigwait_nocancel", print_args, print_errno_num },
	[423] = { "__semwait_signal_nocancel", print_args, print_errno_num },
	[424] = { "__mac_mount", print_args, print_errno_num },
	[425] = { "__mac_get_mount", print_args, print_errno_num },
	[426] = { "__mac_getfsstat", print_args, print_errno_num },
	[427] = { "fsgetpath", print_args, print_errno_num },
	[428] = { "audit_session_self", print_args, print_errno_num },
	[429] = { "audit_session_join", print_args, print_errno_num },
	[430] = { "fileport_makeport", print_args, print_errno_num },
	[431] = { "fileport_makefd", print_args, print_errno_num },
	[432] = { "audit_session_port", print_args, print_errno_num },
	[433] = { "pid_suspend", print_args, print_errno_num },
	[434] = { "pid_resume", print_args, print_errno_num },
	[438] = { "shared_region_map_and_slide_np", print_args, print_errno_num },
	[439] = { "kas_info", print_args, print_errno_num },
	[440] = { "memorystatus_control", print_args, print_errno_num },
	[441] = { "guarded_open_np", print_args, print_errno_num },
	[442] = { "guarded_close_np", print_args, print_errno_num },
	[443] = { "guarded_kqueue_np", print_args, print_errno_num },
	[444] = { "change_fdguard_np", print_args, print_errno_num },
	[445] = { "usrctl", print_args, print_errno_num },
	[446] = { "proc_rlimit_control", print_args, print_errno_num },
	[447] = { "connectx", print_args, print_errno_num },
	[448] = { "disconnectx", print_args, print_errno_num },
	[449] = { "peeloff", print_args, print_errno_num },
	[450] = { "socket_delegate", print_args, print_errno_num },
	[451] = { "telemetry", print_args, print_errno_num },
	[452] = { "proc_uuid_policy", print_args, print_errno_num },
	[453] = { "memorystatus_get_level", print_args, print_errno_num },
	[454] = { "system_override", print_args, print_errno_num },
	[455] = { "vfs_purge", print_args, print_errno_num },
	[456] = { "sfi_ctl", print_args, print_errno_num },
	[457] = { "sfi_pidctl", print_args, print_errno_num },
	[458] = { "coalition", print_args, print_errno_num },
	[459] = { "coalition_info", print_args, print_errno_num },
	[460] = { "necp_match_policy", print_args, print_errno_num },
	[461] = { "getattrlistbulk", print_args, print_errno_num },
	[462] = { "clonefileat", print_args, print_errno_num },
	[463] = { "openat", print_args, print_errno_num },
	[464] = { "openat_nocancel", print_args, print_errno_num },
	[465] = { "renameat", print_args, print_errno_num },
	[466] = { "faccessat", print_args, print_errno_num },
	[467] = { "fchmodat", print_args, print_errno_num },
	[468] = { "fchownat", print_args, print_errno_num },
	[469] = { "fstatat", print_args, print_errno_num },
	[470] = { "fstatat64", print_args, print_errno_num },
	[471] = { "linkat", print_args, print_errno_num },
	[472] = { "unlinkat", print_args, print_errno_num },
	[473] = { "readlinkat", print_args, print_errno_num },
	[474] = { "symlinkat", print_args, print_errno_num },
	[475] = { "mkdirat", print_args, print_errno_num },
	[476] = { "getattrlistat", print_args, print_errno_num },
	[477] = { "proc_trace_log", print_args, print_errno_num },
	[478] = { "bsdthread_ctl", print_args, print_errno_num },
	[479] = { "openbyid_np", print_args, print_errno_num },
	[480] = { "recvmsg_x", print_args, print_errno_num },
	[481] = { "sendmsg_x", print_args, print_errno_num },
	[482] = { "thread_selfusage", print_args, print_errno_num },
	[483] = { "csrctl", print_args, print_errno_num },
	[484] = { "guarded_open_dprotected_np", print_args, print_errno_num },
	[485] = { "guarded_write_np", print_args, print_errno_num },
	[486] = { "guarded_pwrite_np", print_args, print_errno_num },
	[487] = { "guarded_writev_np", print_args, print_errno_num },
	[488] = { "renameatx_np", print_args, print_errno_num },
	[489] = { "mremap_encrypted", print_args, print_errno_num },
	[490] = { "netagent_trigger", print_args, print_errno_num },
	[491] = { "stack_snapshot_with_config", print_args, print_errno_num },
	[492] = { "microstackshot", print_args, print_errno_num },
	[493] = { "grab_pgo_data", print_args, print_errno_num },
	[494] = { "persona", print_args, print_errno_num },
	[499] = { "work_interval_ctl", print_args, print_errno_num },
	[500] = { "getentropy", print_args, print_errno_num },
	[501] = { "necp_open", print_args, print_errno_num },
	[502] = { "necp_client_action", print_args, print_errno_num },
	[515] = { "ulock_wait", print_args, print_errno_num },
	[516] = { "ulock_wake", print_args, print_errno_num },
	[517] = { "fclonefileat", print_args, print_errno_num },
	[518] = { "fs_snapshot", print_args, print_errno_num },
	[520] = { "terminate_with_payload", print_args, print_errno_num },
	[521] = { "abort_with_payload", print_args, print_errno_num },
};

static int print_arg_int(char* buf, void* arg)
{
	return __simple_sprintf(buf, "%d", (int) (long) arg);
}

static int print_arg_ptr(char* buf, void* arg)
{
	if (arg == NULL)
		return __simple_sprintf(buf, "NULL");
	return __simple_sprintf(buf, "%p", arg);
}

int xtrace_format_string_literal(char* buf, const char* str)
{
	const char* initial_buf = buf;

	if (str == NULL)
	{
		return __simple_sprintf(buf, "NULL");
	}

	if (!xtrace_no_color)
		buf += __simple_sprintf(buf, "\033[;1m"); // bold

	*buf++ = '"';

	for (; *str; str++)
	{
		switch (*str)
		{
			case '\\':
				*buf++ = '\\';
				*buf++ = '\\';
				break;
			case '\n':
				*buf++ = '\\';
				*buf++ = 'n';
				break;
			case '\t':
				*buf++ = '\\';
				*buf++ = 't';
				break;
			default:
				*buf++ = *str;
				break;
		}
	}

	*buf++ = '"';

	if (!xtrace_no_color)
		buf += __simple_sprintf(buf, "\033[0m"); // reset

	return buf - initial_buf;
}

static int print_arg_str(char* buf, void* arg)
{
	const char* str = (const char*) arg;
	return xtrace_format_string_literal(buf, str);
}

static int print_arg_prot(char* buf, void* arg)
{
	const char* initial_buf = buf;
	int cnt = 0;
	int prot = (int) (long) arg;

	if (prot & PROT_READ)
	{
		buf += __simple_sprintf(buf, "PROT_READ");
		cnt++;
	}
	if (prot & PROT_WRITE)
	{
		if (cnt > 0)
			*buf++ = '|';
		buf += __simple_sprintf(buf, "PROT_WRITE");
		cnt++;
	}
	if (prot & PROT_EXEC)
	{
		if (cnt > 0)
			*buf++ = '|';
		buf += __simple_sprintf(buf, "PROT_EXEC");
		cnt++;
	}
	if (cnt == 0)
	{
		buf += __simple_sprintf(buf, "PROT_NONE");
	}

	return buf - initial_buf;
}

static int print_mmap_flags(char* buf, void* arg)
{
	const char* initial_buf = buf;
	int cnt = 0;
	int flags = (int) (long) arg;

	static const struct {
		int flag;
		const char* name;
	} all_flags[] = {
		{ MAP_ANON, "MAP_ANON" },
		{ MAP_FIXED, "MAP_FIXED" },
		{ MAP_HASSEMAPHORE, "MAP_HASSEMAPHORE" },
		{ MAP_PRIVATE, "MAP_PRIVATE" },
		{ MAP_SHARED, "MAP_SHARED" },
		{ MAP_NOCACHE, "MAP_NOCACHE" },
		{ 0, NULL }
	};

	for (int i = 0; all_flags[i].name != NULL; i++)
	{
		if (flags & all_flags[i].flag)
		{
			if (cnt > 0)
				*buf++ = '|';
			buf += __simple_sprintf(buf, "%s", all_flags[i].name);
			cnt++;
		}
	}

	if (cnt == 0)
	{
		buf += __simple_sprintf(buf, "MAP_FILE");
	}

	return buf - initial_buf;
}

static int print_open_flags(char* buf, void* arg)
{
	const char* initial_buf = buf;
	int cnt = 0;
	int flags = (int) (long) arg;

	struct {
		int flag;
		const char* name;
	} all_flags[] = {
		{ O_WRONLY, "O_WRONLY" },
		{ O_RDWR, "O_RDWR" },
		{ O_NONBLOCK, "O_NONBLOCK" },
		{ O_APPEND, "O_APPEND" },
		{ O_CREAT, "O_CREAT" },
		{ O_TRUNC, "O_TRUNC" },
		{ O_EXCL, "O_EXCL" },
		{ O_SHLOCK, "O_SHLOCK" },
		{ O_EXLOCK, "O_EXLOCK" },
		{ O_NOFOLLOW, "O_NOFOLLOW" },
		{ O_SYMLINK, "O_SYMLINK" },
		{ O_DIRECTORY, "O_DIRECTORY" },
		{ O_EVTONLY, "O_EVTONLY" },
		{ O_CLOEXEC, "O_CLOEXEC" },
		{ O_NOCTTY, "O_NOCTTY" },
		{ 0, NULL }
	};

	for (int i = 0; all_flags[i].name != NULL; i++)
	{
		if (flags & all_flags[i].flag)
		{
			if (cnt > 0)
				*buf++ = '|';
			buf += __simple_sprintf(buf, "%s", all_flags[i].name);
			cnt++;
		}
	}

	if (cnt == 0)
	{
		buf += __simple_sprintf(buf, "O_RDONLY");
	}

	return buf - initial_buf;
}

static const struct {
	int args_cnt;
	int (*print_arg[8])(char* buf, void* arg);
} args_info[] = {
	[1] = { 1, { print_arg_int } }, // exit
	[2] = { 0, {  } }, // fork
	[3] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // read
	[4] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // write
	[5] = { 3, { print_arg_str, print_open_flags, print_arg_int } }, // open
	[6] = { 1, { print_arg_int } }, // close
	[7] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // wait4
	[9] = { 2, { print_arg_str, print_arg_str } }, // link
	[10] = { 1, { print_arg_str } }, // unlink
	[12] = { 1, { print_arg_str } }, // chdir
	[13] = { 1, { print_arg_int } }, // fchdir
	[14] = { 3, { print_arg_str, print_arg_int, print_arg_int } }, // mknod
	[15] = { 2, { print_arg_str, print_arg_int } }, // chmod
	[16] = { 3, { print_arg_str, print_arg_int, print_arg_int } }, // chown
	[18] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // getfsstat
	[20] = { 0, {  } }, // getpid
	[23] = { 1, { print_arg_int } }, // setuid
	[24] = { 0, {  } }, // getuid
	[25] = { 0, {  } }, // geteuid
	[26] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // ptrace
	[27] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // recvmsg
	[28] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // sendmsg
	[29] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr } }, // recvfrom
	[30] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // accept
	[31] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // getpeername
	[32] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // getsockname
	[33] = { 2, { print_arg_str, print_arg_int } }, // access
	[34] = { 2, { print_arg_str, print_arg_int } }, // chflags
	[35] = { 2, { print_arg_int, print_arg_int } }, // fchflags
	[36] = { 0, {  } }, // sync
	[37] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // kill
	[39] = { 0, {  } }, // getppid
	[41] = { 1, { print_arg_int } }, // dup
	[42] = { 0, {  } }, // pipe
	[43] = { 0, {  } }, // getegid
	[46] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // sigaction
	[47] = { 0, {  } }, // getgid
	[48] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // sigprocmask
	[49] = { 2, { print_arg_ptr, print_arg_int } }, // getlogin
	[50] = { 1, { print_arg_ptr } }, // setlogin
	[51] = { 1, { print_arg_str } }, // acct
	[52] = { 1, { print_arg_ptr } }, // sigpending
	[53] = { 2, { print_arg_ptr, print_arg_ptr } }, // sigaltstack
	[54] = { 3, { print_arg_int, print_arg_int, print_arg_ptr } }, // ioctl
	[55] = { 2, { print_arg_int, print_arg_ptr } }, // reboot
	[56] = { 1, { print_arg_str } }, // revoke
	[57] = { 2, { print_arg_str, print_arg_str } }, // symlink
	[58] = { 3, { print_arg_str, print_arg_ptr, print_arg_int } }, // readlink
	[59] = { 3, { print_arg_str, print_arg_ptr, print_arg_ptr } }, // execve
	[60] = { 1, { print_arg_int } }, // umask
	[61] = { 1, { print_arg_str } }, // chroot
	[65] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // msync
	[66] = { 0, {  } }, // vfork
	[73] = { 2, { print_arg_ptr, print_arg_int } }, // munmap
	[74] = { 3, { print_arg_ptr, print_arg_int, print_arg_prot } }, // mprotect
	[75] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // madvise
	[78] = { 3, { print_arg_ptr, print_arg_int, print_arg_ptr } }, // mincore
	[79] = { 2, { print_arg_int, print_arg_ptr } }, // getgroups
	[80] = { 2, { print_arg_int, print_arg_ptr } }, // setgroups
	[81] = { 0, {  } }, // getpgrp
	[82] = { 2, { print_arg_int, print_arg_int } }, // setpgid
	[83] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // setitimer
	[85] = { 0, {  } }, // swapon
	[86] = { 2, { print_arg_int, print_arg_ptr } }, // getitimer
	[89] = { 0, {  } }, // getdtablesize
	[90] = { 2, { print_arg_int, print_arg_int } }, // dup2
	[92] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // fcntl
	[93] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // select
	[95] = { 1, { print_arg_int } }, // fsync
	[96] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // setpriority
	[97] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // socket
	[98] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // connect
	[100] = { 2, { print_arg_int, print_arg_int } }, // getpriority
	[104] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // bind
	[105] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // setsockopt
	[106] = { 2, { print_arg_int, print_arg_int } }, // listen
	[111] = { 1, { print_arg_int } }, // sigsuspend
	[116] = { 3, { print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // gettimeofday
	[117] = { 2, { print_arg_int, print_arg_ptr } }, // getrusage
	[118] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr } }, // getsockopt
	[120] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // readv
	[121] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // writev
	[122] = { 2, { print_arg_ptr, print_arg_ptr } }, // settimeofday
	[123] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // fchown
	[124] = { 2, { print_arg_int, print_arg_int } }, // fchmod
	[126] = { 2, { print_arg_int, print_arg_int } }, // setreuid
	[127] = { 2, { print_arg_int, print_arg_int } }, // setregid
	[128] = { 2, { print_arg_str, print_arg_str } }, // rename
	[131] = { 2, { print_arg_int, print_arg_int } }, // flock
	[132] = { 2, { print_arg_str, print_arg_int } }, // mkfifo
	[133] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // sendto
	[134] = { 2, { print_arg_int, print_arg_int } }, // shutdown
	[135] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // socketpair
	[136] = { 2, { print_arg_str, print_arg_int } }, // mkdir
	[137] = { 1, { print_arg_str } }, // rmdir
	[138] = { 2, { print_arg_str, print_arg_ptr } }, // utimes
	[139] = { 2, { print_arg_int, print_arg_ptr } }, // futimes
	[140] = { 2, { print_arg_ptr, print_arg_ptr } }, // adjtime
	[142] = { 3, { print_arg_ptr, print_arg_ptr, print_arg_int } }, // gethostuuid
	[147] = { 0, {  } }, // setsid
	[151] = { 1, { print_arg_int } }, // getpgid
	[152] = { 1, { print_arg_int } }, // setprivexec
	[153] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // pread
	[154] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // pwrite
	[155] = { 2, { print_arg_int, print_arg_ptr } }, // nfssvc
	[157] = { 2, { print_arg_str, print_arg_ptr } }, // statfs
	[158] = { 2, { print_arg_int, print_arg_ptr } }, // fstatfs
	[159] = { 2, { print_arg_str, print_arg_int } }, // unmount
	[161] = { 2, { print_arg_ptr, print_arg_ptr } }, // getfh
	[165] = { 4, { print_arg_str, print_arg_int, print_arg_int, print_arg_ptr } }, // quotactl
	[167] = { 4, { print_arg_str, print_arg_str, print_arg_int, print_arg_ptr } }, // mount
	[169] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // csops
	[170] = { 5, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // csops_audittoken
	[173] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // waitid
	[177] = { 2, { print_arg_ptr, print_arg_ptr } }, // kdebug_typefilter
	[178] = { 3, { print_arg_int, print_arg_int, print_arg_str } }, // kdebug_trace_string
	[179] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // kdebug_trace64
	[180] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // kdebug_trace
	[181] = { 1, { print_arg_int } }, // setgid
	[182] = { 1, { print_arg_int } }, // setegid
	[183] = { 1, { print_arg_int } }, // seteuid
	[184] = { 2, { print_arg_ptr, print_arg_int } }, // sigreturn
	[187] = { 1, { print_arg_int } }, // fdatasync
	[188] = { 2, { print_arg_str, print_arg_ptr } }, // stat
	[189] = { 2, { print_arg_int, print_arg_ptr } }, // fstat
	[190] = { 2, { print_arg_str, print_arg_ptr } }, // lstat
	[191] = { 2, { print_arg_str, print_arg_int } }, // pathconf
	[192] = { 2, { print_arg_int, print_arg_int } }, // fpathconf
	[194] = { 2, { print_arg_int, print_arg_ptr } }, // getrlimit
	[195] = { 2, { print_arg_int, print_arg_ptr } }, // setrlimit
	[196] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // getdirentries
	[197] = { 6, { print_arg_ptr, print_arg_int, print_arg_prot, print_mmap_flags, print_arg_int, print_arg_int } }, // mmap
	[199] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // lseek
	[200] = { 2, { print_arg_str, print_arg_int } }, // truncate
	[201] = { 2, { print_arg_int, print_arg_int } }, // ftruncate
	[202] = { 6, { print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_int } }, // sysctl
	[203] = { 2, { print_arg_ptr, print_arg_int } }, // mlock
	[204] = { 2, { print_arg_ptr, print_arg_int } }, // munlock
	[205] = { 1, { print_arg_str } }, // undelete
	[216] = { 5, { print_arg_str, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // open_dprotected_np
	[220] = { 5, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // getattrlist
	[221] = { 5, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // setattrlist
	[222] = { 8, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_int } }, // getdirentriesattr
	[223] = { 3, { print_arg_str, print_arg_str, print_arg_int } }, // exchangedata
	[225] = { 6, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr } }, // searchfs
	[226] = { 1, { print_arg_str } }, // delete
	[227] = { 4, { print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // copyfile
	[228] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // fgetattrlist
	[229] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // fsetattrlist
	[230] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // poll
	[231] = { 2, { print_arg_ptr, print_arg_int } }, // watchevent
	[232] = { 2, { print_arg_ptr, print_arg_ptr } }, // waitevent
	[233] = { 2, { print_arg_ptr, print_arg_int } }, // modwatch
	[234] = { 6, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // getxattr
	[235] = { 6, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // fgetxattr
	[236] = { 6, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // setxattr
	[237] = { 6, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // fsetxattr
	[238] = { 3, { print_arg_str, print_arg_ptr, print_arg_int } }, // removexattr
	[239] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // fremovexattr
	[240] = { 4, { print_arg_str, print_arg_ptr, print_arg_int, print_arg_int } }, // listxattr
	[241] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // flistxattr
	[242] = { 4, { print_arg_str, print_arg_int, print_arg_ptr, print_arg_int } }, // fsctl
	[243] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // initgroups
	[244] = { 5, { print_arg_ptr, print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // posix_spawn
	[245] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // ffsctl
	[247] = { 2, { print_arg_int, print_arg_ptr } }, // nfsclnt
	[248] = { 2, { print_arg_ptr, print_arg_int } }, // fhopen
	[250] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // minherit
	[251] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // semsys
	[252] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // msgsys
	[253] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // shmsys
	[254] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // semctl
	[255] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // semget
	[256] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // semop
	[258] = { 3, { print_arg_int, print_arg_int, print_arg_ptr } }, // msgctl
	[259] = { 2, { print_arg_int, print_arg_int } }, // msgget
	[260] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // msgsnd
	[261] = { 5, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // msgrcv
	[262] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // shmat
	[263] = { 3, { print_arg_int, print_arg_int, print_arg_ptr } }, // shmctl
	[264] = { 1, { print_arg_ptr } }, // shmdt
	[265] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // shmget
	[266] = { 3, { print_arg_str, print_arg_int, print_arg_int } }, // shm_open
	[267] = { 1, { print_arg_str } }, // shm_unlink
	[268] = { 4, { print_arg_str, print_arg_int, print_arg_int, print_arg_int } }, // sem_open
	[269] = { 1, { print_arg_ptr } }, // sem_close
	[270] = { 1, { print_arg_str } }, // sem_unlink
	[271] = { 1, { print_arg_ptr } }, // sem_wait
	[272] = { 1, { print_arg_ptr } }, // sem_trywait
	[273] = { 1, { print_arg_ptr } }, // sem_post
	[274] = { 6, { print_arg_str, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_int } }, // sysctlbyname
	[277] = { 6, { print_arg_str, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // open_extended
	[278] = { 2, { print_arg_int, print_arg_ptr } }, // umask_extended
	[279] = { 4, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // stat_extended
	[280] = { 4, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // lstat_extended
	[281] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // fstat_extended
	[282] = { 5, { print_arg_str, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // chmod_extended
	[283] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // fchmod_extended
	[284] = { 4, { print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // access_extended
	[285] = { 2, { print_arg_int, print_arg_int } }, // settid
	[286] = { 2, { print_arg_ptr, print_arg_ptr } }, // gettid
	[287] = { 2, { print_arg_int, print_arg_ptr } }, // setsgroups
	[288] = { 2, { print_arg_ptr, print_arg_ptr } }, // getsgroups
	[289] = { 2, { print_arg_int, print_arg_ptr } }, // setwgroups
	[290] = { 2, { print_arg_ptr, print_arg_ptr } }, // getwgroups
	[291] = { 5, { print_arg_str, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // mkfifo_extended
	[292] = { 5, { print_arg_str, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // mkdir_extended
	[293] = { 2, { print_arg_int, print_arg_ptr } }, // identitysvc
	[294] = { 1, { print_arg_ptr } }, // shared_region_check_np
	[296] = { 3, { print_arg_int, print_arg_int, print_arg_ptr } }, // vm_pressure_monitor
	[297] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_longrdlock
	[298] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_yieldwrlock
	[299] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_downgrade
	[300] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_upgrade
	[301] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_mutexwait
	[302] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_mutexdrop
	[303] = { 7, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // psynch_cvbroad
	[304] = { 8, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // psynch_cvsignal
	[305] = { 8, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_cvwait
	[306] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_rdlock
	[307] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_wrlock
	[308] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_unlock
	[309] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_rw_unlock2
	[310] = { 1, { print_arg_int } }, // getsid
	[311] = { 2, { print_arg_int, print_arg_int } }, // settid_with_pid
	[312] = { 7, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // psynch_cvclrprepost
	[313] = { 2, { print_arg_int, print_arg_ptr } }, // aio_fsync
	[314] = { 1, { print_arg_ptr } }, // aio_return
	[315] = { 3, { print_arg_ptr, print_arg_int, print_arg_ptr } }, // aio_suspend
	[316] = { 2, { print_arg_int, print_arg_ptr } }, // aio_cancel
	[317] = { 1, { print_arg_ptr } }, // aio_error
	[318] = { 1, { print_arg_ptr } }, // aio_read
	[319] = { 1, { print_arg_ptr } }, // aio_write
	[320] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // lio_listio
	[322] = { 2, { print_arg_int, print_arg_ptr } }, // iopolicysys
	[323] = { 7, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // process_policy
	[324] = { 1, { print_arg_int } }, // mlockall
	[325] = { 1, { print_arg_int } }, // munlockall
	[327] = { 0, {  } }, // issetugid
	[328] = { 2, { print_arg_int, print_arg_int } }, // __pthread_kill
	[329] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // __pthread_sigmask
	[330] = { 2, { print_arg_ptr, print_arg_ptr } }, // __sigwait
	[331] = { 1, { print_arg_int } }, // __disable_threadsignal
	[332] = { 1, { print_arg_int } }, // __pthread_markcancel
	[333] = { 1, { print_arg_int } }, // __pthread_canceled
	[334] = { 6, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // __semwait_signal
	[336] = { 6, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // proc_info
	[337] = { 6, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int } }, // sendfile
	[338] = { 2, { print_arg_str, print_arg_ptr } }, // stat64
	[339] = { 2, { print_arg_int, print_arg_ptr } }, // fstat64
	[340] = { 2, { print_arg_str, print_arg_ptr } }, // lstat64
	[341] = { 4, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // stat64_extended
	[342] = { 4, { print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // lstat64_extended
	[343] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // fstat64_extended
	[344] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // getdirentries64
	[345] = { 2, { print_arg_str, print_arg_ptr } }, // statfs64
	[346] = { 2, { print_arg_int, print_arg_ptr } }, // fstatfs64
	[347] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // getfsstat64
	[348] = { 1, { print_arg_str } }, // __pthread_chdir
	[349] = { 1, { print_arg_int } }, // __pthread_fchdir
	[350] = { 2, { print_arg_ptr, print_arg_int } }, // audit
	[351] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // auditon
	[353] = { 1, { print_arg_ptr } }, // getauid
	[354] = { 1, { print_arg_ptr } }, // setauid
	[357] = { 2, { print_arg_ptr, print_arg_int } }, // getaudit_addr
	[358] = { 2, { print_arg_ptr, print_arg_int } }, // setaudit_addr
	[359] = { 1, { print_arg_str } }, // auditctl
	[360] = { 5, { print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_int } }, // bsdthread_create
	[361] = { 4, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // bsdthread_terminate
	[362] = { 0, {  } }, // kqueue
	[363] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // kevent
	[364] = { 3, { print_arg_str, print_arg_int, print_arg_int } }, // lchown
	[366] = { 7, { print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // bsdthread_register
	[367] = { 0, {  } }, // workq_open
	[368] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // workq_kernreturn
	[369] = { 7, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr } }, // kevent64
	[370] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // __old_semwait_signal
	[371] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // __old_semwait_signal_nocancel
	[372] = { 0, {  } }, // thread_selfid
	[373] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // ledger
	[374] = { 8, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int } }, // kevent_qos
	[380] = { 4, { print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // __mac_execve
	[381] = { 3, { print_arg_ptr, print_arg_int, print_arg_ptr } }, // __mac_syscall
	[382] = { 2, { print_arg_ptr, print_arg_ptr } }, // __mac_get_file
	[383] = { 2, { print_arg_ptr, print_arg_ptr } }, // __mac_set_file
	[384] = { 2, { print_arg_ptr, print_arg_ptr } }, // __mac_get_link
	[385] = { 2, { print_arg_ptr, print_arg_ptr } }, // __mac_set_link
	[386] = { 1, { print_arg_ptr } }, // __mac_get_proc
	[387] = { 1, { print_arg_ptr } }, // __mac_set_proc
	[388] = { 2, { print_arg_int, print_arg_ptr } }, // __mac_get_fd
	[389] = { 2, { print_arg_int, print_arg_ptr } }, // __mac_set_fd
	[390] = { 2, { print_arg_int, print_arg_ptr } }, // __mac_get_pid
	[394] = { 6, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // pselect
	[395] = { 6, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // pselect_nocancel
	[396] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // read_nocancel
	[397] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // write_nocancel
	[398] = { 3, { print_arg_str, print_open_flags, print_arg_int } }, // open_nocancel
	[399] = { 1, { print_arg_int } }, // close_nocancel
	[400] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // wait4_nocancel
	[401] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // recvmsg_nocancel
	[402] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // sendmsg_nocancel
	[403] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr } }, // recvfrom_nocancel
	[404] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // accept_nocancel
	[405] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // msync_nocancel
	[406] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // fcntl_nocancel
	[407] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // select_nocancel
	[408] = { 1, { print_arg_int } }, // fsync_nocancel
	[409] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // connect_nocancel
	[410] = { 1, { print_arg_int } }, // sigsuspend_nocancel
	[411] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // readv_nocancel
	[412] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // writev_nocancel
	[413] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // sendto_nocancel
	[414] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // pread_nocancel
	[415] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // pwrite_nocancel
	[416] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // waitid_nocancel
	[417] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // poll_nocancel
	[418] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // msgsnd_nocancel
	[419] = { 5, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // msgrcv_nocancel
	[420] = { 1, { print_arg_ptr } }, // sem_wait_nocancel
	[421] = { 3, { print_arg_ptr, print_arg_int, print_arg_ptr } }, // aio_suspend_nocancel
	[422] = { 2, { print_arg_ptr, print_arg_ptr } }, // __sigwait_nocancel
	[423] = { 6, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // __semwait_signal_nocancel
	[424] = { 5, { print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr } }, // __mac_mount
	[425] = { 2, { print_arg_ptr, print_arg_ptr } }, // __mac_get_mount
	[426] = { 5, { print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // __mac_getfsstat
	[427] = { 4, { print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // fsgetpath
	[428] = { 0, {  } }, // audit_session_self
	[429] = { 1, { print_arg_int } }, // audit_session_join
	[430] = { 2, { print_arg_int, print_arg_ptr } }, // fileport_makeport
	[431] = { 1, { print_arg_int } }, // fileport_makefd
	[432] = { 2, { print_arg_int, print_arg_ptr } }, // audit_session_port
	[433] = { 1, { print_arg_int } }, // pid_suspend
	[434] = { 1, { print_arg_int } }, // pid_resume
	[438] = { 6, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // shared_region_map_and_slide_np
	[439] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // kas_info
	[440] = { 5, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // memorystatus_control
	[441] = { 5, { print_arg_str, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int } }, // guarded_open_np
	[442] = { 2, { print_arg_int, print_arg_ptr } }, // guarded_close_np
	[443] = { 2, { print_arg_ptr, print_arg_int } }, // guarded_kqueue_np
	[444] = { 6, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr } }, // change_fdguard_np
	[445] = { 1, { print_arg_int } }, // usrctl
	[446] = { 3, { print_arg_int, print_arg_int, print_arg_ptr } }, // proc_rlimit_control
	[447] = { 8, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_ptr } }, // connectx
	[448] = { 3, { print_arg_int, print_arg_int, print_arg_int } }, // disconnectx
	[449] = { 2, { print_arg_int, print_arg_int } }, // peeloff
	[450] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // socket_delegate
	[451] = { 6, { print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // telemetry
	[452] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // proc_uuid_policy
	[453] = { 1, { print_arg_ptr } }, // memorystatus_get_level
	[454] = { 2, { print_arg_int, print_arg_int } }, // system_override
	[455] = { 0, {  } }, // vfs_purge
	[456] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // sfi_ctl
	[457] = { 4, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr } }, // sfi_pidctl
	[458] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // coalition
	[459] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // coalition_info
	[460] = { 3, { print_arg_ptr, print_arg_int, print_arg_ptr } }, // necp_match_policy
	[461] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // getattrlistbulk
	[462] = { 5, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // clonefileat
	[463] = { 4, { print_arg_int, print_arg_str, print_open_flags, print_arg_int } }, // openat
	[464] = { 4, { print_arg_int, print_arg_str, print_open_flags, print_arg_int } }, // openat_nocancel
	[465] = { 4, { print_arg_int, print_arg_str, print_arg_int, print_arg_str } }, // renameat
	[466] = { 4, { print_arg_int, print_arg_str, print_arg_int, print_arg_int } }, // faccessat
	[467] = { 4, { print_arg_int, print_arg_str, print_arg_int, print_arg_int } }, // fchmodat
	[468] = { 5, { print_arg_int, print_arg_str, print_arg_int, print_arg_int, print_arg_int } }, // fchownat
	[469] = { 4, { print_arg_int, print_arg_str, print_arg_ptr, print_arg_int } }, // fstatat
	[470] = { 4, { print_arg_int, print_arg_str, print_arg_ptr, print_arg_int } }, // fstatat64
	[471] = { 5, { print_arg_int, print_arg_str, print_arg_int, print_arg_str, print_arg_int } }, // linkat
	[472] = { 3, { print_arg_int, print_arg_str, print_arg_int } }, // unlinkat
	[473] = { 4, { print_arg_int, print_arg_str, print_arg_ptr, print_arg_int } }, // readlinkat
	[474] = { 3, { print_arg_str, print_arg_int, print_arg_str } }, // symlinkat
	[475] = { 3, { print_arg_int, print_arg_str, print_arg_int } }, // mkdirat
	[476] = { 6, { print_arg_int, print_arg_str, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // getattrlistat
	[477] = { 2, { print_arg_int, print_arg_int } }, // proc_trace_log
	[478] = { 4, { print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // bsdthread_ctl
	[479] = { 3, { print_arg_ptr, print_arg_ptr, print_arg_int } }, // openbyid_np
	[480] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // recvmsg_x
	[481] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // sendmsg_x
	[482] = { 0, {  } }, // thread_selfusage
	[483] = { 3, { print_arg_int, print_arg_ptr, print_arg_ptr } }, // csrctl
	[484] = { 7, { print_arg_str, print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // guarded_open_dprotected_np
	[485] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int } }, // guarded_write_np
	[486] = { 5, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int, print_arg_int } }, // guarded_pwrite_np
	[487] = { 4, { print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_int } }, // guarded_writev_np
	[488] = { 5, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // renameatx_np
	[489] = { 5, { print_arg_ptr, print_arg_int, print_arg_int, print_arg_int, print_arg_int } }, // mremap_encrypted
	[490] = { 2, { print_arg_ptr, print_arg_int } }, // netagent_trigger
	[491] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // stack_snapshot_with_config
	[492] = { 3, { print_arg_ptr, print_arg_int, print_arg_int } }, // microstackshot
	[493] = { 4, { print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // grab_pgo_data
	[494] = { 5, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr } }, // persona
	[499] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // work_interval_ctl
	[500] = { 2, { print_arg_ptr, print_arg_int } }, // getentropy
	[501] = { 1, { print_arg_int } }, // necp_open
	[502] = { 6, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_ptr, print_arg_int } }, // necp_client_action
	[515] = { 4, { print_arg_int, print_arg_ptr, print_arg_int, print_arg_int } }, // ulock_wait
	[516] = { 3, { print_arg_int, print_arg_ptr, print_arg_int } }, // ulock_wake
	[517] = { 4, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int } }, // fclonefileat
	[518] = { 6, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_ptr, print_arg_ptr, print_arg_int } }, // fs_snapshot
	[520] = { 7, { print_arg_int, print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_str, print_arg_int } }, // terminate_with_payload
	[521] = { 6, { print_arg_int, print_arg_int, print_arg_ptr, print_arg_int, print_arg_str, print_arg_int } }, // abort_with_payload
};

static void print_args(char* buf, int nr, void* args[])
{
	int cnt = args_info[nr].args_cnt;
	for (int i = 0; i < cnt; i++)
	{
		if (i > 0)
			buf += __simple_sprintf(buf, ", ");
		buf += (*args_info[nr].print_arg[i])(buf, args[i]);
	}
	*buf = 0;
}


extern "C"
void darling_bsd_syscall_entry_print(int nr, void* args[])
{
	handle_generic_entry(bsd_defs, "bsd", nr, args);

	if (nr == 1 || nr == 59)
	{
		// For exit() or execve(), print an extra newline,
		// as we're likely not going to see the return.
		xtrace_printf("\n");
	}
}

extern "C"
void darling_bsd_syscall_exit_print(uintptr_t retval)
{
	handle_generic_exit(bsd_defs, "bsd", retval, 0);
}

const char* error_strings[128] = {
	[1] = "EPERM",
	[2] = "ENOENT",
	[3] = "ESRCH",
	[4] = "EINTR",
	[5] = "EIO",
	[6] = "ENXIO",
	[7] = "E2BIG",
	[8] = "ENOEXEC",
	[9] = "EBADF",
	[10] = "ECHILD",
	[11] = "EDEADLK",
	[12] = "ENOMEM",
	[13] = "EACCES",
	[14] = "EFAULT",
	[15] = "ENOTBLK",
	[16] = "EBUSY",
	[17] = "EEXIST",
	[18] = "EXDEV",
	[19] = "ENODEV",
	[20] = "ENOTDIR",
	[21] = "EISDIR",
	[22] = "EINVAL",
	[23] = "ENFILE",
	[24] = "EMFILE",
	[25] = "ENOTTY",
	[26] = "ETXTBSY",
	[27] = "EFBIG",
	[28] = "ENOSPC",
	[29] = "ESPIPE",
	[30] = "EROFS",
	[31] = "EMLINK",
	[32] = "EPIPE",
	[33] = "EDOM",
	[34] = "ERANGE",
	[35] = "EAGAIN",
	[36] = "EINPROGRESS",
	[37] = "EALREADY",
	[38] = "ENOTSOCK",
	[39] = "EDESTADDRREQ",
	[40] = "EMSGSIZE",
	[41] = "EPROTOTYPE",
	[42] = "ENOPROTOOPT",
	[43] = "EPROTONOSUPPORT",
	[44] = "ESOCKTNOSUPPORT",
	[45] = "ENOTSUP",
	[46] = "EPFNOSUPPORT",
	[47] = "EAFNOSUPPORT",
	[48] = "EADDRINUSE",
	[49] = "EADDRNOTAVAIL",
	[50] = "ENETDOWN",
	[51] = "ENETUNREACH",
	[52] = "ENETRESET",
	[53] = "ECONNABORTED",
	[54] = "ECONNRESET",
	[55] = "ENOBUFS",
	[56] = "EISCONN",
	[57] = "ENOTCONN",
	[58] = "ESHUTDOWN",
	[59] = "ETOOMANYREFS",
	[60] = "ETIMEDOUT",
	[61] = "ECONNREFUSED",
	[62] = "ELOOP",
	[63] = "ENAMETOOLONG",
	[64] = "EHOSTDOWN",
	[65] = "EHOSTUNREACH",
	[66] = "ENOTEMPTY",
	[67] = "EPROCLIM",
	[68] = "EUSERS",
	[69] = "EDQUOT",
	[70] = "ESTALE",
	[71] = "EREMOTE",
	[72] = "EBADRPC",
	[73] = "ERPCMISMATCH",
	[74] = "EPROGUNAVAIL",
	[75] = "EPROGMISMATCH",
	[76] = "EPROCUNAVAIL",
	[77] = "ENOLCK",
	[78] = "ENOSYS",
	[79] = "EFTYPE",
	[80] = "EAUTH",
	[81] = "ENEEDAUTH",
	[82] = "EPWROFF",
	[83] = "EDEVERR",
	[84] = "EOVERFLOW",
	[85] = "EBADEXEC",
	[86] = "EBADARCH",
	[87] = "ESHLIBVERS",
	[88] = "EBADMACHO",
	[89] = "ECANCELED",
	[90] = "EIDRM",
	[91] = "ENOMSG",
	[92] = "EILSEQ",
	[93] = "ENOATTR",
	[94] = "EBADMSG",
	[95] = "EMULTIHOP",
	[96] = "ENODATA",
	[97] = "ENOLINK",
	[98] = "ENOSR",
	[99] = "ENOSTR",
	[100] = "EPROTO",
	[101] = "ETIME",
	[102] = "EOPNOTSUPP",
	[103] = "ENOPOLICY",
	[104] = "ENOTRECOVERABLE",
	[105] = "EOWNERDEAD",
	[106] = "EQFULL",
};

static void print_errno_num(char* buf, int nr, uintptr_t rv)
{
	intptr_t v = (intptr_t)rv;
	if (v >= 0 || v < -4095)
	{
		__simple_sprintf(buf, "%ld", rv);
	}
	else
		print_errno(buf, nr, rv);
}

static void print_errno_ptr(char* buf, int nr, uintptr_t rv)
{
	intptr_t v = (intptr_t)rv;
	if (v >= 0 || v < -4095)
	{
		__simple_sprintf(buf, "%p", (void*) rv);
	}
	else
		print_errno(buf, nr, rv);
}

static void print_errno(char* buf, int nr, uintptr_t rv)
{
	const char* error = NULL;
	intptr_t v = (intptr_t) rv;
	if (-v < 128)
		error = error_strings[-v];
	if (error != NULL)
		__simple_sprintf(buf, "%s", error);
	else
		__simple_sprintf(buf, "%ld", v);
}

