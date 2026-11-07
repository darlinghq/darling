// Copyright (C) 2026 Lubos Dolezel
//
// The per-user directories from confstr(3) (_CS_DARWIN_USER_DIR,
// _CS_DARWIN_USER_CACHE_DIR) belong to the effective user and do not
// follow HOME. launchd gives a job that runs under an account HOME, USER,
// LOGNAME and SHELL from the user database (launchd.plist(5), UserName),
// so a daemon sees its own account, not that of whoever started launchd.
// The daemon case uses _trustd, a system account that trustd runs as.
#include <errno.h>
#include <launch.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BOGUS_HOME "/nonexistent/dirhelper_home"
#define DAEMON_USER "_trustd"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

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

static int under(const char* path, const char* dir)
{
	size_t len = strlen(dir);
	return strncmp(path, dir, len) == 0 && path[len] == '/';
}

static int user_dirs(char* user_dir, char* cache_dir)
{
	size_t a = confstr(_CS_DARWIN_USER_DIR, user_dir, PATH_MAX);
	size_t b = confstr(_CS_DARWIN_USER_CACHE_DIR, cache_dir, PATH_MAX);
	return a > 0 && a <= PATH_MAX && b > 0 && b <= PATH_MAX;
}

static void check_dirs(const char* home, const char* what)
{
	char user_dir[PATH_MAX], cache_dir[PATH_MAX];

	CHECK(user_dirs(user_dir, cache_dir));
	if (!under(user_dir, home) || !under(cache_dir, home))
	{
		printf("FAIL: %s: user dir %s, cache dir %s, expected under %s\n", what, user_dir, cache_dir, home);
		g_failures++;
	}
}

// In this process: HOME doesn't matter
static void check_in_process(const struct passwd* pw)
{
	char* saved = getenv("HOME");
	if (saved)
		saved = strdup(saved);

	check_dirs(pw->pw_dir, "inherited HOME");

	setenv("HOME", BOGUS_HOME, 1);
	check_dirs(pw->pw_dir, "foreign HOME");

	unsetenv("HOME");
	check_dirs(pw->pw_dir, "no HOME");

	if (saved)
	{
		setenv("HOME", saved, 1);
		free(saved);
	}
}

// Started by launchd: report what the job got
static int report(void)
{
	char user_dir[PATH_MAX], cache_dir[PATH_MAX];
	const char* vars[] = { "HOME", "USER", "LOGNAME", "SHELL" };

	for (size_t i = 0; i < sizeof(vars) / sizeof(vars[0]); i++)
	{
		const char* value = getenv(vars[i]);
		printf("%s=%s\n", vars[i], value ? value : "(unset)");
	}

	setenv("HOME", BOGUS_HOME, 1);
	if (user_dirs(user_dir, cache_dir))
		printf("USER_DIR=%s\nCACHE_DIR=%s\n", user_dir, cache_dir);
	puts("END");
	fflush(stdout);
	return 0;
}

static launch_data_t string_array(const char* const* items, size_t count)
{
	launch_data_t array = launch_data_alloc(LAUNCH_DATA_ARRAY);
	for (size_t i = 0; i < count; i++)
		launch_data_array_set_index(array, launch_data_new_string(items[i]), i);
	return array;
}

static int send_simple(const char* key, launch_data_t value)
{
	launch_data_t msg = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(msg, value, key);

	launch_data_t resp = launch_msg(msg);
	launch_data_free(msg);
	if (resp == NULL)
		return errno ? errno : -1;

	int err = 0;
	if (launch_data_get_type(resp) == LAUNCH_DATA_ERRNO)
		err = launch_data_get_errno(resp);
	launch_data_free(resp);
	return err;
}

// Looks for "NAME=value" in the job's output
static const char* find_value(const char* text, const char* name, char* out, size_t size)
{
	size_t len = strlen(name);
	for (const char* line = text; line && *line; )
	{
		const char* end = strchr(line, '\n');
		size_t line_len = end ? (size_t) (end - line) : strlen(line);
		if (line_len > len && strncmp(line, name, len) == 0 && line[len] == '=')
		{
			size_t vlen = line_len - len - 1;
			if (vlen >= size)
				vlen = size - 1;
			memcpy(out, line + len + 1, vlen);
			out[vlen] = '\0';
			return out;
		}
		line = end ? end + 1 : NULL;
	}
	return NULL;
}

static void check_value(const char* text, const char* name, const char* expected)
{
	char value[PATH_MAX];
	if (!find_value(text, name, value, sizeof(value)))
	{
		printf("FAIL: the job reported no %s\n", name);
		g_failures++;
	}
	else if (strcmp(value, expected) != 0)
	{
		printf("FAIL: the job got %s=%s, expected %s\n", name, value, expected);
		g_failures++;
	}
}

