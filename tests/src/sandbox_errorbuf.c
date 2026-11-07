// Copyright (C) 2026 Lubos Dolezel
//
// sandbox_init(3): the error buffer may be NULL, it is set to NULL on
// success and to a message freed with sandbox_free_error() on failure.
// The extension and profile SPI follow the prototypes their clients
// declare (e.g. WebKit's SandboxSPI.h).
#define __APPLE_API_PRIVATE
#include <sandbox.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// A non-NULL value that must be overwritten
#define SENTINEL ((char*) 0x1234)

// Entering a sandbox is once per process, so each call gets its own child
static void inChild(void (*check)(void), const char* what)
{
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0)
	{
		g_failures = 0;
		check();
		fflush(stdout);
		_exit(g_failures != 0);
	}

	int status = 0;
	CHECK(pid > 0 && waitpid(pid, &status, 0) == pid);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		printf("FAIL: %s\n", what);
		g_failures++;
	}
}

static const char* const g_none[] = { NULL };

// A NULL errorbuf is allowed
static void initNull(void)
{
	CHECK(sandbox_init(kSBXProfileNoWrite, SANDBOX_NAMED, NULL) == 0);
}

static void initWithParametersNull(void)
{
	CHECK(sandbox_init_with_parameters(kSBXProfileNoWrite, SANDBOX_NAMED, g_none, NULL) == 0);
}

static void initWithExtensionsNull(void)
{
	CHECK(sandbox_init_with_extensions(kSBXProfileNoWrite, SANDBOX_NAMED, g_none, NULL) == 0);
}

// On success, *errorbuf is NULL
static void initBuffer(void)
{
	char* err = SENTINEL;
	CHECK(sandbox_init(kSBXProfileNoWrite, SANDBOX_NAMED, &err) == 0);
	CHECK(err == NULL);
}

static void initWithParametersBuffer(void)
{
	char* err = SENTINEL;
	CHECK(sandbox_init_with_parameters(kSBXProfileNoWrite, SANDBOX_NAMED, g_none, &err) == 0);
	CHECK(err == NULL);
}

static void initWithExtensionsBuffer(void)
{
	char* err = SENTINEL;
	CHECK(sandbox_init_with_extensions(kSBXProfileNoWrite, SANDBOX_NAMED, g_none, &err) == 0);
	CHECK(err == NULL);
}

static void checkInitFamily(void)
{
	inChild(initNull, "sandbox_init, NULL errorbuf");
	inChild(initWithParametersNull, "sandbox_init_with_parameters, NULL errorbuf");
	inChild(initWithExtensionsNull, "sandbox_init_with_extensions, NULL errorbuf");
	inChild(initBuffer, "sandbox_init, errorbuf");
	inChild(initWithParametersBuffer, "sandbox_init_with_parameters, errorbuf");
	inChild(initWithExtensionsBuffer, "sandbox_init_with_extensions, errorbuf");
}

// sandbox_wakeup_daemon fails on Darling: no daemon to wake
static void checkWakeupDaemon(void)
{
	char* err = NULL;

	CHECK(sandbox_wakeup_daemon(NULL) != 0);
	CHECK(sandbox_wakeup_daemon(&err) != 0);

	// On failure, *errorbuf describes it
	CHECK(err != NULL);
	sandbox_free_error(err);
}

static void checkExtensions(void)
{
	char* token = sandbox_extension_issue_file("com.apple.app-sandbox.read", "/", 0);
	CHECK(token != NULL);

	if (token)
	{
		int64_t handle = sandbox_extension_consume(token);
		// a process that is not sandboxed gets handle 0 (see sandbox_state.c)
		CHECK(handle >= 0);
		CHECK(sandbox_extension_release(handle) == 0);
		free(token);
	}

	token = sandbox_extension_issue_mach("com.apple.app-sandbox.mach", "com.apple.system.logger", 0);
	CHECK(token != NULL);
	free(token);
}

struct profile
{
	char* builtin;
	unsigned char* data;
	size_t size;
};

static void checkProfiles(void)
{
	void* lib = dlopen("/usr/lib/libsandbox.1.dylib", RTLD_NOW);
	CHECK(lib != NULL);
	if (!lib)
		return;

	struct profile* (*compileString)(const char*, void*, char**) = dlsym(lib, "sandbox_compile_string");
	void (*freeProfile)(struct profile*) = dlsym(lib, "sandbox_free_profile");
	void (*freeParams)(void*) = dlsym(lib, "sandbox_free_params");

	CHECK(compileString != NULL && freeProfile != NULL);
	CHECK(freeParams != NULL);

	if (compileString && freeProfile)
	{
		// The params argument is opaque and must not be written through
		void* params = SENTINEL;
		char* err = SENTINEL;
		struct profile* p = compileString("(version 1)", &params, &err);

		CHECK(p != NULL);
		CHECK(err == NULL);
		CHECK(params == SENTINEL);

		if (p)
		{
			CHECK(p->size == 0 || p->data != NULL);
			freeProfile(p);
		}
	}

	if (freeParams)
		freeParams(NULL);
}

int main(void)
{
	checkExtensions();
	checkProfiles();
	checkWakeupDaemon();
	checkInitFamily();

	if (g_failures == 0)
		puts("PASS");
	return g_failures != 0;
}
