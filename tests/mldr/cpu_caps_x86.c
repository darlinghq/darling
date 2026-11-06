// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of the commpage CPU capability bits on x86. Each kHas* bit must
// come from the CPUID register and bit the Intel SDM defines for that feature, and
// AVX (with FMA, F16C and AVX2) may only be reported when the OS has enabled
// the XMM and YMM state in XCR0.
#include <stdio.h>
#include "cpu_caps_x86.h"

#define PRIVATE
#include <i386/cpu_capabilities.h>

#define BIT(n) (UINT32_C(1) << (n))

// CPUID.01H:ECX bits needed for AVX: XSAVE, OSXSAVE, AVX
#define AVX_CPUID (BIT(26) | BIT(27) | BIT(28))
// XCR0: SSE and AVX state enabled
#define XCR0_YMM 0x6

static int failures;

static void check(const char* what, struct x86_cpu_features f, uint64_t expected)
{
	uint64_t caps = x86_cpu_caps_decode(&f);

	if (caps != expected)
	{
		printf("FAIL: %s: got 0x%llx, expected 0x%llx\n", what,
			(unsigned long long) caps, (unsigned long long) expected);
		failures++;
	}
}

int main(void)
{
	check("no features", (struct x86_cpu_features) {0}, 0);

	// CPUID.01H:EDX
	check("EDX.MMX", (struct x86_cpu_features) { .leaf1_edx = BIT(23) }, kHasMMX);
	check("EDX.SSE", (struct x86_cpu_features) { .leaf1_edx = BIT(25) }, kHasSSE);
	check("EDX.SSE2", (struct x86_cpu_features) { .leaf1_edx = BIT(26) }, kHasSSE2);
	// EDX bit 30 is IA64, not long mode
	check("EDX bit 30", (struct x86_cpu_features) { .leaf1_edx = BIT(30) }, 0);

	// CPUID.01H:ECX
	check("ECX.SSE3", (struct x86_cpu_features) { .leaf1_ecx = BIT(0) }, kHasSSE3);
	check("ECX.SSSE3", (struct x86_cpu_features) { .leaf1_ecx = BIT(9) }, kHasSupplementalSSE3);
	check("ECX.SSE4_1", (struct x86_cpu_features) { .leaf1_ecx = BIT(19) }, kHasSSE4_1);
	check("ECX.SSE4_2", (struct x86_cpu_features) { .leaf1_ecx = BIT(20) }, kHasSSE4_2);
	check("ECX.AES", (struct x86_cpu_features) { .leaf1_ecx = BIT(25) }, kHasAES);
	check("ECX.RDRAND", (struct x86_cpu_features) { .leaf1_ecx = BIT(30) }, kHasRDRAND);
	// ECX bits 23, 25 and 26 are POPCNT, AES and XSAVE, not MMX/SSE/SSE2
	check("ECX bits 23, 26", (struct x86_cpu_features) { .leaf1_ecx = BIT(23) | BIT(26) }, 0);

	// CPUID.(EAX=07H,ECX=0):EBX
	check("leaf 7 EBX", (struct x86_cpu_features) {
		.leaf7_ebx = BIT(2) | BIT(3) | BIT(4) | BIT(8) | BIT(9) | BIT(11) | BIT(14) | BIT(18) | BIT(19)
	}, kHasSGX | kHasBMI1 | kHasHLE | kHasBMI2 | kHasENFSTRG | kHasRTM | kHasMPX | kHasRDSEED | kHasADX);

	// CPUID.80000001H:EDX.LM
	check("long mode", (struct x86_cpu_features) { .long_mode = true }, k64Bit);

	// AVX and the extensions that depend on its register state
	check("AVX usable", (struct x86_cpu_features) {
		.leaf1_ecx = AVX_CPUID | BIT(12) | BIT(29), .leaf7_ebx = BIT(5), .xcr0 = XCR0_YMM
	}, kHasAVX1_0 | kHasFMA | kHasF16C | kHasAVX2_0);
	check("AVX without YMM state in XCR0", (struct x86_cpu_features) {
		.leaf1_ecx = AVX_CPUID | BIT(12) | BIT(29), .leaf7_ebx = BIT(5), .xcr0 = 0x2
	}, 0);
	check("AVX without OSXSAVE", (struct x86_cpu_features) {
		.leaf1_ecx = (AVX_CPUID & ~BIT(27)) | BIT(12) | BIT(29), .leaf7_ebx = BIT(5), .xcr0 = XCR0_YMM
	}, 0);
	check("AVX not supported", (struct x86_cpu_features) {
		.leaf1_ecx = (AVX_CPUID & ~BIT(28)) | BIT(12) | BIT(29), .leaf7_ebx = BIT(5), .xcr0 = XCR0_YMM
	}, 0);

#ifdef __x86_64__
	// Every x86-64 CPU has SSE2 and long mode.
	{
		struct x86_cpu_features f;
		uint64_t caps;

		x86_cpu_features_read(&f);
		caps = x86_cpu_caps_decode(&f);
		if ((caps & (kHasSSE2 | k64Bit)) != (kHasSSE2 | k64Bit))
		{
			printf("FAIL: this CPU reports 0x%llx, without SSE2 or 64-bit\n", (unsigned long long) caps);
			failures++;
		}
	}
#endif

	if (failures)
		return 1;

	printf("cpu_caps_x86: OK\n");
	return 0;
}
