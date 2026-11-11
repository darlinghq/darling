// Copyright (C) 2026 Lubos Dolezel
//
// libdyld exports _dyld_stack_range(), which reports the stack that dyld
// uses for itself. A process that runs on ordinary thread stacks gets an
// empty range: on macOS 27 (arm64 and x86_64) both outputs are set to NULL,
// on the main thread and on other threads. (Passing NULL for an output
// crashes on macOS, so the test always passes both.)

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

typedef void (*stack_range_fn)(const void** bottom, const void** top);

static stack_range_fn g_stack_range;
static int g_failures = 0;

static void check_empty(const char* where)
{
	int sentinel;
	const void* bottom = &sentinel;
	const void* top = &sentinel;
	g_stack_range(&bottom, &top);
	if (bottom != NULL || top != NULL) {
		printf("FAIL: %s: range %p-%p, expected NULL-NULL\n", where, bottom, top);
		g_failures++;
	}
}

static void* thread_main(void* arg)
{
	(void) arg;
	check_empty("second thread");
	return NULL;
}

int main(void)
{
	g_stack_range = (stack_range_fn) dlsym(RTLD_DEFAULT, "_dyld_stack_range");
	if (g_stack_range == NULL) {
		printf("FAIL: _dyld_stack_range is not exported\n");
		return 1;
	}

	Dl_info info;
	if (dladdr((void*) g_stack_range, &info) == 0 || info.dli_fname == NULL) {
		printf("FAIL: dladdr\n");
		g_failures++;
	} else {
		printf("_dyld_stack_range in %s\n", info.dli_fname);
	}

	check_empty("main thread");

	pthread_t thread;
	if (pthread_create(&thread, NULL, thread_main, NULL) != 0) {
		printf("FAIL: pthread_create\n");
		return 1;
	}
	pthread_join(thread, NULL);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
