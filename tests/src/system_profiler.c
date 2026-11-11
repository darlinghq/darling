// Copyright (C) 2026 Lubos Dolezel
//
// system_profiler reports what the system itself reports (hw.model,
// gethostuuid(), kern.osproductversion), prints nothing for a data type it
// does not know, and /System/Volumes/Data is the root tree.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <uuid/uuid.h>

static int g_failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "ok" : "FAIL", what);
	if (!ok)
		g_failures++;
}

static char* run(const char* command, int* status)
{
	FILE* pipe = popen(command, "r");
	char* out = calloc(1, 16384);
	if (pipe == NULL)
	{
		*status = -1;
		return out;
	}
	fread(out, 1, 16383, pipe);
	*status = pclose(pipe);
	return out;
}

int main(void)
{
	int status;
	char model[128], version[64];
	size_t len = sizeof(model);
	char expected[256];
	char* out;

	if (sysctlbyname("hw.model", model, &len, NULL, 0) != 0)
	{
		printf("FAIL: hw.model is not available\n");
		return 1;
	}
	len = sizeof(version);
	if (sysctlbyname("kern.osproductversion", version, &len, NULL, 0) != 0)
	{
		printf("FAIL: kern.osproductversion is not available\n");
		return 1;
	}

	out = run("/usr/sbin/system_profiler SPHardwareDataType", &status);
	check(status == 0, "system_profiler SPHardwareDataType succeeds");
	check(strncmp(out, "Hardware:\n", 10) == 0, "the report starts with the Hardware: heading");
	snprintf(expected, sizeof(expected), "Model Identifier: %s\n", model);
	check(strstr(out, expected) != NULL, "Model Identifier is hw.model");
	{
		uuid_t uuid;
		uuid_string_t string;
		struct timespec wait = { 5, 0 };
		if (gethostuuid(uuid, &wait) == 0)
		{
			uuid_unparse_upper(uuid, string);
			snprintf(expected, sizeof(expected), "Hardware UUID: %s\n", string);
			check(strstr(out, expected) != NULL, "Hardware UUID is gethostuuid()");
		}
	}
	free(out);

	out = run("/usr/sbin/system_profiler SPSoftwareDataType", &status);
	check(status == 0, "system_profiler SPSoftwareDataType succeeds");
	check(strncmp(out, "Software:\n", 10) == 0, "the report starts with the Software: heading");
	snprintf(expected, sizeof(expected), "System Version: macOS %s (", version);
	check(strstr(out, expected) != NULL, "System Version carries kern.osproductversion");
	check(strstr(out, "Kernel Version: Darwin ") != NULL, "Kernel Version is listed");
	free(out);

	out = run("/usr/sbin/system_profiler", &status);
	check(strstr(out, "Hardware:\n") != NULL && strstr(out, "Software:\n") != NULL,
		"with no arguments both overviews are printed");
	free(out);

	out = run("/usr/sbin/system_profiler SPNoSuchDataType", &status);
	check(status == 0 && out[0] == '\0', "an unknown data type prints nothing and succeeds");
	free(out);

	{
		struct stat root, data;
		check(stat("/etc", &root) == 0 && stat("/System/Volumes/Data/etc", &data) == 0
			&& root.st_ino == data.st_ino && root.st_dev == data.st_dev,
			"/System/Volumes/Data/etc is /etc");
	}

	printf(g_failures ? "FAILED\n" : "PASSED\n");
	return g_failures != 0;
}
