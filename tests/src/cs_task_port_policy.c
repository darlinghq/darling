// Copyright (C) 2026 Lubos Dolezel
//
// task_for_pid() and its read, inspect and name variants under the code signing
// policy (darlingserver's task port hook), with the policy pushed through
// libsystem_kernel's Darling-private __darling_{get,set}_system_config. Must run
// as root (`darling shell sudo ...`); the policy is put back at the end.
//
// The targets are an ad-hoc signed copy of this program (adhoc_sign.h; an
// unsigned program is not started under enforce), executed again so that they
// start under the pushed policy. The signature has no entitlements and no
// hardened runtime, so the targets do not allow debugging (no
// CS_GET_TASK_ALLOW) and are not hardened.
//
// - under the policy in effect before the test (off by default) a process of
//   the same user gets every port of a target, as before;
// - under enforce, a process of the same user gets the name and inspect ports
//   of a target, but not its read or control ports (KERN_FAILURE);
// - a process of another user gets no port at all, not even the name port;
// - root gets the control port of a target that is not hardened;
// - a process always gets its own task port;
// - a target that started before enforce was pushed keeps its old policy.
#include <dlfcn.h>
#include <errno.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <darlingserver/rpc-supplement.h>

#include "adhoc_sign.h"

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);
typedef kern_return_t (*tfp_fn)(mach_port_t, int, mach_port_t*);

static get_fn get_config;
static set_fn set_config;
static tfp_fn read_for_pid;
static tfp_fn inspect_for_pid;

static int failures;
static dserver_system_config_t saved;
static int pushed;
static pid_t targets[3];
static uid_t nobody_uid;
static char target_dir[64];
static char target_path[128];

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// get_system_config reports the effective values, defaults included; a push may only carry the
// values whose `present` bit it sets, so the others are cleared here
static dserver_system_config_t pushable(const dserver_system_config_t* config)
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

	// the values in effect before the test (with nothing present: the defaults)
	dserver_system_config_t push = pushable(&saved);
	int err = set_config(&push);
	if (err != 0)
		printf("FAIL: could not put the system configuration back: %d (%s)\n", err, strerror(err));
}

static void finish(void)
{
	restore_policy();
	for (int i = 0; i < 3; i++) {
		if (targets[i] > 0) {
			kill(targets[i], SIGKILL);
			waitpid(targets[i], NULL, 0);
		}
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

// starts the signed copy of this program as a target, as `uid`; returns once it runs the new image
static pid_t start_target(uid_t uid)
{
	int ready[2];
	if (pipe(ready) != 0)
		return -1;

	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		char fd[16];
		close(ready[0]);
		if (uid != 0 && (setuid(uid) != 0 || getuid() != uid))
			_exit(1);
		snprintf(fd, sizeof(fd), "%d", ready[1]);
		execl(target_path, target_path, "target", fd, (char*) NULL);
		_exit(1);
	}
	close(ready[1]);

	char byte = 0;
	ssize_t got = (child > 0) ? read(ready[0], &byte, 1) : -1;
	close(ready[0]);
	return (got == 1) ? child : -1;
}

static void run_target(const char* fd)
{
	// never outlive the test
	signal(SIGALRM, SIG_DFL);
	alarm(30);
	int ready = atoi(fd);
	write(ready, "r", 1);
	close(ready);
	for (;;)
		pause();
}

struct ports {
	kern_return_t control, read, inspect, name;
};

static kern_return_t try_port(tfp_fn fn, pid_t pid)
{
	mach_port_t port = MACH_PORT_NULL;
	kern_return_t kr = fn(mach_task_self(), pid, &port);
	if (kr == KERN_SUCCESS && port != MACH_PORT_NULL)
		mach_port_deallocate(mach_task_self(), port);
	return kr;
}

static struct ports ports_of(pid_t pid)
{
	struct ports result;
	result.control = try_port(task_for_pid, pid);
	result.read = try_port(read_for_pid, pid);
	result.inspect = try_port(inspect_for_pid, pid);
	result.name = try_port(task_name_for_pid, pid);
	return result;
}

// asks for the ports of `pid` (0: of itself) from a child running as `uid`
static int ports_as(uid_t uid, pid_t pid, struct ports* out)
{
	int channel[2];
	if (pipe(channel) != 0)
		return -1;

	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		close(channel[0]);
		if (uid != 0 && (setuid(uid) != 0 || getuid() != uid))
			_exit(1);
		struct ports result = ports_of(pid > 0 ? pid : getpid());
		write(channel[1], &result, sizeof(result));
		_exit(0);
	}
	close(channel[1]);

	ssize_t got = (child > 0) ? read(channel[0], out, sizeof(*out)) : -1;
	close(channel[0]);
	if (child > 0)
		waitpid(child, NULL, 0);
	return (got == (ssize_t) sizeof(*out)) ? 0 : -1;
}

