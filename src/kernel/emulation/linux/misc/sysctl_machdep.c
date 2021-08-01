#include "sysctl_machdep.h"
#include "simple.h"
#include <sys/errno.h>
#include <alloca.h>
#include "../fcntl/open.h"
#include "../readline.h"
#include "../unistd/close.h"

extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);
extern int strncmp(const char* str1, const char* str2, __SIZE_TYPE__ n);;

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
	_CPU_CORE_COUNT,
};

static sysctl_handler(handle_vendor);
static sysctl_handler(handle_max_basic);
static sysctl_handler(handle_family);
static sysctl_handler(handle_model);
static sysctl_handler(handle_stepping);
static sysctl_handler(handle_brand_string);
static sysctl_handler(handle_features);
static sysctl_handler(handle_core_count);

const struct known_sysctl sysctls_machdep_cpu[] = {
    { .oid = _CPU_MAX_BASIC, .type = CTLTYPE_INT, .exttype = "I", .name = "max_basic", .handler = handle_max_basic },
		{ .oid = _CPU_VENDOR, .type = CTLTYPE_STRING, .exttype = "S", .name = "vendor", .handler = handle_vendor },
    { .oid = _CPU_FAMILY, .type = CTLTYPE_INT, .exttype = "I", .name = "family", .handler = handle_family },
		{ .oid = _CPU_MODEL, .type = CTLTYPE_INT, .exttype = "I", .name = "model", .handler = handle_model },
		{ .oid = _CPU_STEPPING, .type = CTLTYPE_INT, .exttype = "I", .name = "stepping", .handler = handle_stepping },
		{ .oid = _CPU_BRAND_STRING, .type = CTLTYPE_STRING, .exttype = "S", .name = "vendor", .handler = handle_brand_string },
		{ .oid = _CPU_FEATURES, .type = CTLTYPE_STRING, .exttype = "S", .name = "features", .handler = handle_features },
		{ .oid = _CPU_CORE_COUNT, .type = CTLTYPE_INT, .exttype = "I", .name = "core_count", .handler = handle_core_count },
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
            unsigned int level = 0;     \
            unsigned int eax = value;   \
            unsigned int ebx;           \
            unsigned int edx;           \
            unsigned int ecx
#endif


static inline void copyout_int(int value, char* to_copy, size_t* to_copy_length)
{
        char tmp[64];
        __simple_sprintf(tmp, "%d", value);
        copyout_string(tmp, to_copy, to_copy_length);
}


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

	copyout_int(eax, (char*)old,oldlen);

	return 0;
}

sysctl_handler(handle_family)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax = eax >> 7;
    eax &= 15;

    copyout_int(eax, old, oldlen);

    return 0;
}

sysctl_handler(handle_model)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax = eax >> 3;
    eax &= 15;

    copyout_int(eax,(char*)old, oldlen);

    return 0;
}

sysctl_handler(handle_stepping)
{
    setup(1);

    __cpuid(level, eax, ebx, ecx, edx);

    eax &= 15;

    copyout_int(eax,(char*)old, oldlen);

    return 0;
}

sysctl_handler(handle_brand_string)
{
    setup(0x80000000);

    __cpuid(level,eax,ebx, ecx, edx);

    if (eax < 0x80000004) // the information is not implemented
        return 2;



    union
    {
        unsigned int brand[12];
        char name[49];
    } v;

		for (int i = 1; i < 4; i++)
		{
				eax = 0x80000000+i;
				__cpuid(level, eax, ebx, ecx, edx);
				v.brand[0x0+(i-1)*4] = eax;
				v.brand[0x1+(i-1)*4] = ebx;
				v.brand[0x2+(i-1)*4] = ecx;
				v.brand[0x3+(i-1)*4] = edx;
		}

    v.name[48] = 0;
    copyout_string(v.name, (char*) old, oldlen);

    return 0;
}

sysctl_handler(handle_features)
{

    setup(1);

    static const char features[][7] = {"FPU","VME", "DE", "PSE", "TSC", "MSR", "PAE", "MCE", "CX8", "APIC", "","SEP","MTRR","PGE",
                        "MCA", "CMOV", "PAT", "PSE-36", "PSN", "CLFSH", "", "DS", "ACPI", "MMX", "FXSR", "SSE", "SSE2", "SS",
                        "HTT", "TM","IA64","PBE"};

    __cpuid(0,eax,ebx,ecx,edx);
    int current_length = 0;

    if (old != NULL)
    {

        char *outsr = (char*)old;

        for (int i = 0; i < 32; i++)
        {
            if (i == 10 || i == 20)
                continue;

            if ( (edx & (1<<i)) && current_length < *oldlen)
            {
                if (current_length)
                {
                    outsr[current_length] = ' ';

                    current_length++;
                }
                if (current_length < *oldlen)
                {
                    int len = __simple_strlen(features[i]);
                    
                    strncpy(outsr + current_length, features[i],
                    *oldlen - current_length);

                    current_length += len;
                }
            }
        }

        if (current_length < *oldlen)
        {
            outsr[current_length] = '\0';
        }

    }
    else
    {

        for (int i = 0; i < 32; i++)
        {

            if (i == 10 || i == 20)
                continue;

            if (edx & (1<<i)) 
            {
                if (current_length != 0)
                    current_length++;
                
                current_length += __simple_strlen(features[i]);
            }

        }

        if (oldlen)
            *oldlen = current_length;

    }
        


    return 0;

}


// TODO: i doubt core count is ever going to change.
//       this is something that could be cached if we had some way of doing something only once in libsystem_kernel (i.e. a simple version of dispatch_once).
sysctl_handler(handle_core_count) {
	// it's easier to use /proc/cpuinfo for this.
	// using cpuid directly is more difficult because AMD processors don't use the same method as Intel processors.

	int infofd = sys_open_nocancel("/proc/cpuinfo", BSD_O_RDONLY, 0);
	struct rdline_buffer rbuf;
	const char* line;

	// a reasonable default
	size_t core_count = 1;

	if (infofd < 0)
		goto out;

	_readline_init(&rbuf);

	while ((line = _readline(infofd, &rbuf)) != NULL) {
		if (strncmp(line, "cpu cores", 9) != 0) {
			continue;
		}

		// great, we've got "cpu cores"
		// now find the actual number
		while (*line != ':' && *line != '\0')
			++line;

		// shouldn't happen, but just in case
		if (*line == '\0')
			continue;

		++line; // skip the colon
		if (*line == '\0')
			continue;

		++line; // skip the space
		if (*line == '\0')
			continue;

		core_count = __simple_atoi(line, NULL);
		break;
	}

out:
	if (infofd >= 0)
		close_internal(infofd);
	if (old != NULL && oldlen && *oldlen >= sizeof(uint32_t))
		*(uint32_t*)old = core_count;
	if (oldlen)
		*oldlen = sizeof(uint32_t);
	return 0;
};
