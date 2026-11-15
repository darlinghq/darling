// Copyright (C) 2026 Lubos Dolezel
//
// sandbox_extension_issue_file_to_self: exported by libSystem, issues a
// free()able file extension token for the calling process (the sibling of
// sandbox_extension_issue_file with the caller as grantee), NULL with EINVAL
// for a NULL class or path, and the token it returns can be consumed and
// released like any other. Observed on macOS 26.7.3 (x86_64); the same
// program runs there.
//
// Build for Darling (x86_64 or i386) or macOS and run without arguments.
// Prints PASS and exits 0 on success.
#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char *(*issue_fn)(const char *, const char *, uint32_t);
typedef int64_t (*consume_fn)(const char *);
typedef int (*release_fn)(int64_t);

static int g_failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); g_failures++; } } while (0)

int main(void)
{
	issue_fn issue = (issue_fn) dlsym(RTLD_DEFAULT, "sandbox_extension_issue_file_to_self");
	consume_fn consume = (consume_fn) dlsym(RTLD_DEFAULT, "sandbox_extension_consume");
	release_fn release = (release_fn) dlsym(RTLD_DEFAULT, "sandbox_extension_release");
	CHECK(issue != NULL);
	CHECK(consume != NULL);
	CHECK(release != NULL);
	if (!issue || !consume || !release) {
		puts("FAIL");
		return 1;
	}

	char *token = issue("com.apple.app-sandbox.read", "/private/tmp", 0);
	CHECK(token != NULL);
	CHECK(token == NULL || strlen(token) > 0);

	// A different path gives a different token
	char *other = issue("com.apple.app-sandbox.read", "/usr", 0);
	CHECK(other != NULL && token != NULL && strcmp(token, other) != 0);

	errno = 0;
	char *bad = issue(NULL, "/private/tmp", 0);
	CHECK(bad == NULL && errno == EINVAL);
	errno = 0;
	bad = issue("com.apple.app-sandbox.read", NULL, 0);
	CHECK(bad == NULL && errno == EINVAL);

	if (token) {
		int64_t handle = consume(token);
		CHECK(handle >= 0);
		CHECK(release(handle) == 0);
	}

	free(token);
	free(other);
	if (g_failures == 0)
		puts("PASS");
	return g_failures != 0;
}
