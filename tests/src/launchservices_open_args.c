// Copyright (C) 2026 Lubos Dolezel
//
// Starting an application bundle with open(1) and LSOpenURLsWithRole:
// - `open -n <bundle> --args ...` starts a new process of the bundle's executable with the
//   arguments after its own path, also while another instance of the same bundle is running;
// - `open -a <bundle> --args ...` starts it with the arguments when it is not running;
// - LSOpenURLsWithRole hands the process LSApplicationParameters' argv and environment;
// - the process runs as the caller's user, with that user's HOME;
// - the process gets the caller's window-system variables (DISPLAY, WAYLAND_DISPLAY), so it
//   reaches the same display as the caller, but not the caller's other variables (an opened
//   application gets the session's environment, as on macOS).
// Run with the display variables of the session set (DISPLAY for X11, WAYLAND_DISPLAY for Wayland).
#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <crt_externs.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define EXEC_NAME "openargs"
#define PROBE "probe"
#define CALLER_ONLY "OPENARGS_CALLER_ONLY"
#define REQUESTED "OPENARGS_REQUESTED"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static bool copyFile(const char* from, const char* to)
{
	char buf[65536];
	ssize_t rd;
	bool ok = true;

	int in = open(from, O_RDONLY);
	if (in < 0)
		return false;
	int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	if (out < 0)
	{
		close(in);
		return false;
	}
	while ((rd = read(in, buf, sizeof(buf))) > 0)
	{
		if (write(out, buf, rd) != rd)
		{
			ok = false;
			break;
		}
	}
	close(in);
	close(out);
	return ok && rd == 0;
}

static bool writeInfoPlist(const char* path)
{
	FILE* f = fopen(path, "w");
	if (!f)
		return false;

	// A fixed name and identifier: every launch of the bundle must still be its own process.
	fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n"
		"\t<key>CFBundleExecutable</key>\n\t<string>" EXEC_NAME "</string>\n"
		"\t<key>CFBundleIdentifier</key>\n\t<string>org.darlinghq.tests.open-args</string>\n"
		"\t<key>CFBundleName</key>\n\t<string>Open Args</string>\n"
		"\t<key>CFBundlePackageType</key>\n\t<string>APPL</string>\n"
		"</dict>\n</plist>\n");
	return fclose(f) == 0;
}

// The launched process: argv[1] is PROBE, argv[2] the report file, argv[3] "hold" to stay
// running for a while. It writes its pid, arguments and variables, then renames the report
// into place so that the test never reads a partial one.
static int runProbe(int argc, const char** argv)
{
	char tmp[PATH_MAX];
	snprintf(tmp, sizeof(tmp), "%s.part", argv[2]);
	FILE* f = fopen(tmp, "w");
	if (!f)
		return 1;
	fprintf(f, "pid=%d\n", (int) getpid());
	fprintf(f, "uid=%d\n", (int) getuid());
	fprintf(f, "argc=%d\n", argc);
	for (int i = 1; i < argc; i++)
		fprintf(f, "arg=%s\n", argv[i]);
	const char* vars[] = { "HOME", "DISPLAY", "WAYLAND_DISPLAY", CALLER_ONLY, REQUESTED };
	for (size_t i = 0; i < sizeof(vars) / sizeof(vars[0]); i++)
	{
		const char* value = getenv(vars[i]);
		fprintf(f, "%s=%s\n", vars[i], value ? value : "(unset)");
	}
	fclose(f);
	rename(tmp, argv[2]);
	if (argc > 3 && strcmp(argv[3], "hold") == 0)
		sleep(5);
	return 0;
}

