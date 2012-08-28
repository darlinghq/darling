#ifndef MACH_HOST_H
#define MACH_HOST_H
#include <stdint.h>
#include <mach/kern_return.h>
#include <mach/i386/vm_types.h>

extern "C"
{
  
typedef long host_t;
typedef long host_priv_t;

#pragma pack(4)
struct __darwin_host_basic_info {
	int max_cpus;
	int avail_cpus;
	int memory_size;
	unsigned int cpu_type;
	unsigned int cpu_subtype;
	unsigned int cpu_threadtype;
	int physical_cpu;
	int physical_cpu_max;
	int logical_cpu;
	int logical_cpu_max;
	uint64_t max_mem;
};
#pragma pack()

#define HOST_BASIC_INFO         1
#define HOST_SCHED_INFO         3
#define HOST_RESOURCE_SIZES     4
#define HOST_PRIORITY_INFO      5
#define HOST_SEMAPHORE_TRAPS    7
#define HOST_MACH_MSG_TRAP      8

kern_return_t host_page_size(host_t host, vm_size_t* size);
kern_return_t host_info(host_t host, int flavor, __darwin_host_basic_info* info, int* info_cnt);

}

#endif
