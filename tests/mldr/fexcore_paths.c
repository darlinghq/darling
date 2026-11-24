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

size_t fexcore_not_found_message(char* buf, size_t buf_size, const char* override_path);

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
	check(count > 3 && strcmp(out[3], "libdarling-fexcore.so") == 0,
		"the dynamic loader's search path is the last resort, under the shim's own name");

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
	check(count > 1 && strcmp(out[1], "libdarling-fexcore.so") == 0,
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

// A stock FEX libFEXCore.so has no darling_fexcore_run_thread, so offering it would
// only produce a misleading "symbol not found" after a successful dlopen.
static void test_stock_fexcore_never_a_candidate(void)
{
	const char* out[MAX_CANDIDATES];
	size_t count;
	int stock = 0;

	unsetenv("DARLING_FEXCORE_LIB");
	count = discover("/opt/darling/libexec/darling/bin/mldr", BUFFER_SIZE, MAX_CANDIDATES, out);
	for (size_t i = 0; i < count; i++) {
		const char* name = strrchr(out[i], '/');
		name = name ? name + 1 : out[i];
		if (strcmp(name, "libFEXCore.so") == 0)
			stock = 1;
	}
	check(count > 0 && !stock, "libFEXCore.so is not among the candidates");
}

static void test_not_found_message(void)
{
	char buf[2048];
	size_t len;

	len = fexcore_not_found_message(buf, sizeof(buf), NULL);
	check(len > 0 && len == strlen(buf), "the not-found message reports its length");
	check(strstr(buf, "no usable libdarling-fexcore.so") != NULL,
		"the headline covers a shim that was found but failed to load");
	check(strstr(buf, "found but unusable") != NULL, "the message explains the per-candidate tag");
	check(strstr(buf, "-DWITH_FEXCORE=ON") != NULL, "the message says how to build the shim");
	check(strstr(buf, "DARLING_FEXCORE_LIB") != NULL, "the message names the override variable");
	check(strstr(buf, "Darling package") != NULL, "the message points at installing a package");
	check(strstr(buf, "darling_fexcore_run_thread") != NULL && strstr(buf, "stock FEX") != NULL,
		"the message says a stock libFEXCore.so is not enough");
	check(len > 0 && buf[len - 1] == '\n', "the message ends with a newline");

	len = fexcore_not_found_message(buf, sizeof(buf), "");
	check(strstr(buf, "no usable") != NULL, "an empty override is treated as unset");

	len = fexcore_not_found_message(buf, sizeof(buf), "/opt/x/libdarling-fexcore.so");
	check(strstr(buf, "DARLING_FEXCORE_LIB=/opt/x/libdarling-fexcore.so") != NULL,
		"a failing override is echoed back");
	check(strstr(buf, "unset DARLING_FEXCORE_LIB") != NULL, "the message offers unsetting a bad override");
	check(len > 0 && len == strlen(buf), "the override message reports its length");

	len = fexcore_not_found_message(buf, 16, NULL);
	check(len == 15 && strlen(buf) == 15, "a small buffer is truncated, not overrun");
	check(fexcore_not_found_message(NULL, 16, NULL) == 0, "a null buffer yields nothing");
	check(fexcore_not_found_message(buf, 0, NULL) == 0, "a zero-size buffer yields nothing");
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
	test_stock_fexcore_never_a_candidate();
	test_not_found_message();

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