static int runOpen(const char* const* args)
{
	pid_t pid;
	int status = 0;

	if (posix_spawn(&pid, "/usr/bin/open", NULL, NULL, (char* const*) args, *_NSGetEnviron()) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

// The report of a launched process, or an empty string when none arrives within 10 s.
static void readReport(const char* path, char* buf, size_t size)
{
	buf[0] = 0;
	for (int i = 0; i < 200; i++)
	{
		FILE* f = fopen(path, "r");
		if (f)
		{
			size_t n = fread(buf, 1, size - 1, f);
			buf[n] = 0;
			fclose(f);
			return;
		}
		usleep(50000);
	}
}

static bool hasLine(const char* report, const char* line)
{
	size_t len = strlen(line);
	for (const char* p = report; (p = strstr(p, line)) != NULL; p++)
	{
		if ((p == report || p[-1] == '\n') && p[len] == '\n')
			return true;
	}
	return false;
}

static int reportPid(const char* report)
{
	const char* p = strstr(report, "pid=");
	return p ? atoi(p + 4) : 0;
}

// The launched process runs as the caller's user and sees the caller's display variables.
static void checkDisplay(const char* report)
{
	char user[PATH_MAX];
	struct passwd* pw = getpwuid(getuid());

	snprintf(user, sizeof(user), "uid=%d", (int) getuid());
	CHECK(hasLine(report, user));
	snprintf(user, sizeof(user), "HOME=%s", pw && pw->pw_dir ? pw->pw_dir : "?");
	CHECK(hasLine(report, user));

	const char* vars[] = { "DISPLAY", "WAYLAND_DISPLAY" };
	for (size_t i = 0; i < sizeof(vars) / sizeof(vars[0]); i++)
	{
		const char* value = getenv(vars[i]);
		char line[PATH_MAX];
		snprintf(line, sizeof(line), "%s=%s", vars[i], value ? value : "(unset)");
		if (value)
		{
			CHECK(hasLine(report, line));
			if (!hasLine(report, line))
				printf("  expected %s\n", line);
		}
	}
}

static void waitForExit(int pid)
{
	for (int i = 0; pid > 0 && i < 200 && kill(pid, 0) == 0; i++)
		usleep(50000);
}

int main(int argc, const char** argv)
{
	// Started from the bundle: only ever a probe (without its arguments, it does nothing).
	size_t len = strlen(argv[0]), suffix = strlen("/Contents/MacOS/" EXEC_NAME);
	if (len >= suffix && strcmp(argv[0] + len - suffix, "/Contents/MacOS/" EXEC_NAME) == 0)
		return argc >= 3 && strcmp(argv[1], PROBE) == 0 ? runProbe(argc, argv) : 2;

	char self[PATH_MAX];
	uint32_t size = sizeof(self);
	if (_NSGetExecutablePath(self, &size) != 0)
		return 1;

	if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY"))
	{
		puts("FAIL: run with DISPLAY or WAYLAND_DISPLAY set");
		return 1;
	}
	// Only in the caller; an opened application must not get it.
	setenv(CALLER_ONLY, "1", 1);

	char tmp[] = "/tmp/lsopenargs.XXXXXX";
	if (!mkdtemp(tmp))
		return 1;

	char bundle[PATH_MAX], contents[PATH_MAX], macos[PATH_MAX], exec[PATH_MAX], plist[PATH_MAX];
	char out1[PATH_MAX], out2[PATH_MAX], out3[PATH_MAX], out4[PATH_MAX];
	char report[8192], report2[8192], line[PATH_MAX];

	snprintf(bundle, sizeof(bundle), "%s/Open Args.app", tmp);
	snprintf(contents, sizeof(contents), "%s/Contents", bundle);
	snprintf(macos, sizeof(macos), "%s/MacOS", contents);
	snprintf(exec, sizeof(exec), "%s/" EXEC_NAME, macos);
	snprintf(plist, sizeof(plist), "%s/Info.plist", contents);
	snprintf(out1, sizeof(out1), "%s/out1", tmp);
	snprintf(out2, sizeof(out2), "%s/out2", tmp);
	snprintf(out3, sizeof(out3), "%s/out3", tmp);
	snprintf(out4, sizeof(out4), "%s/out4", tmp);

	CHECK(mkdir(bundle, 0755) == 0);
	CHECK(mkdir(contents, 0755) == 0);
	CHECK(mkdir(macos, 0755) == 0);
	CHECK(copyFile(self, exec));
	CHECK(writeInfoPlist(plist));

	// open -n <bundle> --args: a new process with the arguments, which stays running.
	{
		const char* args[] = { "open", "-n", bundle, "--args", PROBE, out1, "hold", "x", "y z", NULL };
		CHECK(runOpen(args) == 0);
	}
	readReport(out1, report, sizeof(report));
	CHECK(report[0] != 0);
	CHECK(hasLine(report, "argc=6"));
	CHECK(hasLine(report, "arg=" PROBE));
	snprintf(line, sizeof(line), "arg=%s", out1);
	CHECK(hasLine(report, line));
	CHECK(hasLine(report, "arg=hold"));
	CHECK(hasLine(report, "arg=x"));
	CHECK(hasLine(report, "arg=y z"));
	checkDisplay(report);
	CHECK(hasLine(report, CALLER_ONLY "=(unset)"));
	int pid1 = reportPid(report);
	CHECK(pid1 > 0 && pid1 != getpid());

	// open -n again while the first instance runs: another process.
	{
		const char* args[] = { "open", "-n", bundle, "--args", PROBE, out2, "second", NULL };
		CHECK(runOpen(args) == 0);
	}
	readReport(out2, report2, sizeof(report2));
	CHECK(report2[0] != 0);
	CHECK(hasLine(report2, "argc=4"));
	CHECK(hasLine(report2, "arg=second"));
	int pid2 = reportPid(report2);
	CHECK(pid2 > 0 && pid2 != pid1);
	checkDisplay(report2);

	// open -a <bundle> --args once no instance runs: started with the arguments.
	waitForExit(pid1);
	{
		const char* args[] = { "open", "-a", bundle, "--args", PROBE, out3, "a", NULL };
		CHECK(runOpen(args) == 0);
	}
	readReport(out3, report, sizeof(report));
	CHECK(report[0] != 0);
	CHECK(hasLine(report, "argc=4"));
	CHECK(hasLine(report, "arg=a"));
	checkDisplay(report);

	// LSOpenURLsWithRole with LSApplicationParameters' argv and environment.
	{
		FSRef ref;
		CHECK(FSPathMakeRef((const UInt8*) bundle, &ref, NULL) == noErr);

		CFStringRef out = CFStringCreateWithCString(NULL, out4, kCFStringEncodingUTF8);
		const void* items[] = { CFSTR(PROBE), out, CFSTR("k") };
		CFArrayRef appArgv = CFArrayCreate(NULL, items, 3, &kCFTypeArrayCallBacks);
		const void* keys[] = { CFSTR(REQUESTED) };
		const void* values[] = { CFSTR("v") };
		CFDictionaryRef env = CFDictionaryCreate(NULL, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

		LSApplicationParameters params;
		memset(&params, 0, sizeof(params));
		params.flags = kLSLaunchNewInstance;
		params.application = &ref;
		params.argv = appArgv;
		params.environment = env;
		CHECK(LSOpenURLsWithRole(NULL, kLSRolesAll, NULL, &params, NULL, 0) == noErr);

		readReport(out4, report, sizeof(report));
		CHECK(report[0] != 0);
		CHECK(hasLine(report, "argc=4"));
		CHECK(hasLine(report, "arg=k"));
		CHECK(hasLine(report, REQUESTED "=v"));
		CHECK(hasLine(report, CALLER_ONLY "=(unset)"));
		checkDisplay(report);

		CFRelease(env);
		CFRelease(appArgv);
		CFRelease(out);
	}

	unlink(out1);
	unlink(out2);
	unlink(out3);
	unlink(out4);
	unlink(plist);
	unlink(exec);
	rmdir(macos);
	rmdir(contents);
	rmdir(bundle);
	rmdir(tmp);

	if (g_failures == 0)
		puts("PASS");
	return g_failures != 0;
}
