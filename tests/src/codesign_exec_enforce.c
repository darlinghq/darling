// Copyright (C) 2026 Lubos Dolezel
//
// mldr and Darling's code signing configuration: refusing programs whose
// signature is missing or invalid, mapping the program from a snapshot, and
// sealing the program's read-only segments.
//
// Usage: codesign_exec_enforce
// Run as root (the default in `darling shell`): every part pushes a
// configuration through libsystem_kernel's Darling-private
// __darling_{get,set}_system_config and puts the previous one back (also when
// the alarm fires). Exits by itself.
//
// The program writes copies of itself into a temporary directory:
//   unsigned   as built (the in-tree linker does not sign)
//   plain      ad-hoc signed, no hardened runtime
//   tampered   `plain` with one byte of its __TEXT changed after signing
//   rt         ad-hoc signed, hardened runtime, disable-library-validation
//   rt_gta     the same with get-task-allow
// (signed with adhoc_sign.h; disable-library-validation keeps Darling's
// unsigned libraries loadable under enforce). Each copy is started with posix_spawn and reports:
//   - whether its own __TEXT can still be made writable (not sealed);
//   - whether a change it writes into its own file shows in its mapped
//     __TEXT (not mapped from a snapshot).
//
// Expected:
// - default configuration: every copy runs, mldr prints nothing, nothing is
//   sealed, file changes show (no snapshot);
// - CodeSigningPolicy warn: every copy runs; mldr names `unsigned` on stderr
//   (an invalid signature, `tampered`, is reported in darlingserver's log);
// - CodeSigningPolicy enforce: `unsigned` and `tampered` are killed with
//   SIGKILL before they run, with mldr's reason on stderr, as macOS kills a
//   program whose signature is rejected; the signed copies run, and their
//   file changes no longer show (snapshot);
// - SealImages on: only `rt` (hardened, no get-task-allow) has its __TEXT
//   sealed, and only in 64-bit processes (Linux has no mseal() for 32-bit
//   ones).
// When file changes do not show even without a snapshot (default
// configuration), the snapshot check is skipped; the sealing check is skipped
// when the host kernel has no mseal().
#include <CommonCrypto/CommonDigest.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <kern/cs_blobs.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/dyld.h>
#include <mach-o/ldsyms.h>
#include <mach-o/loader.h>
#include <signal.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/codesign.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <darlingserver/rpc-supplement.h>

#include "adhoc_sign.h"

#define SIGN_ID "org.darlinghq.test.codesign-exec-enforce"
#define ENT_DISABLE_LV "com.apple.security.cs.disable-library-validation"
#define ENT_GET_TASK_ALLOW "com.apple.security.get-task-allow"

// in __TEXT; a child changes it in its own file and looks for the change in memory
static const char marker[] = "codesign-exec-enforce marker: original";
#define MARKER_CHANGED_AT (sizeof(marker) - 9) // "original" -> "Original"

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);

static get_fn get_config;
static set_fn set_config;
static dserver_system_config_t saved;
static int pushed;
static int failures;
static char tmp_dir[64];

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

static int push_config(uint32_t policy, uint32_t seal)
{
	dserver_system_config_t push = as_push(&saved);
	push.code_signing_policy = policy;
	push.seal_images = seal;
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY | DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES;
	int err = set_config(&push);
	if (err == 0)
		pushed = 1;
	return err;
}

static void restore_config(void)
{
	if (!pushed)
		return;
	pushed = 0;
	dserver_system_config_t push = as_push(&saved);
	int err = set_config(&push);
	if (err != 0)
		printf("FAIL: could not put the system configuration back: %d (%s)\n", err, strerror(err));
}

static const char* const copy_names[] = { "unsigned", "plain", "tampered", "rt", "rt_gta" };

static void remove_tmp(void)
{
	char path[256];

	if (!tmp_dir[0])
		return;
	for (size_t i = 0; i < sizeof(copy_names) / sizeof(copy_names[0]); i++) {
		snprintf(path, sizeof(path), "%s/%s", tmp_dir, copy_names[i]);
		unlink(path);
	}
	rmdir(tmp_dir);
}

