// Copyright (C) 2026 Lubos Dolezel
//
// The sandbox state that sandbox_check(pid, NULL, SANDBOX_FILTER_NONE)
// reports (0 before a profile is applied, 1 after, inherited by fork, kept
// across exec, visible to other processes), and the extension token round
// trip in a process that is and one that is not sandboxed. Expected values
// were observed on macOS 27.0.1 (arm64); the same program runs there.
// Darling enforces no profile, so with an operation every check is 0.
//
// Build for macOS or Darling (x86_64 or i386) and run without arguments.
// Prints PASS and exits 0 on success.
#define __APPLE_API_PRIVATE
#include <sandbox.h>
#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// The SDK's <sandbox.h> may lack the SPI; these are the prototypes clients use
// (WebKit's SandboxSPI.h)
int sandbox_check(pid_t pid, const char *operation, enum sandbox_filter_type type, ...);
char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags);
char *sandbox_extension_issue_mach(const char *extension_class, const char *name, uint32_t flags);
int64_t sandbox_extension_consume(const char *extension_token);
int sandbox_extension_release(int64_t extension_handle);

#define PERMISSIVE_PROFILE "(version 1)(allow default)"
#define READ_CLASS "com.apple.app-sandbox.read"
#define ERRNO_SENTINEL 1234

static int g_failures = 0;
static const char *g_self;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void timeout(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static void finishChild(void)
{
	fflush(stdout);
	_exit(g_failures != 0);
}

static void waitChild(pid_t pid, const char *what)
{
	int status = 0;
	CHECK(pid > 0 && waitpid(pid, &status, 0) == pid);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		printf("FAIL: %s\n", what);
		g_failures++;
	}
}

static int enterSandbox(void)
{
	char *err = (char *) 1;
	int rc = sandbox_init(PERMISSIVE_PROFILE, 0, &err);
	if (rc != 0 || err != NULL) {
		printf("FAIL: sandbox_init rc=%d err=%s\n", rc, (err && err != (char *) 1) ? err : "?");
		g_failures++;
	}
	return rc;
}

static void checkUnsandboxed(void)
{
	errno = ERRNO_SENTINEL;
	CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 0);
	CHECK(sandbox_check(0, NULL, SANDBOX_FILTER_NONE) == 0);
	CHECK(sandbox_check(getpid(), "file-read-data", SANDBOX_FILTER_PATH, "/etc/hosts") == 0);
	CHECK(errno == ERRNO_SENTINEL);
}

// QUIRK test (see sandbox_check in src/sandbox/sandbox.c): a pid that names
// no process reports 1 and leaves errno alone
static void quirk_unknown_pid_reports_1(void)
{
	pid_t gone = fork();
	if (gone == 0)
		_exit(0);
	CHECK(gone > 0 && waitpid(gone, NULL, 0) == gone);

	errno = ERRNO_SENTINEL;
	CHECK(sandbox_check(gone, NULL, SANDBOX_FILTER_NONE) == 1);
	CHECK(sandbox_check(gone, "file-read-data", SANDBOX_FILTER_PATH, "/etc/hosts") == 1);
	CHECK(sandbox_check(-1, NULL, SANDBOX_FILTER_NONE) == 1);
	CHECK(errno == ERRNO_SENTINEL);
}

static void checkIssue(void)
{
	char *token;

	errno = 0;
	token = sandbox_extension_issue_file(NULL, "/private/tmp", 0);
	CHECK(token == NULL && errno == EINVAL);
	free(token);

	errno = 0;
	token = sandbox_extension_issue_file(READ_CLASS, NULL, 0);
	CHECK(token == NULL && errno == EINVAL);
	free(token);

	token = sandbox_extension_issue_mach("com.apple.app-sandbox.mach", "com.apple.system.logger", 0);
	CHECK(token != NULL);
	free(token);

	errno = 0;
	CHECK(sandbox_extension_consume("garbage") == -1 && errno == EINVAL);
	errno = 0;
	CHECK(sandbox_extension_consume("") == -1 && errno == EINVAL);
}

// QUIRK test (see sandbox_extension_consume in src/sandbox/sandbox.c): a
// process that is not sandboxed gets handle 0, and any release succeeds
static void quirk_unsandboxed_consume_gives_handle_0(const char *token)
{
	CHECK(sandbox_extension_consume(token) == 0);
	CHECK(sandbox_extension_consume(token) == 0);
	CHECK(sandbox_extension_release(0) == 0);
	CHECK(sandbox_extension_release(0) == 0);
	CHECK(sandbox_extension_release(123456) == 0);
	CHECK(sandbox_extension_release(-1) == 0);
}

