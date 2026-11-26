// Copyright (C) 2026 Lubos Dolezel
//
// End-to-end verification of configurable reported macOS version per prefix:
// - sysctl kern.osproductversion, kern.osversion, kern.osrelease, kern.version
// - uname -r, uname -v
// - Gestalt (gestaltSystemVersionMajor, Minor, BugFix)
// - NSProcessInfo operatingSystemVersion, operatingSystemVersionString,
//   and isOperatingSystemAtLeastVersion
// - __builtin_available(macOS ..., *)
// - /System/Library/CoreServices/SystemVersion.plist and SystemVersionCompat.plist
// - sw_vers command output
//
// When run with no arguments, this test acts as an orchestrator:
// it configures different macOS versions in /Library/Preferences/org.darlinghq.platform.plist,
// executes `darling-prefsd --push-once`, and verifies each scenario in a fresh process.
#include <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <CoreServices/CoreServices.h>

#define PREF_PATH "/Library/Preferences/org.darlinghq.platform.plist"
#define PREFSD_BIN "/usr/libexec/darling-prefsd"

static int g_failures = 0;

static void test_check(int ok, const char* what)
{
	printf("  %s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		g_failures++;
}

// Runtime hook for Clang's __builtin_available
int32_t __isPlatformVersionAtLeast(uint32_t platform, uint32_t major, uint32_t minor, uint32_t subminor)
{
	char version[48] = {0};
	size_t len = sizeof(version) - 1;
	if (sysctlbyname("kern.osproductversion", version, &len, NULL, 0) != 0)
		return 0;
	uint32_t maj = 0, min = 0, patch = 0;
	sscanf(version, "%u.%u.%u", &maj, &min, &patch);
	if (platform != 1 /* macOS */)
		return 1;
	if (maj < major)
		return 0;
	if (maj > major)
		return 1;
	if (min < minor)
		return 0;
	if (min > minor)
		return 1;
	return patch >= subminor;
}

static char* run_cmd(const char* command)
{
	FILE* pipe = popen(command, "r");
	char* out = calloc(1, 4096);
	if (pipe == NULL)
		return out;
	fread(out, 1, 4095, pipe);
	pclose(pipe);
	return out;
}

static int run_checks(const char* expected_product, const char* expected_version,
	const char* expected_release, const char* expected_ios)
{
	int exp_major = 0, exp_minor = 0, exp_patch = 0;
	sscanf(expected_product, "%d.%d.%d", &exp_major, &exp_minor, &exp_patch);

	printf("Checking reported macOS identity (product=%s, build=%s, release=%s, ios=%s):\n",
		expected_product, expected_version, expected_release, expected_ios ? expected_ios : "(omitted)");

	// 1. sysctl kern.osproductversion
	char val[128];
	size_t len = sizeof(val);
	memset(val, 0, sizeof(val));
	if (sysctlbyname("kern.osproductversion", val, &len, NULL, 0) == 0)
		test_check(strcmp(val, expected_product) == 0, "sysctl kern.osproductversion matches expected");
	else
		test_check(0, "sysctl kern.osproductversion failed");

	// 2. sysctl kern.osversion
	len = sizeof(val);
	memset(val, 0, sizeof(val));
	if (sysctlbyname("kern.osversion", val, &len, NULL, 0) == 0)
		test_check(strcmp(val, expected_version) == 0, "sysctl kern.osversion matches expected");
	else
		test_check(0, "sysctl kern.osversion failed");

	// 3. sysctl kern.osrelease
	len = sizeof(val);
	memset(val, 0, sizeof(val));
	if (sysctlbyname("kern.osrelease", val, &len, NULL, 0) == 0)
		test_check(strcmp(val, expected_release) == 0, "sysctl kern.osrelease matches expected");
	else
		test_check(0, "sysctl kern.osrelease failed");

	// 4. sysctl kern.version
	char kver[256];
	len = sizeof(kver);
	memset(kver, 0, sizeof(kver));
	if (sysctlbyname("kern.version", kver, &len, NULL, 0) == 0)
	{
		char expected_prefix[128];
		snprintf(expected_prefix, sizeof(expected_prefix), "Darwin Kernel Version %s", expected_release);
		test_check(strstr(kver, expected_prefix) != NULL, "sysctl kern.version contains expected release");
	}
	else
		test_check(0, "sysctl kern.version failed");

	// 5. uname
	struct utsname un;
	if (uname(&un) == 0)
	{
		test_check(strcmp(un.release, expected_release) == 0, "uname release matches expected");
		test_check(strstr(un.version, expected_release) != NULL, "uname version contains expected release");
	}
	else
		test_check(0, "uname failed");

	// 6. Gestalt
	int g_major = 0, g_minor = 0, g_patch = 0;
	if (Gestalt(gestaltSystemVersionMajor, &g_major) == noErr)
		test_check(g_major == exp_major, "Gestalt major matches expected");
	else
		test_check(0, "Gestalt major failed");

	if (Gestalt(gestaltSystemVersionMinor, &g_minor) == noErr)
		test_check(g_minor == exp_minor, "Gestalt minor matches expected");
	else
		test_check(0, "Gestalt minor failed");

	if (Gestalt(gestaltSystemVersionBugFix, &g_patch) == noErr)
		test_check(g_patch == exp_patch, "Gestalt bugfix/patch matches expected");
	else
		test_check(0, "Gestalt bugfix/patch failed");

	// 7. NSProcessInfo
	@autoreleasepool {
		NSProcessInfo* pi = [NSProcessInfo processInfo];
		NSOperatingSystemVersion osv = [pi operatingSystemVersion];
		test_check((int) osv.majorVersion == exp_major, "NSProcessInfo majorVersion matches expected");
		test_check((int) osv.minorVersion == exp_minor, "NSProcessInfo minorVersion matches expected");
		test_check((int) osv.patchVersion == exp_patch, "NSProcessInfo patchVersion matches expected");

		NSString* pi_str = [pi operatingSystemVersionString];
		// "Version <product version> (Build <build>)". A binary built for an SDK before macOS 11 sees the
		// compatibility version 10.16 for macOS 11 and later, so only the build is compared exactly.
		NSString* expected_build = [NSString stringWithFormat:@"(Build %s)", expected_version];
		test_check(pi_str != nil && [pi_str hasPrefix:@"Version "] && [pi_str hasSuffix:expected_build],
			"NSProcessInfo operatingSystemVersionString has the expected build");

		test_check([pi isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){exp_major, exp_minor, exp_patch}],
			"NSProcessInfo isOperatingSystemAtLeastVersion current version is YES");
		test_check(![pi isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){exp_major + 1, 0, 0}],
			"NSProcessInfo isOperatingSystemAtLeastVersion next major is NO");
	}

	// 8. __builtin_available
	if (exp_major >= 12)
	{
		if (__builtin_available(macOS 12.0, *))
			test_check(1, "__builtin_available macOS 12.0 is true for >= 12");
		else
			test_check(0, "__builtin_available macOS 12.0 is true for >= 12");
	}
	else
	{
		if (__builtin_available(macOS 12.0, *))
			test_check(0, "__builtin_available macOS 12.0 is false for < 12");
		else
			test_check(1, "__builtin_available macOS 12.0 is false for < 12");
	}

	if (__builtin_available(macOS 99.0, *))
		test_check(0, "__builtin_available macOS 99.0 is false");
	else
		test_check(1, "__builtin_available macOS 99.0 is false");

	// 9. SystemVersion.plist
	NSDictionary* sysDict = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
	if (sysDict != nil)
	{
		const char* exp_pname = (exp_major < 11) ? "Mac OS X" : "macOS";
		test_check([[sysDict objectForKey:@"ProductName"] isEqualToString:[NSString stringWithUTF8String:exp_pname]],
			"SystemVersion.plist ProductName matches major (<11 Mac OS X, >=11 macOS)");
		test_check([[sysDict objectForKey:@"ProductVersion"] isEqualToString:[NSString stringWithUTF8String:expected_product]],
			"SystemVersion.plist ProductVersion matches expected");
		test_check([[sysDict objectForKey:@"ProductBuildVersion"] isEqualToString:[NSString stringWithUTF8String:expected_version]],
			"SystemVersion.plist ProductBuildVersion matches expected");
		test_check([[sysDict objectForKey:@"ProductUserVisibleVersion"] isEqualToString:[NSString stringWithUTF8String:expected_product]],
			"SystemVersion.plist ProductUserVisibleVersion matches expected");

		NSString* actual_ios = [sysDict objectForKey:@"iOSSupportVersion"];
		if (expected_ios != NULL && expected_ios[0] != '\0')
			test_check([actual_ios isEqualToString:[NSString stringWithUTF8String:expected_ios]],
				"SystemVersion.plist iOSSupportVersion matches expected");
		else
			test_check(actual_ios == nil, "SystemVersion.plist iOSSupportVersion omitted for < 10.14");
	}
	else
		test_check(0, "failed to read /System/Library/CoreServices/SystemVersion.plist");

	// 10. SystemVersionCompat.plist
	if (exp_major >= 11)
	{
		NSDictionary* compatDict = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersionCompat.plist"];
		if (compatDict != nil)
		{
			test_check([[compatDict objectForKey:@"ProductName"] isEqualToString:@"Mac OS X"],
				"SystemVersionCompat.plist ProductName is Mac OS X");
			test_check([[compatDict objectForKey:@"ProductVersion"] isEqualToString:@"10.16"],
				"SystemVersionCompat.plist ProductVersion is 10.16");
			test_check([[compatDict objectForKey:@"ProductBuildVersion"] isEqualToString:[NSString stringWithUTF8String:expected_version]],
				"SystemVersionCompat.plist ProductBuildVersion matches expected");
		}
		else
			test_check(0, "SystemVersionCompat.plist missing for >= 11");
	}
	else
	{
		test_check(access("/System/Library/CoreServices/SystemVersionCompat.plist", F_OK) != 0,
			"SystemVersionCompat.plist does NOT exist for < 11");
	}

	// 11. sw_vers command
	char* sw_out = run_cmd("/usr/bin/sw_vers");
	if (sw_out != NULL)
	{
		char expected_pver_line[128];
		snprintf(expected_pver_line, sizeof(expected_pver_line), "ProductVersion: %s", expected_product);
		test_check(strstr(sw_out, expected_pver_line) != NULL, "sw_vers contains expected ProductVersion");

		char expected_bver_line[128];
		snprintf(expected_bver_line, sizeof(expected_bver_line), "BuildVersion:   %s", expected_version);
		test_check(strstr(sw_out, expected_bver_line) != NULL, "sw_vers contains expected BuildVersion");

		const char* exp_pname = (exp_major < 11) ? "Mac OS X" : "macOS";
		test_check(strstr(sw_out, exp_pname) != NULL, "sw_vers contains expected ProductName");
		free(sw_out);
	}
	else
		test_check(0, "failed to run /usr/bin/sw_vers");

	return g_failures ? 1 : 0;
}

