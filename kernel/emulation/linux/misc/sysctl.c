#include "sysctl.h"
#include "../base.h"
#include "../errno.h"
#include "../debug.h"
#include <asm/unistd.h>
#include <mach/host_info.h>
#include <mach/machine.h>
#include <mach/mach_init.h>
#include "../../../../platform-include/sys/errno.h"
#include "sysctl_inc.h"
#include <stddef.h>
#include <limits.h>

static long sysctl_name_to_oid(const char* name, int* oid_name,
		unsigned long* oid_len);

extern char *strchr(const char *s, int c);
extern int strncmp(const char *s1, const char *s2, unsigned long n);
extern int strcmp(const char *s1, const char *s2);
extern kern_return_t mach_port_deallocate(ipc_space_t task, mach_port_name_t name);
extern kern_return_t host_info(mach_port_name_t host, int itype, void* hinfo, mach_msg_type_number_t* count);

/* Darling specific */
enum {
	_HW_PHYSICAL_CPU = 1000,
	_HW_PHYSICAL_CPU_MAX,
	_HW_LOGICAL_CPU,
	_HW_LOGICAL_CPU_MAX,
	_HW_CPUTYPE,
	_HW_CPUSUBTYPE,
	_HW_CPUTHREADTYPE
};

long sys_sysctl(int* name, unsigned int nlen, void* old,
		unsigned long* oldlen, void* _new, unsigned long newlen)
{
	int ret;

	if (nlen < 2)
		return -EINVAL;

	if (name[0] == 0 && name[1] == 3)
	{
		return sysctl_name_to_oid((const char*) _new, (int*) old, oldlen);
	}
	
	if (name[0] == CTL_HW)
	{
		int* ovalue = (int*) old;
		struct host_basic_info hinfo;
		mach_msg_type_number_t hcount = HOST_BASIC_INFO_COUNT;
		mach_port_t hself;
		unsigned long orig_old_len = *oldlen;

		hself = mach_host_self();
		host_info(hself, HOST_BASIC_INFO, &hinfo, &hcount);
		mach_port_deallocate(mach_task_self(), hself);

		if (nlen != 2)
			return -ENOTDIR;

		if (*oldlen < sizeof(int))
			return -EINVAL;
		*oldlen = sizeof(int);
		
		switch (name[1])
		{
		case HW_AVAILCPU:
		case HW_NCPU:
			*ovalue = hinfo.avail_cpus;
			return 0;
		case _HW_PHYSICAL_CPU:
			*ovalue = hinfo.physical_cpu;
			return 0;
		case _HW_PHYSICAL_CPU_MAX:
			*ovalue = hinfo.physical_cpu_max;
			return 0;
		case _HW_LOGICAL_CPU:
			*ovalue = hinfo.logical_cpu;
			return 0;
		case _HW_LOGICAL_CPU_MAX:
			*ovalue = hinfo.logical_cpu_max;
			return 0;
		case HW_MEMSIZE:
		case HW_PHYSMEM:
		case HW_USERMEM:
			if (orig_old_len == sizeof(int))
			{
				if (hinfo.max_mem < INT_MAX)
					*ovalue = hinfo.max_mem;
				else
					*ovalue = INT_MAX;
			}
			else
			{
				*oldlen = 2 * sizeof(int);
				*((unsigned long long*) ovalue) = hinfo.max_mem;
			}
			return 0;
		case HW_PAGESIZE:
			*ovalue = 4096; // True on all Darling platforms
			return 0;
		case _HW_CPUTYPE:
			*ovalue = hinfo.cpu_type;
			return 0;
		case _HW_CPUSUBTYPE:
			*ovalue = hinfo.cpu_subtype;
			return 0;
		case _HW_CPUTHREADTYPE:
			*ovalue = hinfo.cpu_threadtype;
			return 0;
		}
	}

	return -ENOTDIR;
}

static long sysctl_name_to_oid(const char* name, int* oid_name,
		unsigned long* oid_len)
{
	const char* dot;
	unsigned long cat_len;
	
	if (*oid_len < 2)
		return -EINVAL;

	dot = strchr(name, '.');
	if (dot == NULL)
		return -ENOTDIR;

	cat_len = dot - name;

	if (strncmp(name, "hw", cat_len) == 0)
	{
		oid_name[0] = CTL_HW;
		*oid_len = 2 * sizeof(int);
		
		if (strcmp(dot+1, "activecpu") == 0)
			oid_name[1] = HW_AVAILCPU;
		else if (strcmp(dot+1, "ncpu") == 0)
			oid_name[1] = HW_NCPU;
		else if (strcmp(dot+1, "physicalcpu") == 0)
			oid_name[1] = _HW_PHYSICAL_CPU;
		else if (strcmp(dot+1, "physicalcpu_max") == 0)
			oid_name[1] = _HW_PHYSICAL_CPU_MAX;
		else if (strcmp(dot+1, "logicalcpu") == 0)
			oid_name[1] = _HW_LOGICAL_CPU;
		else if (strcmp(dot+1, "logicalcpu_max") == 0)
			oid_name[1] = _HW_LOGICAL_CPU_MAX;
		else if (strcmp(dot+1, "memsize") == 0)
			oid_name[1] = HW_MEMSIZE;
		else if (strcmp(dot+1, "pagesize") == 0)
			oid_name[1] = HW_PAGESIZE;
		else if (strcmp(dot+1, "cputype") == 0)
			oid_name[1] = _HW_CPUTYPE;
		else if (strcmp(dot+1, "cpusubtype") == 0)
			oid_name[1] = _HW_CPUSUBTYPE;
		else if (strcmp(dot+1, "cputhreadtype") == 0)
			oid_name[1] = _HW_CPUTHREADTYPE;
		else
			return -ENOTDIR;
		
		return 0;
	}

	__simple_printf("Unknown sysctl: %s\n", name);
	return -ENOTDIR;
}