// In a sandboxed process: a positive handle, the same one for the same
// token, released once per consume
static void checkSandboxedExtensions(const char *parentToken)
{
	char *other = sandbox_extension_issue_file(READ_CLASS, "/usr", 0);
	CHECK(other != NULL);

	// a token issued by another process can be consumed
	int64_t handle = sandbox_extension_consume(parentToken);
	CHECK(handle > 0);
	CHECK(sandbox_extension_consume(parentToken) == handle);

	if (other) {
		int64_t otherHandle = sandbox_extension_consume(other);
		CHECK(otherHandle > 0 && otherHandle != handle);
		CHECK(sandbox_extension_release(otherHandle) == 0);
		free(other);
	}

	CHECK(sandbox_extension_release(handle) == 0);
	CHECK(sandbox_extension_release(handle) == 0);
	errno = 0;
	CHECK(sandbox_extension_release(handle) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(sandbox_extension_release(123456) == -1 && errno == EINVAL);
}

static void checkSandboxed(const char *parentToken)
{
	int ready[2], done[2];
	CHECK(pipe(ready) == 0 && pipe(done) == 0);

	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		char byte = 'x';
		close(ready[0]);
		close(done[1]);
		if (enterSandbox() != 0)
			finishChild();

		errno = ERRNO_SENTINEL;
		CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 1);
		CHECK(errno == ERRNO_SENTINEL);
		// the parent did not apply a profile
		CHECK(sandbox_check(getppid(), NULL, SANDBOX_FILTER_NONE) == 0);
		CHECK(sandbox_check(getpid(), "file-read-data", SANDBOX_FILTER_PATH, "/etc/hosts") == 0);

		// applying a profile again also succeeds
		CHECK(enterSandbox() == 0);

		checkSandboxedExtensions(parentToken);

		// a forked child is sandboxed too
		fflush(stdout);
		pid_t grandchild = fork();
		if (grandchild == 0) {
			CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 1);
			finishChild();
		}
		waitChild(grandchild, "forked child of a sandboxed process");

		// and so is an executed image
		fflush(stdout);
		pid_t execed = fork();
		if (execed == 0) {
			execl(g_self, g_self, "--report-sandboxed", (char *) NULL);
			printf("FAIL: exec errno=%d\n", errno);
			finishChild();
		}
		waitChild(execed, "exec in a sandboxed process");

		// let the parent look at this process
		CHECK(write(ready[1], &byte, 1) == 1);
		CHECK(read(done[0], &byte, 1) == 1);
		finishChild();
	}

	close(ready[1]);
	close(done[0]);
	char byte = 'x';
	CHECK(read(ready[0], &byte, 1) == 1);
	CHECK(sandbox_check(child, NULL, SANDBOX_FILTER_NONE) == 1);
	CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 0);
	CHECK(write(done[1], &byte, 1) == 1);
	waitChild(child, "sandboxed child");
	close(ready[0]);
	close(done[1]);
}

// sandbox_compile_string + sandbox_apply (libsandbox) sandbox the process too
static void checkApply(void)
{
	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		void *lib = dlopen("/usr/lib/libsandbox.1.dylib", RTLD_NOW);
		void *(*compileString)(const char *, void *, char **) = lib ? dlsym(lib, "sandbox_compile_string") : NULL;
		int (*apply)(void *) = lib ? dlsym(lib, "sandbox_apply") : NULL;
		void (*freeProfile)(void *) = lib ? dlsym(lib, "sandbox_free_profile") : NULL;
		CHECK(compileString != NULL && apply != NULL && freeProfile != NULL);
		if (compileString && apply && freeProfile) {
			char *err = NULL;
			void *profile = compileString(PERMISSIVE_PROFILE, NULL, &err);
			CHECK(profile != NULL);
			if (profile) {
				CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 0);
				CHECK(apply(profile) == 0);
				CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 1);
				freeProfile(profile);
			}
		}
		finishChild();
	}
	waitChild(child, "sandbox_apply");
}

int main(int argc, char **argv)
{
	signal(SIGALRM, timeout);
	alarm(30);
	g_self = argv[0];

	if (argc > 1 && strcmp(argv[1], "--report-sandboxed") == 0) {
		CHECK(sandbox_check(getpid(), NULL, SANDBOX_FILTER_NONE) == 1);
		finishChild();
	}

	char *token = sandbox_extension_issue_file(READ_CLASS, "/private/tmp", 0);
	CHECK(token != NULL);

	checkUnsandboxed();
	quirk_unknown_pid_reports_1();
	checkIssue();
	if (token) {
		quirk_unsandboxed_consume_gives_handle_0(token);
		checkSandboxed(token);
	}
	checkApply();
	checkUnsandboxed();
	free(token);

	if (g_failures == 0)
		puts("PASS");
	fflush(stdout);
	_exit(g_failures != 0);
}
