#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CANDIDATES 8
#define PATH_MAX_LEN 4096
#define BUFFER_SIZE (MAX_CANDIDATES * PATH_MAX_LEN)

size_t fexcore_candidate_paths(char* buf, size_t buf_size, size_t max_paths,
	const char** out, const char* self_exe);

static const char* installed_fexcore = INSTALL_PREFIX "/libexec/darling/lib/libdarling-fexcore.so";

static int failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// The returned pointers alias `buf`, which is reused by the next call.
static size_t discover(const char* self_exe, size_t buf_size, size_t max_paths, const char** out)
{
	static char buf[BUFFER_SIZE];
	return fexcore_candidate_paths(buf, buf_size, max_paths, out, self_exe);
}

static void test_override_wins(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	setenv("DARLING_FEXCORE_LIB", "/somewhere/libFEXCore.so", 1);
	count = discover("", BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 1, "override yields exactly one candidate");
	check(count == 1 && strcmp(out[0], "/somewhere/libFEXCore.so") == 0,
		"override candidate is DARLING_FEXCORE_LIB verbatim");
	unsetenv("DARLING_FEXCORE_LIB");
}

static void test_empty_override_ignored(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	setenv("DARLING_FEXCORE_LIB", "", 1);
	count = discover("/opt/darling/bin/mldr", BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 4, "empty override falls back to discovery");
	unsetenv("DARLING_FEXCORE_LIB");
}

static void test_discovery_order(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = discover("/opt/darling/libexec/darling/bin/mldr", BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 4, "four candidates next to an installed mldr");
	check(count > 0
			&& strcmp(out[0], "/opt/darling/libexec/darling/bin/../lib/libdarling-fexcore.so") == 0,
		"the lib directory above the bin install is tried first");
	check(count > 1
			&& strcmp(out[1], "/opt/darling/libexec/darling/bin/../../../lib/libdarling-fexcore.so") == 0,
		"the lib directory above the usr/libexec install is tried second");
	check(count > 2 && strcmp(out[2], installed_fexcore) == 0,
		"configure-time install prefix is tried third");
	check(count > 3 && strcmp(out[3], "libFEXCore.so") == 0,
		"the dynamic loader's search path is the last resort");

	printf("first candidate: %s\n", count > 0 ? out[0] : "(none)");
	printf("install prefix candidate: %s\n", installed_fexcore);
}

static void test_usr_libexec_layout(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = discover("/opt/darling/libexec/darling/usr/libexec/darling/mldr", BUFFER_SIZE,
		MAX_CANDIDATES, out);
	check(count == 4, "the usr/libexec install layout still yields four candidates");
	check(count > 1
			&& strcmp(out[1],
				   "/opt/darling/libexec/darling/usr/libexec/darling/../../../lib/libdarling-fexcore.so")
				== 0,
		"the usr/libexec install reaches the shim three levels up");
}

static void test_relative_self_exe(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = discover("mldr", BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 4, "a self path with no directory still yields all candidates");
	check(count > 0 && strcmp(out[0], "./../lib/libdarling-fexcore.so") == 0,
		"a self path with no directory is relative to the working directory");
}

static void test_unknown_self_exe(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = discover(NULL, BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 2, "without a self path only the install prefix and loader search remain");
	check(count > 0 && strcmp(out[0], installed_fexcore) == 0,
		"install prefix comes first without a self path");
	check(count > 1 && strcmp(out[1], "libFEXCore.so") == 0,
		"loader search path stays last without a self path");
}

static void test_capacity_limits(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = discover("/opt/darling/bin/mldr", BUFFER_SIZE, 2, out);
	check(count == 2, "max_paths caps the number of candidates");

	count = discover("/opt/darling/bin/mldr", 1, MAX_CANDIDATES, out);
	check(count == 0, "a one-byte buffer holds no candidate");
}

static void test_bad_arguments(void)
{
	char buf[PATH_MAX_LEN];
	const char* out[MAX_CANDIDATES];
	size_t count;

	count = fexcore_candidate_paths(NULL, sizeof(buf), MAX_CANDIDATES, out, "/opt/bin/mldr");
	check(count == 0, "a null buffer yields no candidates");

	count = fexcore_candidate_paths(buf, sizeof(buf), MAX_CANDIDATES, NULL, "/opt/bin/mldr");
	check(count == 0, "a null output array yields no candidates");

	count = fexcore_candidate_paths(buf, sizeof(buf), 0, out, "/opt/bin/mldr");
	check(count == 0, "a zero candidate limit yields no candidates");
}

static void test_no_environment_dependence(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;

	unsetenv("DARLING_FEXCORE_LIB");
	count = discover("/opt/darling/bin/mldr", BUFFER_SIZE, MAX_CANDIDATES, out);
	check(count == 4, "discovery is independent of the ambient environment");
}

int main(void)
{
	test_override_wins();
	test_empty_override_ignored();
	test_discovery_order();
	test_usr_libexec_layout();
	test_relative_self_exe();
	test_unknown_self_exe();
	test_capacity_limits();
	test_bad_arguments();
	test_no_environment_dependence();

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
