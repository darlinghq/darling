/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

// system_profiler: the text reports for SPHardwareDataType and
// SPSoftwareDataType. Every value comes from what the system itself reports
// (sysctl, gethostuuid(), the SystemVersion.plist dictionary, uname), so the
// platform identity configured in org.darlinghq.platform shows up here as it
// does everywhere else. Other data types are accepted and print nothing.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPriv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <uuid/uuid.h>

static void print_hardware(void)
{
	char model[128];
	char cpu[256];
	size_t len;
	int cores = 0;
	uint64_t memsize = 0;
	struct timespec wait = { 5, 0 };
	uuid_t uuid;

	printf("Hardware:\n\n");
	printf("    Hardware Overview:\n\n");

	len = sizeof(model);
	if (sysctlbyname("hw.model", model, &len, NULL, 0) == 0)
		printf("      Model Identifier: %s\n", model);

	len = sizeof(cpu);
	if (sysctlbyname("machdep.cpu.brand_string", cpu, &len, NULL, 0) == 0)
	{
#if defined(__arm64__)
		printf("      Chip: %s\n", cpu);
#else
		printf("      Processor Name: %s\n", cpu);
#endif
	}

	len = sizeof(cores);
	if (sysctlbyname("hw.physicalcpu", &cores, &len, NULL, 0) == 0 && cores > 0)
		printf("      Total Number of Cores: %d\n", cores);

	len = sizeof(memsize);
	if (sysctlbyname("hw.memsize", &memsize, &len, NULL, 0) == 0)
		printf("      Memory: %llu GB\n", (unsigned long long) (memsize >> 30));

	if (gethostuuid(uuid, &wait) == 0)
	{
		uuid_string_t string;
		uuid_unparse_upper(uuid, string);
		printf("      Hardware UUID: %s\n", string);
	}

	printf("\n");
}

static void print_software(void)
{
	struct utsname un;
	CFDictionaryRef dict = _CFCopySystemVersionDictionary();

	printf("Software:\n\n");
	printf("    System Software Overview:\n\n");

	if (dict != NULL)
	{
		char name[64], version[64], build[64];
		CFStringRef n = CFDictionaryGetValue(dict, _kCFSystemVersionProductNameKey);
		CFStringRef v = CFDictionaryGetValue(dict, _kCFSystemVersionProductVersionKey);
		CFStringRef b = CFDictionaryGetValue(dict, _kCFSystemVersionBuildVersionKey);

		if (n != NULL && v != NULL && b != NULL
			&& CFStringGetCString(n, name, sizeof(name), kCFStringEncodingUTF8)
			&& CFStringGetCString(v, version, sizeof(version), kCFStringEncodingUTF8)
			&& CFStringGetCString(b, build, sizeof(build), kCFStringEncodingUTF8))
		{
			printf("      System Version: %s %s (%s)\n", name, version, build);
		}
		CFRelease(dict);
	}

	if (uname(&un) == 0)
		printf("      Kernel Version: %s %s\n", un.sysname, un.release);

	printf("\n");
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		print_hardware();
		print_software();
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "SPHardwareDataType") == 0)
			print_hardware();
		else if (strcmp(argv[i], "SPSoftwareDataType") == 0)
			print_software();
	}

	return 0;
}
