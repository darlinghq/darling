/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The commpage's capability word (<i386/cpu_capabilities.h>, _COMM_PAGE_CPU_CAPABILITIES) carries the cache line
 * size as one of kCache32, kCache64 or kCache128. Libraries such as vImage read it to align row sizes: with no
 * bit set every row size comes out 0. Every x86-64 CPU has 64 byte lines.
 */
#include <stdint.h>
#include <stdio.h>

#define kCache32 0x00000010
#define kCache64 0x00000020
#define kCache128 0x00000040

#if defined(__LP64__)
#define COMMPAGE_BASE 0x00007fffffe00000ULL
#else
#define COMMPAGE_BASE 0xffff0000UL
#endif

int main(void)
{
	uint32_t caps = *(volatile uint32_t*) (COMMPAGE_BASE + 0x20);
	uint64_t caps64 = *(volatile uint64_t*) (COMMPAGE_BASE + 0x10);
	int failures = 0;

	if ((caps & (kCache32 | kCache64 | kCache128)) != kCache64) {
		printf("FAIL: cache line bits of the 32-bit capability word: %#x\n", caps);
		failures++;
	}
	if ((caps64 & (kCache32 | kCache64 | kCache128)) != kCache64) {
		printf("FAIL: cache line bits of the 64-bit capability word: %#llx\n", (unsigned long long) caps64);
		failures++;
	}
	if (!failures)
		puts("PASS");
	return failures != 0;
}
