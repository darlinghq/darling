// Copyright (C) 2026 Lubos Dolezel
//
// HiWord and LoWord of the 32-bit Toolbox Utilities return the high and the low 16 bits of a long as a signed
// short. They are not part of the 64-bit interfaces.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
#ifdef __LP64__
	printf("SKIP carboncore_hiword_loword (32-bit only)\n");
	return 0;
#else
	CHECK(HiWord(0x12345678) == 0x1234);
	CHECK(LoWord(0x12345678) == 0x5678);
	CHECK(HiWord(0) == 0 && LoWord(0) == 0);
	CHECK(HiWord(0x00018000) == 1 && LoWord(0x00018000) == -32768);
	CHECK(HiWord(0x7fff7fff) == 0x7fff && LoWord(0x7fff7fff) == 0x7fff);
	CHECK(HiWord((long) 0x80000000) == -32768 && LoWord((long) 0x80000000) == 0);
	CHECK(HiWord(-1) == -1 && LoWord(-1) == -1);
	CHECK(HiWord((long) 0xffff0001) == -1 && LoWord((long) 0xffff0001) == 1);
	if (g_failures) {
		printf("%d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS carboncore_hiword_loword\n");
	return 0;
#endif
}