static void finish(void)
{
	restore_config();
	remove_tmp();
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

//
// the started copies
//

static int find_marker(const char* path, off_t* offset)
{
	int fd = open(path, O_RDONLY);
	struct stat st;
	int found = 0;

	if (fd < 0)
		return 0;
	if (fstat(fd, &st) == 0 && st.st_size > 0) {
		char* data = malloc((size_t)st.st_size);
		if (data && pread(fd, data, (size_t)st.st_size, 0) == st.st_size) {
			for (off_t i = 0; i + (off_t)sizeof(marker) <= st.st_size; i++) {
				if (memcmp(data + i, marker, sizeof(marker)) == 0) {
					*offset = i;
					found = 1;
					break;
				}
			}
		}
		free(data);
	}
	close(fd);
	return found;
}

// Runs in each copy. Prints one RESULT line.
static int child(const char* self)
{
	long page = sysconf(_SC_PAGESIZE);

	// does a change of the file show in the mapped __TEXT? (-1: not tested)
	int followed = -1;
	off_t offset;
	if (find_marker(self, &offset)) {
		int fd = open(self, O_RDWR);
		char original = marker[MARKER_CHANGED_AT];
		char changed = original ^ 0x20;
		if (fd >= 0 && pwrite(fd, &changed, 1, offset + (off_t)MARKER_CHANGED_AT) == 1) {
			followed = (((const volatile char*)marker)[MARKER_CHANGED_AT] == changed);
			// the copy is started again later; its signature must still match
			pwrite(fd, &original, 1, offset + (off_t)MARKER_CHANGED_AT);
		}
		if (fd >= 0)
			close(fd);
	}

	// can the first page of __TEXT be made writable? (-1: other error)
	int sealed;
	void* text = (void*)((uintptr_t)&_mh_execute_header & ~(uintptr_t)(page - 1));
	if (mprotect(text, (size_t)page, PROT_READ | PROT_WRITE) == 0) {
		sealed = 0;
		mprotect(text, (size_t)page, PROT_READ | PROT_EXEC);
	} else {
		sealed = (errno == EPERM) ? 1 : -1;
	}

	printf("RESULT sealed=%d followed=%d\n", sealed, followed);
	fflush(stdout);
	return 0;
}

struct result {
	int ran;           // exited with 0 after printing a RESULT line
	int killed;        // killed by SIGKILL
	int sealed, followed;
	int no_mseal;      // dyld said the host kernel has no mseal()
	char errors[4096]; // what it wrote to stderr
};

static struct result run_copy(const char* name)
{
	struct result r;
	memset(&r, 0, sizeof(r));
	r.sealed = r.followed = -2;

	char path[128];
	snprintf(path, sizeof(path), "%s/%s", tmp_dir, name);
	char* argv[] = { path, "child", NULL };
	char* envp[] = { "PATH=/usr/bin:/bin", NULL };

	int out[2], err[2];
	if (pipe(out) != 0 || pipe(err) != 0)
		return r;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, out[1], 1);
	posix_spawn_file_actions_adddup2(&actions, err[1], 2);
	posix_spawn_file_actions_addclose(&actions, out[0]);
	posix_spawn_file_actions_addclose(&actions, err[0]);
	pid_t pid;
	int rc = posix_spawn(&pid, path, &actions, NULL, argv, envp);
	posix_spawn_file_actions_destroy(&actions);
	close(out[1]);
	close(err[1]);
	if (rc != 0) {
		close(out[0]);
		close(err[0]);
		printf("  %s: posix_spawn failed: %d (%s)\n", name, rc, strerror(rc));
		return r;
	}

	char text[4096] = "";
	size_t used = 0, err_used = 0;
	ssize_t n;
	while ((n = read(out[0], text + used, sizeof(text) - 1 - used)) > 0)
		used += (size_t)n;
	while ((n = read(err[0], r.errors + err_used, sizeof(r.errors) - 1 - err_used)) > 0)
		err_used += (size_t)n;
	text[used] = r.errors[err_used] = '\0';
	close(out[0]);
	close(err[0]);
	int status = 0;
	waitpid(pid, &status, 0);

	const char* line = strstr(text, "RESULT ");
	r.ran = line && sscanf(line, "RESULT sealed=%d followed=%d", &r.sealed, &r.followed) == 2 &&
		WIFEXITED(status) && WEXITSTATUS(status) == 0;
	r.killed = WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL;
	r.no_mseal = strstr(r.errors, "does not support mseal") != NULL;
	if (WIFSIGNALED(status))
		printf("  %s: killed by signal %d\n", name, WTERMSIG(status));
	else
		printf("  %s: exit %d, %s", name, WEXITSTATUS(status), line ? line : "(no result)\n");
	if (err_used)
		printf("  %s stderr: %s%s", name, r.errors, r.errors[err_used - 1] == '\n' ? "" : "\n");
	return r;
}

