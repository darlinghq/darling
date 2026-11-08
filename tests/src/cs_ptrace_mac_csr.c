// Copyright (C) 2026 Lubos Dolezel
//
// PT_DENY_ATTACH, PT_ATTACHEXC refused by darlingserver, __mac_syscall, csrctl
// and the hidden mseal helper. Run as root (`darling shell sudo ...`) for
// the attach check, which pushes the enforce policy through libsystem_kernel's
// Darling-private __darling_{get,set}_system_config and puts the previous
// configuration back. Exits by itself.
//
// - PT_DENY_ATTACH succeeds, also when repeated;
// - under enforce, PT_ATTACHEXC to a target that called PT_DENY_ATTACH fails
//   with EBUSY and the target is not stopped (attaching under the default
//   policy is not tried: it would start a debugging session);
// - QUIRK test_quirk_mac_syscall_unsupported_call: the Sandbox policy fails an
//   unsupported call with ENOTSUP and AMFI with ENOSYS; an unknown policy gives
//   ENOPOLICY, a NULL name EFAULT;
// - csr_get_active_config() returns darlingserver's csr-active-config, and
//   csr_check() succeeds exactly for the flags it contains (the kernel
//   debugger counting as allowed when untrusted kexts are);
// - libsystem_kernel exports no __darling_mseal_range.
// The attach target is an ad-hoc signed copy of this program (adhoc_sign.h),
// so that it may run under enforce.
#include <dlfcn.h>
#include <errno.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/csr.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <darlingserver/rpc-supplement.h>

#include "adhoc_sign.h"

extern int __mac_syscall(const char* policy, int call, void* arg);

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);

static get_fn get_config;
static set_fn set_config;
static int failures;
static dserver_system_config_t saved;
static int pushed;
static pid_t target;
static char target_dir[64];
static char target_path[128];

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// a configuration as read back, made into a push: the fields that were not
// pushed (their `present` bit is clear) and the generation must be zero
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
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_TRUST_STORE_PATH))
		memset(push.trust_store_path, 0, sizeof(push.trust_store_path));
	return push;
}

static void restore_policy(void)
{
	if (!pushed)
		return;
	pushed = 0;
	// the values pushed before the test (with nothing pushed: the defaults)
	dserver_system_config_t push = as_push(&saved);
	int err = set_config(&push);
	if (err != 0)
		printf("FAIL: could not put the system configuration back: %d (%s)\n", err, strerror(err));
}

static void finish(void)
{
	restore_policy();
	if (target > 0) {
		kill(target, SIGKILL);
		waitpid(target, NULL, 0);
	}
	if (target_path[0])
		unlink(target_path);
	if (target_dir[0])
		rmdir(target_dir);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	fflush(stdout);
	_exit(failures ? 1 : 0);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	failures++;
	finish();
}

// the target: this program again, so that it starts under the pushed policy
static void run_target(const char* fd)
{
	signal(SIGALRM, SIG_DFL);
	alarm(30);
	int ready = atoi(fd);
	char result = (ptrace(PT_DENY_ATTACH, 0, 0, 0) == 0) ? 'd' : 'e';
	write(ready, &result, 1);
	close(ready);
	for (;;)
		pause();
}

static void test_quirk_mac_syscall_unsupported_call(void)
{
	errno = 0;
	check(__mac_syscall("Sandbox", 9999, NULL) == -1 && errno == ENOTSUP, "QUIRK __mac_syscall Sandbox, unsupported call: ENOTSUP");
	errno = 0;
	check(__mac_syscall("AMFI", 9999, NULL) == -1 && errno == ENOSYS, "QUIRK __mac_syscall AMFI, unsupported call: ENOSYS");
}

