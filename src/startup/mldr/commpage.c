#include "commpage.h"
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <tgmath.h>
#include <string.h>
#include <stdlib.h>
#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#endif
#include <unistd.h>
#include <sys/sysinfo.h>

// Include commpage definitions
#define PRIVATE
#include <i386/cpu_capabilities.h>

static const char* SIGNATURE32 = "commpage 32-bit";
static const char* SIGNATURE64 = "commpage 64-bit";

static uint64_t get_cpu_caps(void);

#define CGET(p) (commpage + ((p)-_COMM_PAGE_START_ADDRESS))

void commpage_setup(bool _64bit)
{
	uint8_t* commpage;
	uint64_t* cpu_caps64;
	uint32_t* cpu_caps;
	uint16_t* version;
	char* signature;
	uint64_t my_caps;
	uint8_t *ncpus, *nactivecpus;
	uint8_t *physcpus, *logcpus;
	uint8_t *user_page_shift, *kernel_page_shift;
	struct sysinfo si;

	commpage = (uint8_t*) mmap((void*)(_64bit ? _COMM_PAGE64_BASE_ADDRESS : _COMM_PAGE32_BASE_ADDRESS),
			_64bit ? _COMM_PAGE64_AREA_LENGTH : _COMM_PAGE32_AREA_LENGTH, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (commpage == MAP_FAILED)
	{
		fprintf(stderr, "Cannot mmap commpage: %s\n", strerror(errno));
		exit(1);
	}

	signature = (char*)CGET(_COMM_PAGE_SIGNATURE);
	version = (uint16_t*)CGET(_COMM_PAGE_VERSION);
	cpu_caps64 = (uint64_t*)CGET(_COMM_PAGE_CPU_CAPABILITIES64);
   	cpu_caps = (uint32_t*)CGET(_COMM_PAGE_CPU_CAPABILITIES);

	strcpy(signature, _64bit ? SIGNATURE64 : SIGNATURE32);
	*version = _COMM_PAGE_THIS_VERSION;

	ncpus = (uint8_t*)CGET(_COMM_PAGE_NCPUS);
	*ncpus = sysconf(_SC_NPROCESSORS_CONF);

	nactivecpus = (uint8_t*)CGET(_COMM_PAGE_ACTIVE_CPUS);
	*nactivecpus = sysconf(_SC_NPROCESSORS_ONLN);

	// Better imprecise information than no information
	physcpus = (uint8_t*)CGET(_COMM_PAGE_PHYSICAL_CPUS);
	logcpus = (uint8_t*)CGET(_COMM_PAGE_LOGICAL_CPUS);
	*physcpus = *logcpus = *ncpus;

	// I'm not sure if Linux has separate page sizes for kernel and user space.
	// Apple's code uses left shift logical (1 << user_page_shift) to get the page size value.
	// Since it's very unlikely that the page size won't be a power of 2, we can use __builtin_ctzl()
	// as a substitute for log2().
	user_page_shift = (uint8_t*)CGET(_64bit ? _COMM_PAGE_USER_PAGE_SHIFT_64 : _COMM_PAGE_USER_PAGE_SHIFT_32);
	kernel_page_shift = (uint8_t*)CGET(_COMM_PAGE_KERNEL_PAGE_SHIFT);
	*kernel_page_shift = *user_page_shift = (uint8_t)__builtin_ctzl(sysconf(_SC_PAGESIZE));

	my_caps = get_cpu_caps();
	if (*ncpus == 1)
		my_caps |= kUP;

	*cpu_caps = (uint32_t) my_caps;
	*cpu_caps64 = my_caps;

	if (sysinfo(&si) == 0)
	{
		uint64_t* memsize = (uint64_t*)CGET(_COMM_PAGE_MEMORY_SIZE);
		*memsize = si.totalram * si.mem_unit;
	}
}

uint64_t get_cpu_caps(void)
{
	uint64_t caps = 0;

	{
		union cpu_flags1 eax;
		union cpu_flags2 ecx;
		union cpu_flags3 edx;
		uint32_t ebx;

		eax.reg = ecx.reg = edx.reg = 0;
		__cpuid(1, eax.reg, ebx, ecx.reg, edx.reg);

		if (ecx.mmx)
			caps |= kHasMMX;
		if (ecx.sse)
			caps |= kHasSSE;
		if (ecx.sse2)
			caps |= kHasSSE2;
		if (edx.sse3)
			caps |= kHasSSE3;
		if (ecx.ia64)
			caps |= k64Bit;
		if (edx.ssse3)
			caps |= kHasSupplementalSSE3;
		if (edx.sse41)
			caps |= kHasSSE4_1;
		if (edx.sse42)
			caps |= kHasSSE4_2;
		if (edx.aes)
			caps |= kHasAES;
		if (edx.avx)
			caps |= kHasAVX1_0;
		if (edx.rdrnd)
			caps |= kHasRDRAND;
		if (edx.fma)
			caps |= kHasFMA;
		if (edx.f16c)
			caps |= kHasF16C;
	}
	{
		union cpu_flags4 ebx;
		union cpu_flags5 ecx;
		uint32_t edx = 0, eax = 7;

		__cpuid(7, eax, ebx.reg, ecx.reg, edx);

		if (ebx.erms)
			caps |= kHasENFSTRG;
		if (ebx.avx2)
			caps |= kHasAVX2_0;
		if (ebx.bmi1)
			caps |= kHasBMI1;
		if (ebx.bmi2)
			caps |= kHasBMI2;
		if (ebx.rtm)
			caps |= kHasRTM;
		if (ebx.hle)
			caps |= kHasHLE;
		if (ebx.rdseed)
			caps |= kHasRDSEED;
		if (ebx.adx)
			caps |= kHasADX;
		if (ebx.mpx)
			caps |= kHasMPX;
		if (ebx.sgx)
			caps |= kHasSGX;
	}

	return caps;
}

unsigned long commpage_address(bool _64bit)
{
	return _64bit ? _COMM_PAGE64_BASE_ADDRESS : _COMM_PAGE32_BASE_ADDRESS;
}

