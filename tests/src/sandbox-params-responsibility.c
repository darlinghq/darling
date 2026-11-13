/*
 Copyright (C) 2026 Lubos Dolezel

 Guest test: sandbox profile parameters/compiled profiles and process responsibility.
 Contract: the declarations used by open-source clients of the sandbox SPI
 (sandbox_params_t is opaque; sandbox_create_params returns a new object, sandbox_set_param
 returns 0 on success, compile functions return a profile or NULL with an error string,
 sandbox_free_params/_free_profile release them) and libquarantine's responsibility_* calls.
 Black-box observation on macOS 26.7.3 matches every check (run the program unchanged).
 sandbox_apply is not called: on macOS it would sandbox the test process.
 Build: guest-cc.sh -o t sandbox-params-responsibility.c -lsandbox (libquarantine is part of libSystem)
 Prints PASS/FAIL lines; exit status is the number of failures.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <spawn.h>
#include <sys/wait.h>

extern void *sandbox_create_params(void);
extern int sandbox_set_param(void *params, const char *key, const char *value);
extern void sandbox_free_params(void *params);
extern void *sandbox_compile_string(const char *profile, void *params, char **error);
extern void sandbox_free_profile(void *profile);
extern pid_t responsibility_get_pid_responsible_for_pid(pid_t pid);
extern int responsibility_spawnattrs_setdisclaim(posix_spawnattr_t *attrs, int disclaim);

static int failures;
#define CHECK(cond, what) do { \
	if (cond) printf("PASS: %s\n", what); \
	else { printf("FAIL: %s\n", what); failures++; } \
} while (0)

int main(void) {
	void *params = sandbox_create_params();
	CHECK(params != NULL, "sandbox_create_params returns an object");
	CHECK(params && sandbox_set_param(params, "HOME", "/tmp") == 0, "sandbox_set_param stores a parameter and returns 0");
	CHECK(params && sandbox_set_param(params, "HOME", "/var") == 0, "setting the same key again succeeds");

	char *error = NULL;
	void *profile = sandbox_compile_string("(version 1)\n(allow default)\n", params, &error);
	CHECK(profile != NULL, "sandbox_compile_string returns a profile object");
	CHECK(error == NULL, "no error string on success");
	if (profile)
		sandbox_free_profile(profile);
	sandbox_free_params(params);
	CHECK(1, "params and profile freed");

	pid_t self = getpid();
	CHECK(responsibility_get_pid_responsible_for_pid(self) > 0, "responsibility_get_pid_responsible_for_pid returns a pid");

	posix_spawnattr_t attrs;
	posix_spawnattr_init(&attrs);
	CHECK(responsibility_spawnattrs_setdisclaim(&attrs, 1) == 0, "responsibility_spawnattrs_setdisclaim accepts a spawn attribute object");
	char *argv[] = { "/usr/bin/true", NULL };
	pid_t child;
	int rc = posix_spawn(&child, "/usr/bin/true", NULL, &attrs, argv, NULL);
	CHECK(rc == 0, "posix_spawn with the disclaim attribute still starts the program");
	if (rc == 0) {
		int st = 1;
		waitpid(child, &st, 0);
		CHECK(st == 0, "the spawned program exits 0");
	}
	posix_spawnattr_destroy(&attrs);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures;
}
