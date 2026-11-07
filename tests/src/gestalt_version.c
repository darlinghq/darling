// Copyright (C) 2026 Lubos Dolezel
//
// Gestalt's system version selectors report the macOS product version
// that kern.osproductversion publishes, never the Darwin release:
// - gestaltSystemVersionMajor/Minor/BugFix return its three parts;
// - gestaltSystemVersion packs it as BCD digits: the major version in the
//   high byte, minor and bug-fix in one nibble each, both saturated at 9
//   (documented for 10.x; macOS 27.0.1 returns 0x2701, a quirk);
// - none of them is overwritten by the architecture selector;
// - an unknown selector returns gestaltUndefSelectorErr and a zero response.
#include <CoreServices/CoreServices.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysctl.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures = 0;

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

static void check_selector(const char* name, OSType selector, OSErr expect_err, int expect)
{
	int value = 0x5a5a5a5a; // SInt32 in the SDK; the same type on LP64
	OSErr err = Gestalt(selector, &value);

	if (err != expect_err || value != expect)
	{
		printf("FAIL: Gestalt(%s) = %d, value %d (0x%x); expected %d, value %d (0x%x)\n",
			name, (int) err, (int) value, (unsigned) value,
			(int) expect_err, (int) expect, (unsigned) expect);
		g_failures++;
	}
	else
		printf("ok: Gestalt(%s) = %d, value %d (0x%x)\n", name, (int) err, (int) value, (unsigned) value);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	char product[64] = { 0 };
	size_t len = sizeof(product) - 1;
	if (sysctlbyname("kern.osproductversion", product, &len, NULL, 0) != 0)
	{
		printf("FAIL: sysctl kern.osproductversion is not available\n");
		finish(1);
	}

	int major = 0, minor = 0, bugfix = 0;
	if (sscanf(product, "%d.%d.%d", &major, &minor, &bugfix) < 2)
	{
		printf("FAIL: cannot parse kern.osproductversion \"%s\"\n", product);
		finish(1);
	}
	printf("kern.osproductversion = %s\n", product);

	int packed = ((major / 10) << 12) | ((major % 10) << 8)
		| ((minor > 9 ? 9 : minor) << 4) | (bugfix > 9 ? 9 : bugfix);

	check_selector("gestaltSystemVersionMajor", gestaltSystemVersionMajor, noErr, major);
	check_selector("gestaltSystemVersionMinor", gestaltSystemVersionMinor, noErr, minor);
	check_selector("gestaltSystemVersionBugFix", gestaltSystemVersionBugFix, noErr, bugfix);
	check_selector("gestaltSystemVersion", gestaltSystemVersion, noErr, packed);
	check_selector("'zzzz'", 'zzzz', gestaltUndefSelectorErr, 0);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}
