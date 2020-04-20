#include "sysctl_machdep.h"
#include <sys/errno.h>

enum {
	_MACHDEP_CPU = 1000,
};

enum {
	_CPU_VENDOR = 1000,
	_CPU_MAX_BASIC,
	_CPU_FAMILY,
	_CPU_MODEL,
	_CPU_STEPPING,
	_CPU_BRAND_STRING,
	_CPU_FEATURES,
};

static sysctl_handler(handle_vendor);
static sysctl_handler(handle_max_basic);
static sysctl_handler(handle_family);
static sysctl_handler(handle_model);
static sysctl_handler(handle_stepping);
static sysctl_handler(handle_brand_string);
static sysctl_handler(handle_features);

const struct known_sysctl sysctls_machdep_cpu[] = {
    { .oid = _CPU_MAX_BASIC, .type = CTLTYPE_INT, .exttype = "I", .name = "max_basic", .handler = handle_max_basic },
		{ .oid = _CPU_VENDOR, .type = CTLTYPE_STRING, .exttype = "S", .name = "vendor", .handler = handle_vendor },
    { .oid = _CPU_FAMILY, .type = CTLTYPE_INT, .exttype = "I", .name = "family", .handler = handle_family },
		{ .oid = _CPU_MODEL, .type = CTLTYPE_INT, .exttype = "I", .name = "model", .handler = handle_model },
		{ .oid = _CPU_STEPPING, .type = CTLTYPE_INT, .exttype = "I", .name = "stepping", .handler = handle_stepping },
		{ .oid = _CPU_BRAND_STRING, .type = CTLTYPE_STRING, .exttype = "S", .name = "vendor", .handler = handle_brand_string },
		{ .oid = _CPU_FEATURES, .type = CTLTYPE_STRING, .exttype = "S", .name = "features", .handler = handle_features },
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

#ifndef setup
#define setup(value)\
            unsigned int level = 0;\
            unsigned int eax = value;\
            unsigned int ebx;\
            unsigned int edx;\
            unsigned int ecx

sysctl_handler(handle_vendor)
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
	setup(0);


    __cpuid(level, eax, ebx, ecx, edx);


	sprintf(old,"%d",eax);

	return 0;
}

sysctl_handler(handle_family)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax = eax >> 7;
    eax &= 15;

    sprintf(old,"%d",eax);

    return 0;
}

sysctl_handler(handle_model)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax = eax >> 3;
    eax &= 15;

    sprintf(old,"%d",eax);

    return 0;
}

sysctl_handler(handle_stepping)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax &= 15;

    sprintf(old,"%d",eax);

    return 0;
}

sysctl_handler(handle_brand_string)
{
    unsigned int level = 0;
    unsigned int eax = 0x80000000;

    unsigned int test[3];

    __cpuid(level,eax,test, test+1,test+2);

    if(eax < 0x80000004) // the information is not implemented
        return 2;

    eax = 0x80000001;

    union
    {
        unsigned int brand[12];
        char name[49];
    } v;

    __cpuid(level, eax, v.brand+0x1, v.brand+0x2, v.brand+0x3);
    v.brand[0x0] = eax;
    eax = 0x80000002;

    __cpuid(level, eax, v.brand+0x5, v.brand+0x6, v.brand+0x7);
    v.brand[0x4] = eax;
    eax = 0x80000003;

    __cpuid(level, eax, v.brand+0x9, v.brand+0xa, v.brand+0xb);
    v.brand[0x8] = eax;

    v.name[48] = 0;
    copyout_string(v.name, (char*) old, oldlen);

    return 0;
}
sysctl_handler(handle_features)
{
    setup(1);

    char *features[] = {"FPU","VME", "DE", "PSE", "TSC", "MSR", "PAE", "MCE", "CX8", "APIC", NULL,"SEP","PGE","MCA",
                        "CMOV", "PAT", "PSE-36", "PSN", "CLFSH", "DS", "ACPI", "MMX", "FXSR", "SSE", "SSE2", "SS",
                        "HTT", "TM", NULL, "PBE"};

    char *out = (char*)(calloc(119, sizeof(char))); //"worst" case: we have all of it plus null termination

    __cpuid(0,eax,ebx,ecx,edx);

    int j = 0;
    int size = sizeof(char);
    int counter = 0;

    for (int i = 0; i < 32; ++i)
    {
        if(edx>>i&1)
        {
            for(j=0;j< sizeof(features[i])/size; j++)
            {
                out[counter+j] = features[i][j]
            }

            counter = counter + j + 1;

            out[counter] = ' ';

            counter += 1;

        }
    }

    copyout_string(out,(*char) old, oldlen);

    return 0;

}
