// Copyright (C) 2026 Lubos Dolezel
//
// CarbonCore and HIToolbox helpers: OSTypeToCString, the TSM dead-key state
// accessors and _CSGetProcessorArchitecture.
// The program passes unchanged on macOS 27.0.1 (arm64 and x86_64).
#include <CoreServices/CoreServices.h>
#include <mach/machine.h>
#include <stdio.h>
#include <string.h>

extern void OSTypeToCString(OSType type, char *string);
extern int _CSGetProcessorArchitecture(void);
extern UInt32 TSMGetDeadKeyState(void);
extern void TSMSetDeadKeyState(UInt32 state);

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void test_ostype_to_cstring(void)
{
	char buffer[8];

	memset(buffer, 'x', sizeof(buffer));
	OSTypeToCString('ABCD', buffer);
	CHECK(strcmp(buffer, "ABCD") == 0);

	// Bytes outside the printable ASCII range become spaces.
	for (int byte = 0; byte < 256; byte++) {
		memset(buffer, 'x', sizeof(buffer));
		OSTypeToCString(('A' << 24) | (byte << 16) | ('B' << 8) | 'C', buffer);
		int printable = byte >= 32 && byte < 128;
		if ((unsigned char) buffer[1] != (printable ? byte : ' ')) {
			printf("FAIL: byte %d became %d\n", byte, (unsigned char) buffer[1]);
			g_failures++;
		}
		CHECK(buffer[0] == 'A' && buffer[2] == 'B' && buffer[3] == 'C' && buffer[4] == 0);
	}
}

static void test_dead_key_state(void)
{
	CHECK(TSMGetDeadKeyState() == 0);
	TSMSetDeadKeyState(7);
	CHECK(TSMGetDeadKeyState() == 7);
	TSMSetDeadKeyState(0);
	CHECK(TSMGetDeadKeyState() == 0);
}

static void test_processor_architecture(void)
{
#if defined(__x86_64__)
	CHECK(_CSGetProcessorArchitecture() == CPU_TYPE_X86_64);
#elif defined(__i386__)
	CHECK(_CSGetProcessorArchitecture() == CPU_TYPE_I386);
#elif defined(__arm64__)
	CHECK(_CSGetProcessorArchitecture() == CPU_TYPE_ARM64);
#endif
}

int main(void)
{
	test_ostype_to_cstring();
	test_dead_key_state();
	test_processor_architecture();
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