static int mldr_said(const struct result* r, const char* what)
{
	return strstr(r->errors, "mldr: ") != NULL && (what == NULL || strstr(r->errors, what) != NULL);
}

// the copy ran and mldr said nothing (or only what `allowed` names)
static void expect_ran(const char* scenario, const char* name, const struct result* r, const char* allowed)
{
	char what[256];
	snprintf(what, sizeof(what), "%s, %s: ran", scenario, name);
	check(r->ran, what);
	if (!allowed) {
		snprintf(what, sizeof(what), "%s, %s: no message from mldr", scenario, name);
		check(!mldr_said(r, NULL), what);
	}
}

static void expect_refused(const char* scenario, const char* name, const struct result* r, const char* reason)
{
	char what[256];
	snprintf(what, sizeof(what), "%s, %s: killed with SIGKILL before it ran", scenario, name);
	check(r->killed && !r->ran, what);
	snprintf(what, sizeof(what), "%s, %s: mldr says its code signature %s", scenario, name, reason);
	check(mldr_said(r, "refusing to run it") && mldr_said(r, reason), what);
}

static void expect_sealed(const char* scenario, const char* name, const struct result* r, int want)
{
	char what[256];
	if (!r->ran)
		return;
	if (want && r->no_mseal) {
		printf("SKIP: %s, %s: the host kernel has no mseal()\n", scenario, name);
		return;
	}
	snprintf(what, sizeof(what), "%s, %s: __TEXT %s", scenario, name, want ? "sealed (mprotect EPERM)" : "not sealed");
	check(r->sealed == want, what);
}

// changes the byte of the marker in the file `path` (copied signed file -> tampered)
static int tamper(const char* path)
{
	off_t offset;
	if (!find_marker(path, &offset))
		return -1;
	int fd = open(path, O_RDWR);
	char changed = marker[MARKER_CHANGED_AT] ^ 0x20;
	int ok = fd >= 0 && pwrite(fd, &changed, 1, offset + (off_t)MARKER_CHANGED_AT) == 1;
	if (fd >= 0)
		close(fd);
	return ok ? 0 : -1;
}

static int copy_file(const char* source, const char* target)
{
	char chunk[16384];
	ssize_t n;
	int in = open(source, O_RDONLY), out = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0755), ok = in >= 0 && out >= 0;
	while (ok && (n = read(in, chunk, sizeof(chunk))) > 0)
		ok = write(out, chunk, (size_t)n) == n;
	if (in >= 0)
		close(in);
	if (out >= 0)
		close(out);
	return ok ? 0 : -1;
}