static void check_launchd_job(const struct passwd* pw)
{
	char self[PATH_MAX], path[PATH_MAX];
	uint32_t size = sizeof(path);

	printf("launchd job as %s\n", pw->pw_name);
	if (_NSGetExecutablePath(path, &size) != 0 || realpath(path, self) == NULL)
	{
		printf("FAIL: cannot find own executable\n");
		g_failures++;
		return;
	}

	char label[128], out_path[PATH_MAX];
	snprintf(label, sizeof(label), "org.darlinghq.test.dirhelper_home.%d.%d", (int) getpid(), (int) pw->pw_uid);
	snprintf(out_path, sizeof(out_path), "/private/tmp/%s.out", label);
	unlink(out_path);

	const char* args[] = { self, "--report" };

	launch_data_t job = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(job, launch_data_new_string(label), LAUNCH_JOBKEY_LABEL);
	launch_data_dict_insert(job, launch_data_new_string(pw->pw_name), LAUNCH_JOBKEY_USERNAME);
	launch_data_dict_insert(job, string_array(args, 2), LAUNCH_JOBKEY_PROGRAMARGUMENTS);
	launch_data_dict_insert(job, launch_data_new_string(out_path), LAUNCH_JOBKEY_STANDARDOUTPATH);
	launch_data_dict_insert(job, launch_data_new_string(out_path), LAUNCH_JOBKEY_STANDARDERRORPATH);
	launch_data_dict_insert(job, launch_data_new_bool(true), LAUNCH_JOBKEY_RUNATLOAD);

	int err = send_simple(LAUNCH_KEY_SUBMITJOB, job);
	if (err != 0)
	{
		printf("FAIL: submitting the job: %s\n", strerror(err));
		g_failures++;
		return;
	}

	// Wait for the job to finish its report
	char text[8192] = "";
	for (int i = 0; i < 150; i++)
	{
		FILE* f = fopen(out_path, "r");
		if (f)
		{
			size_t n = fread(text, 1, sizeof(text) - 1, f);
			text[n] = '\0';
			fclose(f);
			if (strstr(text, "END\n"))
				break;
		}
		usleep(100000);
	}

	send_simple(LAUNCH_KEY_REMOVEJOB, launch_data_new_string(label));
	unlink(out_path);

	if (!strstr(text, "END\n"))
	{
		printf("FAIL: the job didn't report back:\n%s\n", text);
		g_failures++;
		return;
	}

	check_value(text, "HOME", pw->pw_dir);
	check_value(text, "USER", pw->pw_name);
	check_value(text, "LOGNAME", pw->pw_name);
	check_value(text, "SHELL", pw->pw_shell);

	char value[PATH_MAX];
	if (!find_value(text, "USER_DIR", value, sizeof(value)) || !under(value, pw->pw_dir))
	{
		printf("FAIL: the job's user dir isn't under %s\n", pw->pw_dir);
		g_failures++;
	}
	if (!find_value(text, "CACHE_DIR", value, sizeof(value)) || !under(value, pw->pw_dir))
	{
		printf("FAIL: the job's cache dir isn't under %s\n", pw->pw_dir);
		g_failures++;
	}
}

// Later getpw* calls may overwrite the static entry
static void copy_passwd(struct passwd* out, const struct passwd* in)
{
	*out = *in;
	out->pw_name = strdup(in->pw_name);
	out->pw_dir = strdup(in->pw_dir ? in->pw_dir : "");
	out->pw_shell = strdup(in->pw_shell ? in->pw_shell : "");
}

int main(int argc, char** argv)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	if (argc > 1 && strcmp(argv[1], "--report") == 0)
		finish(report());

	struct passwd* found = getpwuid(geteuid());
	if (found == NULL || found->pw_dir == NULL || found->pw_dir[0] == '\0')
	{
		printf("FAIL: no user database entry for uid %d\n", (int) geteuid());
		finish(1);
	}

	struct passwd pw;
	copy_passwd(&pw, found);

	check_in_process(&pw);
	check_launchd_job(&pw);

	// A daemon account from the system's directory, as trustd uses
	found = getpwnam(DAEMON_USER);
	if (found != NULL)
	{
		struct passwd daemon;
		copy_passwd(&daemon, found);
		check_launchd_job(&daemon);
	}
	else
		printf("note: no %s account, daemon check skipped\n", DAEMON_USER);

	if (g_failures == 0)
		puts("PASS");
	finish(g_failures != 0);
	return 0;
}
