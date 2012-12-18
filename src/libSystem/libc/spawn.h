#ifndef LIBC_SPAWN_H
#define LIBC_SPAWN_H
#include <spawn.h>
#include "signals.h"

extern "C" {

// sizeof on Darwin: ptr size
// sizeof on Linux (x86-64): 80 bytes
struct __darwin_posix_spawn_file_actions_t
{
	posix_spawn_file_actions_t* native;
};

struct __darwin_posix_spawnattr_t
{
	posix_spawnattr_t* native;
};

int __darwin_posix_spawn_file_actions_destroy(__darwin_posix_spawn_file_actions_t* file_actions);
int __darwin_posix_spawn_file_actions_init(__darwin_posix_spawn_file_actions_t* file_actions);

int __darwin_posix_spawn_file_actions_addclose(__darwin_posix_spawn_file_actions_t* file_actions, int filedes);
int __darwin_posix_spawn_file_actions_addopen(__darwin_posix_spawn_file_actions_t* file_actions, int filedes, const char* path, int oflag, mode_t mode);
int __darwin_posix_spawn_file_actions_adddup2(__darwin_posix_spawn_file_actions_t* file_actions, int filedes, int newfiledes);
int __darwin_posix_spawn_file_actions_addinherit_np(__darwin_posix_spawn_file_actions_t* file_actions, int filedes);

int __darwin_posix_spawnattr_destroy(__darwin_posix_spawnattr_t* attr);
int __darwin_posix_spawnattr_init(__darwin_posix_spawnattr_t* attr);

int __darwin_posix_spawnattr_getsigdefault(const __darwin_posix_spawnattr_t* attr, __darwin_sigset_t* sigdefault);
int __darwin_posix_spawnattr_setsigdefault(__darwin_posix_spawnattr_t* attr, const __darwin_sigset_t* sigdefault);
int __darwin_posix_spawnattr_getsigmask(const __darwin_posix_spawnattr_t* attr, __darwin_sigset_t* sigmask);
int __darwin_posix_spawnattr_setsigmask(__darwin_posix_spawnattr_t* attr, const __darwin_sigset_t* sigmask);
int __darwin_posix_spawnattr_getflags(const __darwin_posix_spawnattr_t* attr, short* flags);
int __darwin_posix_spawnattr_setflags(__darwin_posix_spawnattr_t* attr, short flags);
int __darwin_posix_spawnattr_getpgroup(const __darwin_posix_spawnattr_t* attr, pid_t* pgroup);
int __darwin_posix_spawnattr_setpgroup(__darwin_posix_spawnattr_t *attr, pid_t pgroup);
int __darwin_posix_spawnattr_getschedparam(const __darwin_posix_spawnattr_t* attr, struct sched_param* schedparam);
int __darwin_posix_spawnattr_setschedparam(__darwin_posix_spawnattr_t* attr, const struct sched_param* schedparam);
int __darwin_posix_spawnattr_getschedpolicy(const __darwin_posix_spawnattr_t* attr, int* schedpolicy);
int __darwin_posix_spawnattr_setschedpolicy(__darwin_posix_spawnattr_t *attr, int schedpolicy);

int __darwin_posix_spawn(pid_t* pid, const char* path, const __darwin_posix_spawn_file_actions_t* file_actions, const __darwin_posix_spawnattr_t* attrp,
		char* const argv[], char* const envp[]);
int __darwin_posix_spawnp(pid_t* pid, const char* file, const __darwin_posix_spawn_file_actions_t* file_actions,const __darwin_posix_spawnattr_t* attrp,
		char* const argv[], char* const envp[]);

}

#endif

