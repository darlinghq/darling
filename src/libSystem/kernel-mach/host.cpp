#include "config.h"
#include "host.h"
#include "trace.h"
#include <unistd.h>
#include <cstring>
#include <algorithm>

kern_return_t host_page_size(host_t host, size_t* size)
{
	TRACE2(host, size);

	// TODO: add a check for host

	*size = getpagesize();
	return KERN_SUCCESS;
}

#define CPU_TYPE_I386 7
#define CPU_TYPE_X86_64 (CPU_TYPE_I386 | 0x1000000)
#define CPU_SUBTYPE_I386_ALL 3

kern_return_t host_info(host_t host, int flavor, __darwin_host_basic_info* info, int* info_cnt)
{
	// TODO: add a check for host
	
	if (flavor != HOST_BASIC_INFO)
	{
		LOG << "Unsupported host_info flavor: " << flavor << std::endl;
		return KERN_FAILURE;
	}
	
	memset(info, 0, *info_cnt);

#if defined(__i386__)
	info->cpu_type = CPU_TYPE_I386;
	info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#elif defined(__x86_64__)
	info->cpu_type = CPU_TYPE_I386; // TODO: check the behavior on Darwin!
	info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#else
#	error Unsupported platform
#endif

	info->max_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	info->avail_cpus = sysconf(_SC_NPROCESSORS_CONF);
	
	int64_t pages = ::sysconf(_SC_PHYS_PAGES);
	int64_t page_size = ::sysconf(_SC_PAGE_SIZE);
	int64_t max_mem = pages * page_size;
	
	if (*info_cnt >= sizeof(*info))
	{
			info->max_mem = max_mem;
			// TODO: we need to parse /proc/cpuinfo to get detailed CPU info
	}
	
	info->memory_size = (max_mem <= 0x80000000) ? int32_t(max_mem) : 0x80000000;

	*info_cnt = std::min<int32_t>(sizeof(*info), *info_cnt);

	return KERN_SUCCESS;
}