static void test_csr(void)
{
	dserver_system_config_t config;
	csr_config_t active = 0;

	check(get_config(&config) == 0, "system configuration read");
	check(csr_get_active_config(&active) == 0, "csr_get_active_config succeeds");
	printf("  csr-active-config 0x%x (darlingserver 0x%x)\n", active, config.csr_active_config);
	check(active == config.csr_active_config, "csr_get_active_config returns the configured csr-active-config");

	csr_config_t effective = active;
	if (effective & (CSR_ALLOW_UNTRUSTED_KEXTS | CSR_ALLOW_APPLE_INTERNAL))
		effective |= CSR_ALLOW_KERNEL_DEBUGGER;
	int consistent = 1;
	for (int bit = 0; bit < 32; bit++) {
		csr_config_t mask = 1u << bit;
		if (!(CSR_VALID_FLAGS & mask))
			continue;
		int rc = csr_check(mask);
		int expected = (effective & mask) ? 0 : -1;
		if (rc != expected || (rc == -1 && errno != EPERM)) {
			printf("  csr_check(0x%x) = %d, expected %d\n", mask, rc, expected);
			consistent = 0;
		}
	}
	check(consistent, "csr_check succeeds exactly for the flags in csr-active-config (else EPERM)");
	check(csr_check(0) == 0, "csr_check(0) succeeds");
}

static void test_attach_refused(void)
{
	int ready[2];
	char self[1024];
	uint32_t self_size = sizeof(self);

	if (_NSGetExecutablePath(self, &self_size) != 0) {
		check(0, "own path");
		return;
	}

	// an unsigned program is not started under enforce
	strcpy(target_dir, "/tmp/cs-ptrace.XXXXXX");
	if (mkdtemp(target_dir) == NULL) {
		target_dir[0] = '\0';
		check(0, "temporary directory for the signed target");
		return;
	}
	snprintf(target_path, sizeof(target_path), "%s/target", target_dir);
	if (adhoc_sign(self, target_path, "org.darlinghq.test.cs-ptrace-mac-csr", 0, NULL) != 0) {
		target_path[0] = '\0';
		check(0, "ad-hoc signed copy of this program");
		return;
	}

	dserver_system_config_t push = as_push(&saved);
	push.code_signing_policy = dserver_code_signing_policy_enforce;
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY;
	int err = set_config(&push);
	check(err == 0, "enforce policy pushed");
	if (err != 0)
		return;
	pushed = 1;

	if (pipe(ready) != 0)
		return;
	fflush(stdout);
	target = fork();
	if (target == 0) {
		char fd[16];
		close(ready[0]);
		snprintf(fd, sizeof(fd), "%d", ready[1]);
		execl(target_path, target_path, "target", fd, (char*) NULL);
		_exit(1);
	}
	close(ready[1]);
	char result = 0;
	check(target > 0 && read(ready[0], &result, 1) == 1 && result == 'd', "target called PT_DENY_ATTACH under enforce");
	close(ready[0]);
	restore_policy();

	errno = 0;
	int rc = ptrace(PT_ATTACHEXC, target, 0, 0);
	printf("  PT_ATTACHEXC: %d, errno %d\n", rc, rc ? errno : 0);
	check(rc == -1 && errno == EBUSY, "PT_ATTACHEXC to a target that denied attachment, under enforce: EBUSY");

	usleep(200000);
	int status = 0;
	pid_t stopped = waitpid(target, &status, WNOHANG | WUNTRACED);
	check(stopped == 0, "the target was not stopped");
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "target") == 0)
		run_target(argv[2]);

	signal(SIGALRM, on_alarm);
	alarm(20);

	get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	check(get_config && set_config, "libsystem_kernel has the system configuration calls");
	if (!get_config || !set_config)
		finish();
	memset(&saved, 0, sizeof(saved));
	check(get_config(&saved) == 0, "saved the system configuration");

	check(ptrace(PT_DENY_ATTACH, 0, 0, 0) == 0, "PT_DENY_ATTACH succeeds");
	check(ptrace(PT_DENY_ATTACH, 0, 0, 0) == 0, "PT_DENY_ATTACH succeeds again");

	test_quirk_mac_syscall_unsupported_call();
	errno = 0;
	check(__mac_syscall("NoSuchPolicy", 0, NULL) == -1 && errno == ENOPOLICY, "__mac_syscall, unknown policy: ENOPOLICY");
	errno = 0;
	check(__mac_syscall(NULL, 0, NULL) == -1 && errno == EFAULT, "__mac_syscall, NULL policy: EFAULT");

	test_csr();

	check(dlsym(RTLD_DEFAULT, "__darling_mseal_range") == NULL && dlsym(RTLD_DEFAULT, "_darling_mseal_range") == NULL,
		"libsystem_kernel exports no __darling_mseal_range");

	if (getuid() == 0)
		test_attach_refused();
	else
		printf("SKIP: the attach check needs root\n");

	finish();
	return 0;
}
