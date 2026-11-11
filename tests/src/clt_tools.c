// Copyright (C) 2026 Lubos Dolezel
//
// softwareupdate lists the Command Line Tools exactly when they are not
// installed, and the DarlingCLT xcrun reports a missing tool name or tool
// instead of crashing, and finds a tool on PATH when it is not next to xcrun.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int g_failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "ok" : "FAIL", what);
	if (!ok)
		g_failures++;
}

static char* run(const char* command, int* exitCode)
{
	FILE* pipe = popen(command, "r");
	char* out = calloc(1, 16384);
	int status;
	if (pipe == NULL)
	{
		*exitCode = -1;
		return out;
	}
	fread(out, 1, 16383, pipe);
	status = pclose(pipe);
	*exitCode = (status >> 8) & 0xff;
	return out;
}

int main(void)
{
	const char* xcrun = "/Library/Developer/DarlingCLT/usr/bin/xcrun";
	int code;
	char* out;
	char dir[] = "/tmp/clt_tools.XXXXXX";
	char path[256], command[1024];
	FILE* file;

	out = run("/usr/sbin/softwareupdate -l", &code);
	check(code == 0, "softwareupdate -l succeeds");
	const char* heading = "Software Update Tool\n\nFinding available software\n";
	check(strncmp(out, heading, strlen(heading)) == 0, "softwareupdate -l prints its heading");
	if (access("/Library/Developer/CommandLineTools/usr/bin/git", F_OK) != 0)
		check(strstr(out, "* Label: Command Line Tools") != NULL, "missing Command Line Tools are listed");
	else
		check(strstr(out, "No new software available.") != NULL, "installed Command Line Tools are not listed");
	free(out);

	out = run("/usr/sbin/softwareupdate -i", &code);
	check(code == 0 && strstr(out, "Installing") == NULL, "softwareupdate -i without a label installs nothing");
	free(out);

	snprintf(command, sizeof(command), "%s 2>&1", xcrun);
	out = run(command, &code);
	check(code == 1 && strstr(out, "no tool specified") != NULL, "xcrun without arguments reports the missing tool");
	free(out);

	snprintf(command, sizeof(command), "%s xcrun-no-such-tool 2>&1", xcrun);
	out = run(command, &code);
	check(code == 1 && strstr(out, "cannot find tool") != NULL, "xcrun reports a tool it cannot find");
	free(out);

	if (mkdtemp(dir) == NULL)
	{
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	snprintf(path, sizeof(path), "%s/xcrun-test-tool", dir);
	file = fopen(path, "w");
	fprintf(file, "#!/bin/sh\necho tool-ran $1\n");
	fclose(file);
	chmod(path, 0755);

	snprintf(command, sizeof(command), "PATH=%s:$PATH %s xcrun-test-tool arg 2>&1", dir, xcrun);
	out = run(command, &code);
	check(code == 0 && strstr(out, "tool-ran arg") != NULL, "xcrun runs a tool found on PATH");
	free(out);

	unlink(path);
	rmdir(dir);
	printf(g_failures ? "FAILED\n" : "PASSED\n");
	return g_failures != 0;
}
