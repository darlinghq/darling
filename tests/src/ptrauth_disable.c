// Copyright (C) 2026 Lubos Dolezel
//
// DisablePointerAuthentication (org.darlinghq.security): with it on, mldr
// turns off arm64 pointer authentication for the processes it starts, so a
// PAC instruction leaves the pointer unchanged; with it off (the default),
// signing a pointer changes it.
//
// Usage: ptrauth_disable
// Run as root (the default in `darling shell`): it pushes the setting through
// libsystem_kernel's Darling-private __darling_{get,set}_system_config, starts
// a copy of itself, and puts the previous configuration back.
// On an arm64 host whose kernel does not support pointer authentication,
// signing never changes a pointer and the test skips. On other architectures
// it only checks that the setting is accepted.
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <darlingserver/rpc-supplement.h>

extern char** environ;

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// 1 if signing a pointer with the IA key changes it, 0 if not
static int signing_changes_pointer(void)
{
#if defined(__arm64__) || defined(__aarch64__)
	register uint64_t x17 __asm__("x17") = 0x1000;
	register uint64_t x16 __asm__("x16") = 42;
	// PACIA1716, in the hint space: a no-op where pointer authentication is off
	__asm__ volatile("hint #8" : "+r"(x17) : "r"(x16));
	return x17 != 0x1000;
#else
	return 0;
#endif
}

// a configuration as read back, made into a push
static dserver_system_config_t as_push(const dserver_system_config_t* config)
{
	dserver_system_config_t push = *config;
	push.generation = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY))
		push.code_signing_policy = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_CSR_ACTIVE_CONFIG))
		push.csr_active_config = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES))
		push.seal_images = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_DISABLE_POINTER_AUTHENTICATION))
		push.disable_pointer_authentication = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_TRUST_STORE_PATH))
		memset(push.trust_store_path, 0, sizeof(push.trust_store_path));
	return push;
}

// starts a copy of this program, which exits with signing_changes_pointer()
static int child_signing_changes_pointer(const char* self)
{
	char* argv[] = { (char*) self, "child", NULL };
	pid_t pid;
	int status;

	if (posix_spawn(&pid, self, NULL, NULL, argv, environ) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "child") == 0)
		return signing_changes_pointer();

	get_fn get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_fn set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	check(get_config && set_config, "libsystem_kernel has the system configuration calls");
	if (!get_config || !set_config)
		return 1;

	dserver_system_config_t saved;
	check(get_config(&saved) == 0, "read the system configuration");

	char self[1024];
	uint32_t self_size = sizeof(self);
	check(_NSGetExecutablePath(self, &self_size) == 0, "own path");

	dserver_system_config_t push = as_push(&saved);
	push.disable_pointer_authentication = 0;
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_DISABLE_POINTER_AUTHENTICATION;
	check(set_config(&push) == 0, "DisablePointerAuthentication off pushed");
	int before = child_signing_changes_pointer(self);

	push.disable_pointer_authentication = 1;
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_DISABLE_POINTER_AUTHENTICATION;
	int err = set_config(&push);
	check(err == 0, "DisablePointerAuthentication pushed");

	dserver_system_config_t current;
	check(get_config(&current) == 0 && current.disable_pointer_authentication == 1, "DisablePointerAuthentication read back");

	int after = child_signing_changes_pointer(self);

	push = as_push(&saved);
	if (set_config(&push) != 0)
		check(0, "put the system configuration back");

#if defined(__arm64__) || defined(__aarch64__)
	if (before == 1) {
		check(after == 0, "with the setting on, signing leaves a pointer unchanged");
	} else {
		check(before == 0, "the copy ran");
		printf("SKIP: pointer authentication is not available on this host\n");
	}
#else
	check(before == 0 && after == 0, "the copies ran");
#endif

	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures ? 1 : 0;
}
