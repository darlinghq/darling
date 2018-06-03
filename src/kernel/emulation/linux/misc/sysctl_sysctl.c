#include "sysctl_sysctl.h"
#include <lkm/api.h>
#include <mach/machine.h>
#include <sys/errno.h>

extern int lkm_call(int call, void*);

enum {
	_PROC_CPUTYPE = 1000,
};

static sysctl_handler(handle_proc_cputype);

const struct known_sysctl sysctls_sysctl[] = {
	{ .oid = _PROC_CPUTYPE, .type = CTLTYPE_INT, .exttype = "I", .name = "proc_cputype", .handler = handle_proc_cputype },
	{ .oid = -1 }
};

sysctl_handler(handle_proc_cputype)
{
	if (nlen < 3)
		return -ENOENT;

	int pid = name[2];
	int is64bit = lkm_call(NR_task_64bit, (void*)(long)pid);

	if (is64bit < 0)
		return -ENOENT;
	else
	{
		if (*oldlen < sizeof(int))
			return -EINVAL;

#if defined(__i386__) || defined(__x86_64__)
		*((unsigned int*) old) = CPU_TYPE_I386;
#elif defined(__arm__) || defined(__aarch64__)
		*((unsigned int*) old) = CPU_TYPE_ARM;
#else
#	warning Missing code for this arch!
		*((unsigned int*) old) = 0;
#endif

		if (is64bit)
			*((unsigned int*) old) |= CPU_ARCH_ABI64;
	}

	return 0;
}

