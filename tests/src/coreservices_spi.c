// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// Two CoreServices private symbols that embedding frameworks import: the options key
// of the private open calls and _CSCheckFixDisable. Resolved through dlsym so the same
// program runs on macOS.
#include <CoreServices/CoreServices.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	int failures = 0;
	CFStringRef *key = dlsym(RTLD_DEFAULT, "_kLSOpenOptionBackgroundLaunchKey");
	void (*checkFixDisable)(void) = dlsym(RTLD_DEFAULT, "_CSCheckFixDisable");

	if (!key || !*key)
	{
		printf("FAIL: _kLSOpenOptionBackgroundLaunchKey missing\n");
		failures++;
	}
	else
	{
		char buffer[128] = "";
		CFStringGetCString(*key, buffer, sizeof buffer, kCFStringEncodingUTF8);
		if (CFGetTypeID(*key) != CFStringGetTypeID() || strcmp(buffer, "_kLSOpenOptionBackgroundLaunchKey") != 0)
		{
			printf("FAIL: key value [%s]\n", buffer);
			failures++;
		}
	}

	if (!checkFixDisable)
	{
		printf("FAIL: _CSCheckFixDisable missing\n");
		failures++;
	}
	else
	{
		checkFixDisable();
		checkFixDisable(); // calling again is harmless
	}

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures ? 1 : 0;
}
