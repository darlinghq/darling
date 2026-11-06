/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cpu_caps_x86.h"
#include <cpuid.h>
#include <stddef.h>

#define PRIVATE
#include <i386/cpu_capabilities.h>

#define BIT(n) (UINT32_C(1) << (n))

// Bit positions from the Intel 64 and IA-32 Architectures Software Developer's
// Manual, Vol. 2A, CPUID instruction (tables "Feature Information Returned in
// the ECX/EDX Register" and "Structured Extended Feature Flags Enumeration Leaf").

// CPUID.01H:EDX
#define LEAF1_EDX_MMX      BIT(23)
#define LEAF1_EDX_SSE      BIT(25)
#define LEAF1_EDX_SSE2     BIT(26)

// CPUID.01H:ECX
#define LEAF1_ECX_SSE3     BIT(0)
#define LEAF1_ECX_SSSE3    BIT(9)
#define LEAF1_ECX_FMA      BIT(12)
#define LEAF1_ECX_SSE4_1   BIT(19)
#define LEAF1_ECX_SSE4_2   BIT(20)
#define LEAF1_ECX_AES      BIT(25)
#define LEAF1_ECX_XSAVE    BIT(26)
#define LEAF1_ECX_OSXSAVE  BIT(27)
#define LEAF1_ECX_AVX      BIT(28)
#define LEAF1_ECX_F16C     BIT(29)
#define LEAF1_ECX_RDRAND   BIT(30)

// CPUID.(EAX=07H,ECX=0):EBX
#define LEAF7_EBX_SGX      BIT(2)
#define LEAF7_EBX_BMI1     BIT(3)
#define LEAF7_EBX_HLE      BIT(4)
#define LEAF7_EBX_AVX2     BIT(5)
#define LEAF7_EBX_BMI2     BIT(8)
#define LEAF7_EBX_ERMS     BIT(9)
#define LEAF7_EBX_RTM      BIT(11)
#define LEAF7_EBX_MPX      BIT(14)
#define LEAF7_EBX_RDSEED   BIT(18)
#define LEAF7_EBX_ADX      BIT(19)

// CPUID.80000001H:EDX
#define EXT1_EDX_LM        BIT(29)

// XCR0 state components
#define XCR0_SSE           (UINT64_C(1) << 1)
#define XCR0_AVX           (UINT64_C(1) << 2)

void x86_cpu_features_read(struct x86_cpu_features* features)
{
	unsigned int eax, ebx, ecx, edx;
	unsigned int max_leaf = __get_cpuid_max(0, NULL);

	*features = (struct x86_cpu_features) {0};

	if (max_leaf >= 1)
	{
		__cpuid_count(1, 0, eax, ebx, ecx, edx);
		features->leaf1_ecx = ecx;
		features->leaf1_edx = edx;
	}
	if (max_leaf >= 7)
	{
		__cpuid_count(7, 0, eax, ebx, ecx, edx);
		features->leaf7_ebx = ebx;
	}

	// XGETBV is only available once the OS has set CR4.OSXSAVE
	if ((features->leaf1_ecx & (LEAF1_ECX_XSAVE | LEAF1_ECX_OSXSAVE)) == (LEAF1_ECX_XSAVE | LEAF1_ECX_OSXSAVE))
	{
		unsigned int lo, hi;
		__asm__ volatile ("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
		features->xcr0 = ((uint64_t) hi << 32) | lo;
	}

	if (__get_cpuid_max(0x80000000, NULL) >= 0x80000001)
	{
		__cpuid_count(0x80000001, 0, eax, ebx, ecx, edx);
		features->long_mode = (edx & EXT1_EDX_LM) != 0;
	}
}

uint64_t x86_cpu_caps_decode(const struct x86_cpu_features* f)
{
	uint64_t caps = 0;
	const uint32_t avx_cpuid = LEAF1_ECX_XSAVE | LEAF1_ECX_OSXSAVE | LEAF1_ECX_AVX;
	bool avx_usable;

	if (f->long_mode)
		caps |= k64Bit;

	if (f->leaf1_edx & LEAF1_EDX_MMX)
		caps |= kHasMMX;
	if (f->leaf1_edx & LEAF1_EDX_SSE)
		caps |= kHasSSE;
	if (f->leaf1_edx & LEAF1_EDX_SSE2)
		caps |= kHasSSE2;

	if (f->leaf1_ecx & LEAF1_ECX_SSE3)
		caps |= kHasSSE3;
	if (f->leaf1_ecx & LEAF1_ECX_SSSE3)
		caps |= kHasSupplementalSSE3;
	if (f->leaf1_ecx & LEAF1_ECX_SSE4_1)
		caps |= kHasSSE4_1;
	if (f->leaf1_ecx & LEAF1_ECX_SSE4_2)
		caps |= kHasSSE4_2;
	if (f->leaf1_ecx & LEAF1_ECX_AES)
		caps |= kHasAES;
	if (f->leaf1_ecx & LEAF1_ECX_RDRAND)
		caps |= kHasRDRAND;

	// AVX and the other VEX-encoded extensions may only be used when the CPU
	// supports them *and* the OS saves the XMM and YMM state on context switches
	// (SDM Vol. 1, "Detection of Intel AVX Instructions").
	avx_usable = (f->leaf1_ecx & avx_cpuid) == avx_cpuid
		&& (f->xcr0 & (XCR0_SSE | XCR0_AVX)) == (XCR0_SSE | XCR0_AVX);

	if (avx_usable)
	{
		caps |= kHasAVX1_0;
		if (f->leaf1_ecx & LEAF1_ECX_FMA)
			caps |= kHasFMA;
		if (f->leaf1_ecx & LEAF1_ECX_F16C)
			caps |= kHasF16C;
		if (f->leaf7_ebx & LEAF7_EBX_AVX2)
			caps |= kHasAVX2_0;
	}

	if (f->leaf7_ebx & LEAF7_EBX_ERMS)
		caps |= kHasENFSTRG;
	if (f->leaf7_ebx & LEAF7_EBX_BMI1)
		caps |= kHasBMI1;
	if (f->leaf7_ebx & LEAF7_EBX_BMI2)
		caps |= kHasBMI2;
	if (f->leaf7_ebx & LEAF7_EBX_RTM)
		caps |= kHasRTM;
	if (f->leaf7_ebx & LEAF7_EBX_HLE)
		caps |= kHasHLE;
	if (f->leaf7_ebx & LEAF7_EBX_RDSEED)
		caps |= kHasRDSEED;
	if (f->leaf7_ebx & LEAF7_EBX_ADX)
		caps |= kHasADX;
	if (f->leaf7_ebx & LEAF7_EBX_MPX)
		caps |= kHasMPX;
	if (f->leaf7_ebx & LEAF7_EBX_SGX)
		caps |= kHasSGX;

	return caps;
}
