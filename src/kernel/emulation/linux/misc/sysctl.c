#include "sysctl.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include <linux-syscalls/linux.h>
#include <mach/host_info.h>
#include <mach/machine.h>
#include <mach/mach_init.h>
#include "../../../../../platform-include/sys/errno.h"
#include "sysctl_inc.h"
#include <stddef.h>
#include <limits.h>
#include "../ext/sys/utsname.h"
#include "../ext/sysinfo.h"
#include "../ext/syslog.h"
#include "../time/gettimeofday.h"
#include "getrlimit.h"
#include "darling-config.h"

// TODO: This file needs a rework to eliminate all the switch()es
// and replace them with lookup tables.

static long sysctl_name_to_oid(const char* name, int* oid_name,
		unsigned long* oid_len);
static long sysctl_get_type(const int* oid, int nlen, int* out, unsigned long* outlen);
static long sysctl_oid_to_name(const int* oid, int nlen, char* name, unsigned long* outlen);

extern char *strchr(const char *s, int c);
extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern int strcmp(const char *s1, const char *s2);
extern kern_return_t mach_port_deallocate(ipc_space_t task, mach_port_name_t name);
extern kern_return_t host_info(mach_port_name_t host, int itype, void* hinfo, mach_msg_type_number_t* count);

struct kinfo_proc;
extern int _sysctl_proc(int what, int flag, struct kinfo_proc* out, unsigned long* buflen);
extern int _sysctl_procargs(int pid, char* buf, unsigned long* buflen);
static void get_cpu_brand(char* brand);

extern int cerror(int err);

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

enum {
	_CPU_BRAND_STRING = 1000,
};
enum {
	_KERN_MSGBUF = 1000,
};

enum {
	CTLTYPE_NODE = 1,
	CTLTYPE_INT,
	CTLTYPE_STRING,
	CTLTYPE_QUAD,
	CTLTYPE_OPAQUE,
	CTLTYPE_STRUCT = CTLTYPE_OPAQUE,
};

static struct linux_utsname lu;
static void copyout_string(const char* str, char* out, unsigned long* out_len);
static void need_uname(void);

