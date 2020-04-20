#include "sysctl_machdep.h"
#include <sys/errno.h>

enum {
	_MACHDEP_CPU = 1000,
};

enum {
	_CPU_BRAND_STRING = 1000,
	_CPU_MAX_BASIC=999,
};

static sysctl_handler(handle_brand_string);
static sysctl_handler(handle_max_basic);

const struct known_sysctl sysctls_machdep_cpu[] = {
	{ .oid = _CPU_BRAND_STRING, .type = CTLTYPE_STRING, .exttype = "S", .name = "brand_string", .handler = handle_brand_string },
	{ .oid = _CPU_MAX_BASIC, .type = CTLTYPE_INT, .exttype = "I", .name = "max_basic", .handler = handle_max_basic },
	{ .oid = -1 }
};

const struct known_sysctl sysctls_machdep[] = {
	{ .oid = _MACHDEP_CPU, .type = CTLTYPE_NODE, .exttype = "", .name = "cpu", .subctls = sysctls_machdep_cpu },
	{ .oid = -1 }
};

#ifndef __cpuid
#define __cpuid(level, a, b, c, d)			\
  __asm__ ("cpuid\n\t"					\
	   : "=a" (a), "=b" (b), "=c" (c), "=d" (d)	\
	   : "0" (level))
#endif

sysctl_handler(handle_brand_string)
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
		char name[13];
	} v;

    __cpuid(level, eax, v.ebx, v.ecx, v.edx);

	v.name[12] = 0;
	copyout_string(v.name, (char*) old, oldlen);

	return 0;
}

sysctl_handler(handle_max_basic)
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
		char name[13];
	} v;

    __cpuid(level, eax, v.ebx, v.ecx, v.edx);

	
	sprintf(old,"%d",eax);

	return 0;
}

