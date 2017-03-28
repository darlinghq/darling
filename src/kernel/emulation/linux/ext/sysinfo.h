#ifndef _SYSINFO_H
#define _SYSINFO_H

struct sysinfo
{
	long uptime;
	unsigned long loads[3];
	unsigned long totalram;
	unsigned long freeram;
	unsigned long sharedram;
	unsigned long bufferram;
	unsigned long totalswap;
	unsigned long freeswap;
	unsigned short procs;
	unsigned long totalhigh;
	unsigned long freehigh;
	unsigned int mem_unit;
	char _f[20-2*sizeof(long)-sizeof(int)];
};

int __linux_sysinfo(struct sysinfo* info);

#endif