static void print_ports(const char* who, const struct ports* p)
{
	printf("  %s: control %d, read %d, inspect %d, name %d\n", who, p->control, p->read, p->inspect, p->name);
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "target") == 0)
		run_target(argv[2]);

	signal(SIGALRM, on_alarm);
	alarm(20);

	get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	read_for_pid = (tfp_fn) dlsym(RTLD_DEFAULT, "task_read_for_pid");
	inspect_for_pid = (tfp_fn) dlsym(RTLD_DEFAULT, "task_inspect_for_pid");
	check(get_config && set_config, "libsystem_kernel has the system configuration calls");
	check(read_for_pid && inspect_for_pid, "libsystem_kernel has task_read_for_pid and task_inspect_for_pid");
	if (!get_config || !set_config || !read_for_pid || !inspect_for_pid)
		finish();

	check(getuid() == 0, "running as root");
	// macOS's "nobody" is uid -2; use it when the user database has no entry
	struct passwd* nobody = getpwnam("nobody");
	nobody_uid = nobody ? nobody->pw_uid : (uid_t)-2;
	// the uid reaches darlingserver as a signed 32-bit value, and a negative one (such as -2)
	// is taken there for "unchanged"; the pairings then need another unprivileged user
	if ((int32_t) nobody_uid < 0) {
		struct passwd* daemon_user = getpwnam("daemon");
		nobody_uid = daemon_user ? daemon_user->pw_uid : 1;
		printf("  nobody's uid does not reach darlingserver; using uid %u instead\n", (unsigned) nobody_uid);
	}
	check(nobody_uid != 0, "the \"nobody\" uid is not root");
	if (getuid() != 0 || nobody_uid == 0)
		finish();

	check(get_config(&saved) == 0, "get_system_config succeeds");
	printf("  policy before the test: %u (generation %llu)\n", saved.code_signing_policy, (unsigned long long) saved.generation);
	if (saved.code_signing_policy == dserver_code_signing_policy_enforce) {
		printf("  enforce is already in effect; the \"before\" checks are skipped\n");
	}

	// the signed target, readable and executable by the unprivileged user
	char self[1024];
	uint32_t self_size = sizeof(self);
	strcpy(target_dir, "/tmp/cs-task-port.XXXXXX");
	if (_NSGetExecutablePath(self, &self_size) != 0 || mkdtemp(target_dir) == NULL) {
		target_dir[0] = '\0';
		check(0, "own path and a temporary directory");
		finish();
	}
	snprintf(target_path, sizeof(target_path), "%s/target", target_dir);
	chmod(target_dir, 0755);
	if (adhoc_sign(self, target_path, "org.darlinghq.test.cs-task-port-policy", 0, NULL) != 0) {
		target_path[0] = '\0';
		check(0, "ad-hoc signed copy of this program");
		finish();
	}

	struct ports p;

	// before: the old policy
	targets[0] = start_target(nobody_uid);
	check(targets[0] > 0, "start a target as nobody under the old policy");
	if (saved.code_signing_policy != dserver_code_signing_policy_enforce) {
		check(ports_as(nobody_uid, targets[0], &p) == 0, "ask for its ports as nobody");
		print_ports("nobody -> nobody target, old policy", &p);
		check(p.control == KERN_SUCCESS && p.read == KERN_SUCCESS && p.inspect == KERN_SUCCESS && p.name == KERN_SUCCESS,
			"old policy: the same user gets every port");
	}

	// enforce
	dserver_system_config_t push = pushable(&saved);
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY;
	push.code_signing_policy = dserver_code_signing_policy_enforce;
	int err = set_config(&push);
	check(err == 0, "push code signing policy enforce");
	if (err != 0) {
		printf("  error %d (%s)\n", err, strerror(err));
		finish();
	}
	pushed = 1;

	targets[1] = start_target(nobody_uid);
	targets[2] = start_target(0);
	check(targets[1] > 0 && targets[2] > 0, "start targets as nobody and root under enforce");
	if (targets[1] <= 0 || targets[2] <= 0)
		finish();

	check(ports_as(nobody_uid, targets[1], &p) == 0, "ask for the nobody target's ports as nobody");
	print_ports("nobody -> nobody target, enforce", &p);
	check(p.name == KERN_SUCCESS && p.inspect == KERN_SUCCESS, "enforce: the same user gets the name and inspect ports");
	// a refusal is KERN_FAILURE from the Mach traps (task_for_pid, task_name_for_pid), but -1
	// from the BSD system calls (task_read_for_pid, task_inspect_for_pid), as on macOS
	check(p.control != KERN_SUCCESS && p.read != KERN_SUCCESS, "enforce: the same user gets no control or read port without get-task-allow");

	check(ports_as(nobody_uid, targets[2], &p) == 0, "ask for the root target's ports as nobody");
	print_ports("nobody -> root target, enforce", &p);
	check(p.name != KERN_SUCCESS && p.inspect != KERN_SUCCESS && p.read != KERN_SUCCESS && p.control != KERN_SUCCESS,
		"enforce: another user gets no port at all");

	p = ports_of(targets[1]);
	print_ports("root -> nobody target, enforce", &p);
	check(p.control == KERN_SUCCESS && p.read == KERN_SUCCESS && p.name == KERN_SUCCESS, "enforce: root gets the ports of a target that is not hardened");

	check(ports_as(nobody_uid, 0, &p) == 0, "ask for its own ports as nobody");
	print_ports("nobody -> itself, enforce", &p);
	check(p.control == KERN_SUCCESS && p.read == KERN_SUCCESS, "a process always gets its own task port");

	check(ports_as(nobody_uid, targets[0], &p) == 0, "ask for the old target's ports as nobody");
	print_ports("nobody -> nobody target started before enforce", &p);
	if (saved.code_signing_policy != dserver_code_signing_policy_enforce) {
		check(p.control == KERN_SUCCESS, "a target started before enforce keeps its policy");
	}

	finish();
}
