#include "config.h"
#include "host.h"
#include "trace.h"
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>

__attribute__((constructor)) static void checkProcfs()
{
	if (::access("/proc/", X_OK) == -1)
	{
		std::cerr << "/proc is required to be mounted.\n";
		abort();
	}
}

kern_return_t host_page_size(host_t host, size_t* size)
{
	TRACE2(host, size);

	// TODO: add a check for host

	*size = getpagesize();
	return KERN_SUCCESS;
}

static void getPhysicalLogicalCPUCounts(int& physical, int& logical)
{
	std::ifstream file("/proc/cpuinfo");
	std::string line;
	std::set<int> phys_ids;

	physical = logical = 0;

	while(std::getline(file, line))
	{
		if (line.compare(0, 10, "processor\t") == 0)
		{
			logical++;
		}
		else if (line.compare(0, 12, "physical id\t") == 0)
		{
			phys_ids.insert(atoi(line.c_str() + 14));
		}
	}

	physical = phys_ids.size();
}

// TODO: test this function
kern_return_t host_info(host_t host, int flavor, host_basic_info* info, long* info_cnt)
{
	if (flavor != HOST_BASIC_INFO)
	{
		LOG << "Unsupported host_info flavor: " << flavor << std::endl;
		return KERN_FAILURE;
	}
	
	memset(info, 0, sizeof(*info));

#if defined(__i386__)
	info->cpu_type = CPU_TYPE_I386;
	info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#elif defined(__x86_64__)
	info->cpu_type = CPU_TYPE_I386; // TODO: check the behavior on Darwin!
	info->cpu_subtype = CPU_SUBTYPE_I386_ALL;
#elif defined(__arm__)
	info->cpu_type = CPU_TYPE_ARM;
	info->cpu_subtype = CPU_SUBTYPE_ARM_ALL;
#else
#	error Unsupported platform
#endif

	info->max_cpus = sysconf(_SC_NPROCESSORS_CONF);
	info->avail_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	
	int64_t pages = ::sysconf(_SC_PHYS_PAGES);
	int64_t page_size = ::sysconf(_SC_PAGE_SIZE);
	int64_t max_mem = pages * page_size;
	
	if (*info_cnt >= sizeof(*info))
	{
			info->max_mem = max_mem;
			info->physical_cpu_max = info->logical_cpu_max = info->max_cpus;

			getPhysicalLogicalCPUCounts(info->physical_cpu, info->logical_cpu);
	}
	
	info->memory_size = (max_mem <= 0x80000000) ? int32_t(max_mem) : 0x80000000; // 2 GB cap

	*info_cnt = std::min<int32_t>(sizeof(*info), *info_cnt);

	return KERN_SUCCESS;
}

static std::vector<int> getTicksLine()
{
	std::ifstream file("/proc/stat");
	std::string dummy;
	std::vector<int> rv;

	if (!file.is_open())
		return rv;

	file >> dummy;
	if (dummy != "cpu")
		return rv;

	for (int i = 0; i < 7; i++)
	{
		int n;
		file >> n;
		rv.push_back(n);
	}

	return rv;
}

kern_return_t host_statistics(host_t host, int flavor, void* out, long* out_cnt)
{
	memset(out, 0, *out_cnt);

	if (flavor == HOST_LOAD_INFO)
	{
		host_load_info* info = static_cast<host_load_info*>(out);
		if (*out_cnt < sizeof(host_load_info))
			return KERN_INVALID_ARGUMENT;

		double nat_avg[3];
		if (getloadavg(nat_avg, 3) == -1)
			return KERN_FAILURE;

		// We're diverging from what Mach returns here, but this is as close as we can get
		info->avenrun[0] = int(nat_avg[0]);
		info->avenrun[1] = int(nat_avg[1]);
		info->avenrun[2] = int(nat_avg[2]);

		*out_cnt = sizeof(*info);

		return KERN_SUCCESS;
	}
	else if (flavor == HOST_VM_INFO)
	{
		return KERN_FAILURE; // not implementable?
	}
	else if (flavor == HOST_CPU_LOAD_INFO)
	{
		host_cpu_load_info* load = static_cast<host_cpu_load_info*>(out);
		if (*out_cnt < sizeof(host_cpu_load_info))
			return KERN_INVALID_ARGUMENT;

		std::vector<int> ticks = getTicksLine();
		if (ticks.size() < 4)
			return KERN_FAILURE;

		load->cpu_ticks[CPU_STATE_USER] = ticks[0];
		load->cpu_ticks[CPU_STATE_NICE] = ticks[1];
		load->cpu_ticks[CPU_STATE_SYSTEM] = ticks[2];
		load->cpu_ticks[CPU_STATE_IDLE] = ticks[3];

		*out_cnt = sizeof(*load);

		return KERN_SUCCESS;
	}
	else if (flavor == HOST_VM_INFO64)
	{
		return KERN_FAILURE; // not implementable?
	}

	return KERN_INVALID_ARGUMENT;
}

host_t mach_host_self()
{
	static struct host h;
	return &h;
}

