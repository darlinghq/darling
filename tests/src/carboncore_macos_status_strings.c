// Copyright (C) 2026 Lubos Dolezel
//
// GetMacOSStatusErrorString() returns the MacErrors.h constant name(s) for an
// OSStatus, constants sharing a value joined by " / ", or "" when unknown.
// GetMacOSStatusCommentString() never returns NULL.
// Documented in CarbonCore/Debugging.h.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void check_name(OSStatus code, const char *expected)
{
	const char *s = GetMacOSStatusErrorString(code);
	if (s == NULL || strcmp(s, expected) != 0) {
		printf("FAIL: %d -> \"%s\", expected \"%s\"\n", (int) code, s ? s : "(null)", expected);
		g_failures++;
	}
}

int main(void)
{
	check_name(0, "noErr");
	check_name(-43, "fnfErr");
	check_name(-36, "ioErr");
	check_name(-50, "paramErr");
	check_name(-128, "userCanceledErr");
	check_name(-27, "abortErr / iIOAbortErr / iIOAbort");
	check_name(-999999, "");
	check_name(123456, "");

	CHECK(GetMacOSStatusCommentString(-43) != NULL);
	CHECK(GetMacOSStatusCommentString(-999999) != NULL);
	CHECK(strcmp(GetMacOSStatusCommentString(-999999), "") == 0);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