int main(int argc, char** argv)
{
	if (argc == 2 && strcmp(argv[1], "child") == 0) {
		alarm(20);
		return child(argv[0]);
	}

	signal(SIGALRM, on_alarm);
	alarm(90);
	setvbuf(stdout, NULL, _IOLBF, 0);

	if (getuid() != 0) {
		printf("SKIP: pushing a code signing configuration needs root\n");
		return 0;
	}

	get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	check(get_config && set_config, "libsystem_kernel has the system configuration calls");
	if (!get_config || !set_config)
		finish();
	check(get_config(&saved) == 0, "read the system configuration");

	char self[1024];
	uint32_t self_size = sizeof(self);
	check(_NSGetExecutablePath(self, &self_size) == 0, "own path");
	off_t offset;
	check(find_marker(self, &offset), "the marker is in the file");

	strcpy(tmp_dir, "/tmp/codesign-enforce.XXXXXX");
	check(mkdtemp(tmp_dir) != NULL, "temporary directory");

	static const char* const ents_rt[] = { ENT_DISABLE_LV, NULL };
	static const char* const ents_gta[] = { ENT_DISABLE_LV, ENT_GET_TASK_ALLOW, NULL };
	char path[128];
	snprintf(path, sizeof(path), "%s/unsigned", tmp_dir);
	check(copy_file(self, path) == 0, "unsigned copy");
	snprintf(path, sizeof(path), "%s/plain", tmp_dir);
	check(adhoc_sign(self, path, SIGN_ID, 0, NULL) == 0, "signed copy plain");
	snprintf(path, sizeof(path), "%s/tampered", tmp_dir);
	check(adhoc_sign(self, path, SIGN_ID, 0, NULL) == 0 && tamper(path) == 0, "signed copy tampered, then changed");
	snprintf(path, sizeof(path), "%s/rt", tmp_dir);
	check(adhoc_sign(self, path, SIGN_ID, 1, ents_rt) == 0, "signed copy rt");
	snprintf(path, sizeof(path), "%s/rt_gta", tmp_dir);
	check(adhoc_sign(self, path, SIGN_ID, 1, ents_gta) == 0, "signed copy rt_gta");
	if (failures)
		finish();

	struct result unsigned_copy, plain, tampered, rt, rt_gta;
	int baseline_follows = 0;

	// 1. the default configuration
	check(push_config(dserver_code_signing_policy_off, 0) == 0, "default configuration pushed");
	unsigned_copy = run_copy("unsigned");
	plain = run_copy("plain");
	tampered = run_copy("tampered");
	rt = run_copy("rt");
	rt_gta = run_copy("rt_gta");
	restore_config();
	expect_ran("default", "unsigned", &unsigned_copy, NULL);
	expect_ran("default", "plain", &plain, NULL);
	expect_ran("default", "tampered", &tampered, NULL);
	expect_ran("default", "rt", &rt, NULL);
	expect_ran("default", "rt_gta", &rt_gta, NULL);
	expect_sealed("default", "rt", &rt, 0);
	baseline_follows = plain.ran && plain.followed == 1;
	if (!baseline_follows)
		printf("SKIP: snapshot checks: a change of the file does not show in the mapping even without a snapshot\n");

	// 2. CodeSigningPolicy warn
	int err = push_config(dserver_code_signing_policy_warn, 0);
	check(err == 0, "warn pushed");
	if (err == 0) {
		unsigned_copy = run_copy("unsigned");
		plain = run_copy("plain");
		tampered = run_copy("tampered");
		restore_config();
		expect_ran("warn", "unsigned", &unsigned_copy, "warning");
		check(mldr_said(&unsigned_copy, "warning") && mldr_said(&unsigned_copy, "is missing"),
			"warn, unsigned: mldr warns that its code signature is missing");
		expect_ran("warn", "plain", &plain, NULL);
		// an invalid signature is reported in darlingserver's log, not by mldr
		expect_ran("warn", "tampered", &tampered, "warning");
	}

	// 3. CodeSigningPolicy enforce
	err = push_config(dserver_code_signing_policy_enforce, 0);
	check(err == 0, "enforce pushed");
	if (err == 0) {
		unsigned_copy = run_copy("unsigned");
		plain = run_copy("plain");
		tampered = run_copy("tampered");
		rt = run_copy("rt");
		rt_gta = run_copy("rt_gta");
		restore_config();
		expect_refused("enforce", "unsigned", &unsigned_copy, "is missing");
		expect_refused("enforce", "tampered", &tampered, "is not valid");
		expect_ran("enforce", "plain", &plain, NULL);
		expect_ran("enforce", "rt", &rt, NULL);
		expect_ran("enforce", "rt_gta", &rt_gta, NULL);
		if (baseline_follows && plain.ran)
			check(plain.followed == 0, "enforce, plain: mapped from a snapshot (a change of the file does not show)");
		expect_sealed("enforce", "rt", &rt, 0);
	}

	// 4. SealImages
	err = push_config(dserver_code_signing_policy_off, 1);
	check(err == 0, "SealImages pushed");
	if (err == 0) {
		plain = run_copy("plain");
		rt = run_copy("rt");
		rt_gta = run_copy("rt_gta");
		restore_config();
		expect_ran("SealImages", "plain", &plain, NULL);
		expect_ran("SealImages", "rt", &rt, NULL);
		expect_ran("SealImages", "rt_gta", &rt_gta, NULL);
		expect_sealed("SealImages", "plain", &plain, 0);
#ifdef __i386__
		printf("SKIP: SealImages, rt: Linux has no mseal() for 32-bit processes\n");
		expect_sealed("SealImages", "rt", &rt, 0);
#else
		expect_sealed("SealImages", "rt", &rt, 1);
		if (rt.ran && rt.sealed == 1)
			check(rt.followed != 1, "SealImages, rt: a change of the file does not show in sealed __TEXT");
#endif
		expect_sealed("SealImages", "rt_gta", &rt_gta, 0);
	}

	finish();
	return 0;
}
