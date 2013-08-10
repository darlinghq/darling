#include "spawn.h"
#include "errno.h"
#include "../common/auto.h"
#include "../kernel-bsd/io.h"
#include "darwin_errno_codes.h"
#include "exec.h"
#include <type_traits>
#include "MachO.h"
#include "trace.h"

template<typename NativeInit, typename T> int GenericInit(T* t, NativeInit nativeInit)
{
	int err;

	if (!t)
		return DARWIN_EINVAL;

	t->native = new typename std::remove_reference<decltype(*t->native)>::type;
	err = AutoErrnoPosix(nativeInit, t->native);

	if (err)
		delete t->native;
	return err;
}

template<typename NativeDestroy, typename T> int GenericDestroy(T* t, NativeDestroy nativeDestroy)
{
	if (t != nullptr)
	{
		int err = AutoErrnoPosix(nativeDestroy, t->native);
		delete t->native;
		return err;
	}
	else
		return DARWIN_EINVAL;
}

int __darwin_posix_spawn_file_actions_destroy(__darwin_posix_spawn_file_actions_t* file_actions)
{
	return GenericDestroy(file_actions, posix_spawn_file_actions_destroy);
}

int __darwin_posix_spawn_file_actions_init(__darwin_posix_spawn_file_actions_t* file_actions)
{
	return GenericInit(file_actions, posix_spawn_file_actions_init);
}

int __darwin_posix_spawn_file_actions_addclose(__darwin_posix_spawn_file_actions_t* file_actions, int filedes)
{
	return AutoErrnoPosix(posix_spawn_file_actions_addclose, file_actions ? file_actions->native : nullptr, filedes);
}

int __darwin_posix_spawn_file_actions_addopen(__darwin_posix_spawn_file_actions_t* file_actions, int filedes, const char* path, int oflag, mode_t mode)
{
	return AutoErrnoPosix(posix_spawn_file_actions_addopen, file_actions ? file_actions->native : nullptr,
			filedes, translatePathCI(path), Darling::openflagsDarwinToNative(oflag), mode);
}

int __darwin_posix_spawn_file_actions_adddup2(__darwin_posix_spawn_file_actions_t* file_actions, int filedes, int newfiledes)
{
	return AutoErrnoPosix(posix_spawn_file_actions_adddup2, file_actions ? file_actions->native : nullptr, filedes, newfiledes);
}

int __darwin_posix_spawn_file_actions_addinherit_np(__darwin_posix_spawn_file_actions_t* file_actions, int filedes)
{
	return DARWIN_ENOTSUP;
}

int __darwin_posix_spawnattr_destroy(__darwin_posix_spawnattr_t* attr)
{
	return GenericDestroy(attr, posix_spawnattr_destroy);
}

int __darwin_posix_spawnattr_init(__darwin_posix_spawnattr_t* attr)
{
	return GenericInit(attr, posix_spawnattr_init);
}

int __darwin_posix_spawnattr_getsigdefault(const __darwin_posix_spawnattr_t* attr, __darwin_sigset_t* sigdefault)
{
	sigset_t ns;
	int err = AutoErrnoPosix(posix_spawnattr_getsigdefault, attr ? attr->native : nullptr, &ns);

	if (!err)
		*sigdefault = Darling::sigsetLinuxToDarwin(&ns);

	return err;
}

int __darwin_posix_spawnattr_setsigdefault(__darwin_posix_spawnattr_t* attr, const __darwin_sigset_t* sigdefault)
{
	sigset_t ns;
	ns = Darling::sigsetDarwinToLinux(sigdefault);

	return AutoErrnoPosix(posix_spawnattr_setsigdefault, attr ? attr->native : nullptr, &ns);
}

int __darwin_posix_spawnattr_getsigmask(const __darwin_posix_spawnattr_t* attr, __darwin_sigset_t* sigmask)
{
	sigset_t ns;
	int err = AutoErrnoPosix(posix_spawnattr_getsigmask, attr ? attr->native : nullptr, &ns);

	if (!err)
		*sigmask = Darling::sigsetLinuxToDarwin(&ns);

	return err;
}