static bool run_prefsd(void)
{
	int status = system(PREFSD_BIN " --push-once >/dev/null 2>&1");
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static bool write_platform_prefs(NSDictionary* dict)
{
	unlink(PREF_PATH);
	if (dict == nil)
		return true;
	return [dict writeToFile:@PREF_PATH atomically:YES];
}

static int spawn_check(const char* self_bin, const char* product, const char* build, const char* release, const char* ios)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		return 1;
	}
	if (pid == 0)
	{
		setenv("SYSTEM_VERSION_COMPAT", "0", 1);
		if (ios && ios[0] != '\0')
			execl(self_bin, self_bin, "--check", product, build, release, ios, NULL);
		else
			execl(self_bin, self_bin, "--check", product, build, release, "", NULL);
		perror("execl");
		_exit(127);
	}

	int status = 0;
	waitpid(pid, &status, 0);
	return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : 1;
}

int main(int argc, char* argv[])
{
	if (argc >= 5 && strcmp(argv[1], "--check") == 0)
	{
		const char* product = argv[2];
		const char* build = argv[3];
		const char* release = argv[4];
		const char* ios = (argc >= 6 && argv[5][0] != '\0') ? argv[5] : NULL;
		return run_checks(product, build, release, ios);
	}

	const char* self_bin = argv[0];
	int overall_failures = 0;

	printf("=== Running prefix macOS version integration test suite ===\n\n");

	// Scenario 1: Default macOS version (no preferences)
	printf("Scenario 1: Default macOS identity (unconfigured)\n");
	write_platform_prefs(nil);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for defaults");
	if (spawn_check(self_bin, "26.4", "25E246", "25.4.0", "26.4") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 2: macOS 12 Monterey
	printf("Scenario 2: Configured macOS 12 Monterey (12.6.8 / 21G725 / 21.6.0)\n");
	NSDictionary* prefs12 = @{
		@"OSProductVersion": @"12.6.8",
		@"OSVersion": @"21G725",
		@"OSRelease": @"21.6.0"
	};
	write_platform_prefs(prefs12);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for macOS 12");
	if (spawn_check(self_bin, "12.6.8", "21G725", "21.6.0", "15.6") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 3: macOS 10.15 Catalina
	printf("Scenario 3: Configured macOS 10.15 Catalina (10.15.7 / 19H2 / 19.6.0)\n");
	NSDictionary* prefs1015 = @{
		@"OSProductVersion": @"10.15.7",
		@"OSVersion": @"19H2",
		@"OSRelease": @"19.6.0"
	};
	write_platform_prefs(prefs1015);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for macOS 10.15");
	if (spawn_check(self_bin, "10.15.7", "19H2", "19.6.0", "13.4") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 4: macOS 10.14 Mojave
	printf("Scenario 4: Configured macOS 10.14 Mojave (10.14.6 / 18G103 / 18.7.0)\n");
	NSDictionary* prefs1014 = @{
		@"OSProductVersion": @"10.14.6",
		@"OSVersion": @"18G103",
		@"OSRelease": @"18.7.0"
	};
	write_platform_prefs(prefs1014);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for macOS 10.14");
	if (spawn_check(self_bin, "10.14.6", "18G103", "18.7.0", "12.2") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 5: Explicit OSiOSSupportVersion override
	printf("Scenario 5: Explicit OSiOSSupportVersion override\n");
	NSDictionary* prefsOverride = @{
		@"OSProductVersion": @"12.6.8",
		@"OSVersion": @"21G725",
		@"OSRelease": @"21.6.0",
		@"OSiOSSupportVersion": @"16.2"
	};
	write_platform_prefs(prefsOverride);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for OSiOSSupportVersion override");
	if (spawn_check(self_bin, "12.6.8", "21G725", "21.6.0", "16.2") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 6: Partial key configuration rejection
	printf("Scenario 6: Partial configuration rejection (only OSProductVersion specified)\n");
	NSDictionary* partialPrefs = @{
		@"OSProductVersion": @"15.0"
	};
	write_platform_prefs(partialPrefs);
	test_check(run_prefsd(), "darling-prefsd --push-once ran for partial configuration");
	// Partial configuration must be rejected in favor of defaults (26.4), never applying partial 15.0
	if (spawn_check(self_bin, "26.4", "25E246", "25.4.0", "26.4") != 0)
		overall_failures++;
	printf("\n");

	// Scenario 7: Revert to default
	printf("Scenario 7: Clean up and revert to default\n");
	write_platform_prefs(nil);
	test_check(run_prefsd(), "darling-prefsd --push-once succeeded for revert");
	if (spawn_check(self_bin, "26.4", "25E246", "25.4.0", "26.4") != 0)
		overall_failures++;
	printf("\n");

	printf("Integration test suite result: %s (%d failure(s))\n",
		overall_failures == 0 ? "SUCCESS (ALL PASSED)" : "FAILED", overall_failures);
	return overall_failures ? 1 : 0;
}
