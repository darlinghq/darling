#include "sysctl_hw.h"
#include <mach/host_info.h>
#include <mach/machine.h>
#include <mach/mach_init.h>
#include <sys/errno.h>
#include "sysctl_kern.h"
#include "sysctl_proc.h"
#include "../ext/sys/utsname.h"

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
	_HW_CPUTHREADTYPE,
	_HW_64BITCAPABLE,
	_HW_CPUFREQUENCY = 15,
};

static sysctl_handler(handle_availcpu);
static sysctl_handler(handle_physicalcpu);
static sysctl_handler(handle_physicalcpu_max);
static sysctl_handler(handle_logicalcpu);
static sysctl_handler(handle_logicalcpu_max);
static sysctl_handler(handle_memsize);
static sysctl_handler(handle_pagesize);
static sysctl_handler(handle_cputype);
static sysctl_handler(handle_cpusubtype);
static sysctl_handler(handle_cputhreadtype);
static sysctl_handler(handle_cpu64bitcapable);
static sysctl_handler(handle_machine);
static sysctl_handler(handle_cpufrequency);

const struct known_sysctl sysctls_hw[] = {
	{ .oid = HW_AVAILCPU, .type = CTLTYPE_INT, .exttype = "", .name = "availcpu", .handler = handle_availcpu },
	{ .oid = HW_NCPU, .type = CTLTYPE_INT, .exttype = "", .name = "ncpu", .handler = handle_availcpu },
	{ .oid = _HW_PHYSICAL_CPU, .type = CTLTYPE_INT, .exttype = "", .name = "physicalcpu", .handler = handle_physicalcpu },
	{ .oid = _HW_PHYSICAL_CPU_MAX, .type = CTLTYPE_INT, .exttype = "", .name = "physicalcpu_max", .handler = handle_physicalcpu_max },
	{ .oid = _HW_LOGICAL_CPU, .type = CTLTYPE_INT, .exttype = "", .name = "logicalcpu", .handler = handle_logicalcpu },
	{ .oid = _HW_LOGICAL_CPU_MAX, .type = CTLTYPE_INT, .exttype = "", .name = "logicalcpu_max", .handler = handle_logicalcpu_max },
	{ .oid = HW_MEMSIZE, .type = CTLTYPE_QUAD, .exttype = "U", .name = "memsize", .handler = handle_memsize },
	{ .oid = HW_PAGESIZE, .type = CTLTYPE_INT, .exttype = "", .name = "pagesize", .handler = handle_pagesize },
	{ .oid = _HW_CPUTYPE, .type = CTLTYPE_INT, .exttype = "", .name = "cputype", .handler = handle_cputype },
	{ .oid = _HW_CPUSUBTYPE, .type = CTLTYPE_INT, .exttype = "", .name = "cpusubtype", .handler = handle_cpusubtype },
	{ .oid = _HW_CPUTHREADTYPE, .type = CTLTYPE_INT, .exttype = "", .name = "cputhreadtype", .handler = handle_cputhreadtype },
	{ .oid = _HW_64BITCAPABLE, .type = CTLTYPE_INT, .exttype = "", .name = "cpu64bit_capable", .handler = handle_cpu64bitcapable },
	{ .oid = HW_MACHINE, .type = CTLTYPE_STRING, .exttype = "S", .name = "machine", .handler = handle_machine },
	{ .oid = _HW_CPUFREQUENCY, .type = CTLTYPE_INT, .exttype = "", .name = "cpufrequency", .handler = handle_cpufrequency },
	{ .oid = -1 }
};

static const struct host_basic_info* gethostinfo(void)
{
	static struct host_basic_info hinfo;

	if (!hinfo.avail_cpus)
	{
		mach_msg_type_number_t hcount = HOST_BASIC_INFO_COUNT;
		mach_port_t hself;

		hself = mach_host_self();
		host_info(hself, HOST_BASIC_INFO, &hinfo, &hcount);
		mach_port_deallocate(mach_task_self(), hself);
	}

	return &hinfo;
}

sysctl_handler(handle_availcpu)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->avail_cpus;
	return 0;
}

sysctl_handler(handle_physicalcpu)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->physical_cpu;
	return 0;
}

sysctl_handler(handle_physicalcpu_max)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->physical_cpu_max;
	return 0;
}

sysctl_handler(handle_logicalcpu)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->logical_cpu;
	return 0;
}

sysctl_handler(handle_logicalcpu_max)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->logical_cpu_max;
	return 0;
}

sysctl_handler(handle_memsize)
{

	sysctl_handle_size(sizeof(unsigned long long));
	*((unsigned long long*) old) = gethostinfo()->max_mem;
	return 0;
}

sysctl_handler(handle_pagesize)
{
	//sysctl_handle_size(sizeof(int));
	// TODO: maybe should be int64_t (long long) all the time, keeping compatability with int for now
	if (old == NULL)
	{
		*oldlen = sizeof(int);
		return 0;
	}
	else if (*oldlen == sizeof(long long))
	{
		*((long long*) old) = 4096; // true on all Darling platforms
	}
	else
	{
		*((int*) old) = 4096; // true on all Darling platforms
		*oldlen = sizeof(int);
	}
	return 0;
}

sysctl_handler(handle_cputype)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->cpu_type;
	return 0;
}

sysctl_handler(handle_cpusubtype)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->cpu_subtype;
	return 0;
}

sysctl_handler(handle_cputhreadtype)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = gethostinfo()->cpu_threadtype;
	return 0;
}

sysctl_handler(handle_cpufrequency)
{
	int freq = 2400000;
	char buf[16];

	if (read_string("/sys/bus/cpu/devices/cpu0/cpufreq/cpuinfo_max_freq", buf, sizeof(buf)))
		freq = __simple_atoi(buf, NULL);
	*((int*) old) = freq;
	
	return 0;
}

sysctl_handler(handle_cpu64bitcapable)
{
	sysctl_handle_size(sizeof(int));
	*((int*) old) = 1; // Darling only supports 64-bit CPUs
	return 0;
}

sysctl_handler(handle_machine)
{
	copyout_string(need_uname()->machine, (char*) old, oldlen);
	return 0;
}