int __darwin_posix_spawnattr_setsigmask(__darwin_posix_spawnattr_t* attr, const __darwin_sigset_t* sigmask)
{
	sigset_t ns;
	ns = Darling::sigsetDarwinToLinux(sigmask);

	return AutoErrnoPosix(posix_spawnattr_setsigmask, attr ? attr->native : nullptr, &ns);
}


// flag values are compatible
int __darwin_posix_spawnattr_getflags(const __darwin_posix_spawnattr_t* attr, short* flags)
{
	return AutoErrnoPosix(posix_spawnattr_getflags, attr ? attr->native : nullptr, flags);
}

int __darwin_posix_spawnattr_setflags(__darwin_posix_spawnattr_t* attr, short flags)
{
	return AutoErrnoPosix(posix_spawnattr_setflags, attr ? attr->native : nullptr, flags);
}

int __darwin_posix_spawnattr_getpgroup(const __darwin_posix_spawnattr_t* attr, pid_t* pgroup)
{
	return AutoErrnoPosix(posix_spawnattr_getpgroup, attr ? attr->native : nullptr, pgroup);
}

int __darwin_posix_spawnattr_setpgroup(__darwin_posix_spawnattr_t *attr, pid_t pgroup)
{
	return AutoErrnoPosix(posix_spawnattr_setpgroup, attr ? attr->native : nullptr, pgroup);
}

int __darwin_posix_spawnattr_getschedparam(const __darwin_posix_spawnattr_t* attr, struct sched_param* schedparam)
{
	return AutoErrnoPosix(posix_spawnattr_getschedparam, attr ? attr->native : nullptr, schedparam);
}

int __darwin_posix_spawnattr_setschedparam(__darwin_posix_spawnattr_t* attr, const struct sched_param* schedparam)
{
	return AutoErrnoPosix(posix_spawnattr_setschedparam, attr ? attr->native : nullptr, schedparam);
}

int __darwin_posix_spawnattr_getschedpolicy(const __darwin_posix_spawnattr_t* attr, int* schedpolicy)
{
	return AutoErrnoPosix(posix_spawnattr_getschedpolicy, attr ? attr->native : nullptr, schedpolicy);
}

int __darwin_posix_spawnattr_setschedpolicy(__darwin_posix_spawnattr_t *attr, int schedpolicy)
{
	return AutoErrnoPosix(posix_spawnattr_setschedpolicy, attr ? attr->native : nullptr, schedpolicy);
}

int __darwin_posix_spawn(pid_t* pid, const char* path, const __darwin_posix_spawn_file_actions_t* file_actions, const __darwin_posix_spawnattr_t* attrp,
		        char* const argv[], char* const envp[])
{
	TRACE6(pid, path, file_actions, attrp, argv, envp);
	char* const* argv_copy = nullptr;
	int err;

	path = translatePathCI(path);

	if (MachO::isMachO(path))
	{
		argv = argv_copy = Darling::prependLoaderPath(argv, path);
		path = argv[0];
	}

	err = AutoErrnoPosix(posix_spawn, pid, path, file_actions ? file_actions->native : nullptr,
			attrp ? attrp->native : nullptr, argv, envp);

	delete [] argv_copy;
	return err;
}

int __darwin_posix_spawnp(pid_t* pid, const char* file, const __darwin_posix_spawn_file_actions_t* file_actions,const __darwin_posix_spawnattr_t* attrp,
		        char* const argv[], char* const envp[])
{
	TRACE6(pid, file, file_actions, attrp, argv, envp);
	char* const* argv_copy = nullptr;
	int err;

	const char* path = Darling::findInPath(file);
	if (!path)
		return DARWIN_ENOENT;

	if (MachO::isMachO(path))
		argv = argv_copy = Darling::prependLoaderPath(argv, path);

	err = AutoErrnoPosix(posix_spawn, pid, path, file_actions ? file_actions->native : nullptr,
			attrp ? attrp->native : nullptr, argv, envp);

	delete [] argv_copy;
	return err;
}