long sys_sysctl(int* name, unsigned int nlen, void* old,
		unsigned long* oldlen, void* _new, unsigned long newlen)
{
	int ret;

	if (nlen < 2)
		return -EINVAL;

	// __simple_printf("sysctl %d,%d\n", name[0], name[1]);
	if (name[0] == CTL_UNSPEC)
	{
		switch (name[1])
		{
			case 1: // oid to name
				return sysctl_oid_to_name(name + 2, nlen - 2, (char*) old, oldlen);
			case 3: // name to oid
				return sysctl_name_to_oid((const char*) _new, (int*) old, oldlen);
			case 4: // get data type
				return sysctl_get_type(name + 2, nlen - 2, (int*) old, oldlen);
		}
	}
	else if (name[0] == CTL_HW)
	{
		int* ovalue = (int*) old;
		struct host_basic_info hinfo;
		mach_msg_type_number_t hcount = HOST_BASIC_INFO_COUNT;
		mach_port_t hself;
		unsigned long orig_old_len = *oldlen;

		hself = mach_host_self();
		host_info(hself, HOST_BASIC_INFO, &hinfo, &hcount);
		mach_port_deallocate(mach_task_self(), hself);

		if (nlen < 2)
			return -ENOTDIR;

		// String values
		switch (name[1])
		{
			case HW_MACHINE:
			{
				need_uname();
				copyout_string(lu.machine, (char*) old, oldlen);
				return 0;
			}
		}

		// Integer values
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
	else if (name[0] == CTL_KERN)
	{
		switch (name[1])
		{
			case _KERN_MSGBUF:
			{
				if (*oldlen <= 8)
				{
					// Caller is asking for buffer size
					if (*oldlen < sizeof(int))
						return -EINVAL;

					*((int*) old) = __linux_syslog(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);
					return 0;
				}
				else
				{
					// Caller is asking for buffer contents
					int ret;

					ret = __linux_syslog(SYSLOG_ACTION_READ_ALL, (char*) old, *oldlen);
					if (ret < 0)
						return errno_linux_to_bsd(ret);

					*oldlen = ret;
					return 0;
				}
				break;
			}
			case KERN_PROC:
			{
				if (nlen < 4)
					return -ENOTDIR;
				// Returns array of struct kinfo_proc
				return _sysctl_proc(name[2], name[3], (struct kinfo_proc*) old, oldlen);

			}
			case KERN_PROCARGS2:
			{
				if (nlen < 3)
					return -ENOTDIR;
				return _sysctl_procargs(name[2], (char*) old, oldlen);
			}
			case KERN_ARGMAX:
			{
				struct rlimit lim;
				int r;
				int* ovalue = (int*) old;

				r = sys_getrlimit(BSD_RLIMIT_STACK, &lim);
				if (r < 0)
					return r;

				if (ovalue != NULL)
					*ovalue = lim.rlim_cur / 4;
				*oldlen = sizeof(int);
				return 0;
			}
			case KERN_BOOTTIME:
			{
				// Fill in struct timeval { tv_sec, tv_usec }
				struct sysinfo info;
				struct bsd_timeval* tv;

				if (__linux_sysinfo(&info) == -1)
					return -errno;

				tv = (struct bsd_timeval*) old;
				if (tv != NULL)
				{
					if (*oldlen < sizeof(*tv))
						return -EINVAL;
					sys_gettimeofday(tv, NULL);

					tv->tv_sec -= info.uptime;
				}
				*oldlen = sizeof(*tv);
				
				return 0;
			}
		}

		need_uname();

		// String values
		switch (name[1])
		{
			case KERN_OSTYPE:
			{
				const char* s = EMULATED_SYSNAME;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_HOSTNAME:
				copyout_string(lu.nodename, (char*) old, oldlen);
				return 0;
			case KERN_OSRELEASE:
			{
				const char* s = EMULATED_RELEASE;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_VERSION:
			{
				const char* s = EMULATED_VERSION;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_DOMAINNAME:
				copyout_string(lu.domainname, (char*) old, oldlen);
				return 0;
		}
	}
	else if (name[0] == CTL_MACHDEP)
	{
		switch (name[1])
		{
			case _CPU_BRAND_STRING:
			{
				char brand[13];
				get_cpu_brand(brand);
				copyout_string(brand, (char*) old, oldlen);
				return 0;
			}
		}
	}

	return -ENOTDIR;
}

static void need_uname(void)
{
#ifndef VARIANT_DYLD
	// Cache __linux_uname results
	if (!lu.sysname[0])
	{
		__linux_uname(&lu);
	}
#endif
}

extern unsigned long strlcpy(char* dst, const char* src, unsigned long size);
static void copyout_string(const char* str, char* out, unsigned long* out_len)
{
	unsigned long len;
	if (out != NULL)
		len = strlcpy(out, str, *out_len);
	else
		len = strlen(str);

	*out_len = len;
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
	else if (strncmp(name, "kern", cat_len) == 0)
	{
		oid_name[0] = CTL_KERN;
		*oid_len = 2 * sizeof(int);

		if (strcmp(dot+1, "msgbuf") == 0)
			oid_name[1] = _KERN_MSGBUF;
		else if (strcmp(dot+1, "boottime") == 0)
			oid_name[1] = KERN_BOOTTIME;
		else
			return -ENOTDIR;

		return 0;
	}
	else if (strncmp(name, "machdep", cat_len) == 0)
	{
		oid_name[0] = CTL_MACHDEP;
		*oid_len = 2 * sizeof(int);

		if (strcmp(dot+1, "cpu.brand_string") == 0)
			oid_name[1] = _CPU_BRAND_STRING;
		else
			return -ENOTDIR;
		return 0;
	}

	__simple_printf("Unknown sysctl: %s\n", name);
	return -ENOTDIR;
}

long sys_sysctlbyname(const char* name, unsigned long namelen, void* old, unsigned long* oldlen, void* _new, unsigned long newlen)
{
	long rv;
	int oid[4];
	unsigned long oid_len = 4;

	rv = sysctl_name_to_oid(name, oid, &oid_len);
	if (rv < 0)
		return rv;

	return sys_sysctl(oid, oid_len, old, oldlen, _new, newlen);
}

#define settype(first, str) \
	if (*outlen < sizeof(int) + sizeof(str)) \
			return -EINVAL; \
	out[0] = first; \
	strcpy((char*) &out[1], str); \
	*outlen = sizeof(int) + sizeof(str);

static long sysctl_get_type(const int* oid, int nlen, int* out, unsigned long* outlen)
{
	switch (oid[0])
	{
		case CTL_HW:
			if (nlen < 2)
			{
				settype(CTLTYPE_NODE, "");
				return 0;
			}

			switch (oid[1])
			{
				case HW_AVAILCPU:
				case HW_NCPU:
				case _HW_PHYSICAL_CPU:
				case _HW_PHYSICAL_CPU_MAX:
				case _HW_LOGICAL_CPU:
				case _HW_LOGICAL_CPU_MAX:
				case HW_PAGESIZE:
				case _HW_CPUTYPE:
				case _HW_CPUSUBTYPE:
				case _HW_CPUTHREADTYPE:
					settype(CTLTYPE_INT, "I");
					return 0;
				case HW_MEMSIZE:
				case HW_PHYSMEM:
				case HW_USERMEM:
					settype(CTLTYPE_QUAD, "QU");
					return 0;
			}
			break;
		case CTL_KERN:
			if (nlen < 2)
			{
				settype(CTLTYPE_NODE, "");
				return 0;
			}

			switch (oid[1])
			{
				case _KERN_MSGBUF:
				case KERN_PROCARGS2:
				case KERN_OSTYPE:
				case KERN_HOSTNAME:
				case KERN_OSRELEASE:
				case KERN_VERSION:
				case KERN_DOMAINNAME:
					settype(CTLTYPE_STRING, "");
					return 0;
				case KERN_PROC:
					settype(CTLTYPE_STRUCT, "");
					return 0;
				case KERN_BOOTTIME:
				{
					settype(CTLTYPE_STRUCT, "S,timeval");
					return 0;
				}
				case KERN_ARGMAX:
					settype(CTLTYPE_INT, "L");
					return 0;
			}
			break;
		case CTL_MACHDEP:
			if (nlen < 2)
			{
				settype(CTLTYPE_NODE, "");
				return 0;
			}

			switch (oid[1])
			{
				case _CPU_BRAND_STRING:
					settype(CTLTYPE_STRING, "");
					return 0;
			}
	}
	return -ENOENT;
}

#define outstring(str) \
	strncpy(name, str, *outlen - 1); \
	name[*outlen - 1] = '\0'; \
	if (*outlen > sizeof(str)-1) \
		*outlen = sizeof(str)-1;

static long sysctl_oid_to_name(const int* oid, int nlen, char* name, unsigned long* outlen)
{
	if (nlen == 1)
	{
		switch (oid[0])
		{
			case CTL_HW:
				outstring("hw");
				return 0;
			case CTL_KERN:
				outstring("kern");
				return 0;
		}
	}
	else if (nlen == 2)
	{
		switch (oid[0])
		{
			case CTL_HW:
				switch (oid[1])
				{
					case HW_AVAILCPU:
						outstring("hw.activecpu");
						return 0;
					case HW_NCPU:
						outstring("hw.ncpu");
						return 0;
					case _HW_PHYSICAL_CPU:
						outstring("hw.physicalcpu");
						return 0;
					case _HW_PHYSICAL_CPU_MAX:
						outstring("hw.physicalcpu_max");
						return 0;
					case _HW_LOGICAL_CPU:
						outstring("hw.logicalcpu");
						return 0;
					case _HW_LOGICAL_CPU_MAX:
						outstring("hw.logicalcpu_max");
						return 0;
					case HW_MEMSIZE:
						outstring("hw.memsize");
						return 0;
					case HW_PAGESIZE:
						outstring("hw.pagesize");
						return 0;
					case _HW_CPUTYPE:
						outstring("hw.cputype");
						return 0;
					case _HW_CPUSUBTYPE:
						outstring("hw.cpusubtype");
						return 0;
					case _HW_CPUTHREADTYPE:
						outstring("hw.cputhreadtype");
						return 0;
				}
				break;
			case CTL_KERN:
				switch (oid[1])
				{
					case _KERN_MSGBUF:
						outstring("kern.msgbuf");
						return 0;
					case KERN_BOOTTIME:
						outstring("kern.boottime");
						return 0;
					case KERN_OSTYPE:
						outstring("kern.ostype");
						return 0;
					case KERN_VERSION:
						outstring("kern.version");
						return 0;
					case KERN_OSRELEASE:
						outstring("kern.osrelease");
						return 0;
					case KERN_HOSTNAME:
						outstring("kern.hostname");
						return 0;
					case KERN_DOMAINNAME:
						outstring("kern.domainname");
						return 0;
				}
				break;
			case CTL_MACHDEP:
				switch (oid[1])
				{
					case _CPU_BRAND_STRING:
						outstring("machdep.cpu.brand_string");
						return 0;
				}
		}
	}

	return -ENOENT;
}

#ifndef __cpuid
#define __cpuid(level, a, b, c, d)			\
  __asm__ ("cpuid\n\t"					\
	   : "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
	   : "0" (level))
#endif

void get_cpu_brand(char* brand)
{
	unsigned int level = 0;
    unsigned int eax = 0;
	union
	{
		struct
		{
	    	unsigned int ebx;
		    unsigned int edx;
	    	unsigned int ecx;
		};
		char name[12];
	} v;

    __cpuid(level, eax, v.ebx, v.ecx, v.edx);

	strncpy(brand, v.name, 12);
	brand[12] = 0;
}

