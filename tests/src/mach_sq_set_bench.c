// Copyright (C) 2026 Lubos Dolezel
//
// A run loop and dispatch main queue workload for counting darlingserver calls: the main thread waits for work
// the way applications do (CFRunLoopRun on the main thread, whose mode waits on a Mach port set), and a second
// thread (or timers) wakes it up N times.
//
//   sq_setbench <mode> <n>
//     rlsource   main run loop; a thread signals a run loop source and wakes the loop, waits for the source to run
//     maindisp   main run loop; a thread dispatch_async()s a block to the main queue, waits for it to run
//     dispmain   dispatch_main(): the same, for a process that parks its main thread in libdispatch
//     rltimer    main run loop with one repeating 2 ms timer, n fires
//     mixed      all of: a source, a main queue block and a timer, one round each
//
// Prints (to stderr) the number of iterations and the time they took.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>

static int N;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int acks;

static void ack(void)
{
	pthread_mutex_lock(&mu);
	acks++;
	pthread_cond_broadcast(&cv);
	pthread_mutex_unlock(&mu);
}

static void wait_ack(int n)
{
	pthread_mutex_lock(&mu);
	while (acks < n)
		pthread_cond_wait(&cv, &mu);
	pthread_mutex_unlock(&mu);
}

static double now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static double t_start;

static void report(const char* mode, int iterations)
{
	double t = now() - t_start;
	fprintf(stderr, "%s: %d iterations in %.3f s (%.0f/s)\n", mode, iterations, t, iterations / t);
}

static CFRunLoopRef loop;
static CFRunLoopSourceRef source;
static const char* current_mode;

static void source_perform(void* info)
{
	(void) info;
	ack();
}

static void add_source(void)
{
	CFRunLoopSourceContext c;
	memset(&c, 0, sizeof(c));
	c.perform = source_perform;
	source = CFRunLoopSourceCreate(NULL, 0, &c);
	CFRunLoopAddSource(loop, source, kCFRunLoopDefaultMode);
}

static void finish(void)
{
	report(current_mode, N);
	exit(0);
}

static void* source_poster(void* arg)
{
	(void) arg;
	usleep(300000);
	t_start = now();
	for (int i = 1; i <= N; i++)
	{
		CFRunLoopSourceSignal(source);
		CFRunLoopWakeUp(loop);
		wait_ack(i);
	}
	finish();
	return NULL;
}

static void* main_queue_poster(void* arg)
{
	(void) arg;
	usleep(300000);
	t_start = now();
	for (int i = 1; i <= N; i++)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			ack();
		});
		wait_ack(i);
	}
	finish();
	return NULL;
}

static int fires;
static void timer_fired(CFRunLoopTimerRef timer, void* info)
{
	(void) timer;
	(void) info;
	if (++fires == N)
		finish();
}

static void* mixed_poster(void* arg)
{
	(void) arg;
	usleep(300000);
	t_start = now();
	for (int i = 1; i <= N; i++)
	{
		CFRunLoopSourceSignal(source);
		CFRunLoopWakeUp(loop);
		wait_ack(2 * i - 1);
		dispatch_async(dispatch_get_main_queue(), ^{
			ack();
		});
		wait_ack(2 * i);
	}
	finish();
	return NULL;
}

int main(int argc, char** argv)
{
	pthread_t thread;

	if (argc < 3)
		return 2;
	current_mode = argv[1];
	N = atoi(argv[2]);
	loop = CFRunLoopGetCurrent();

	if (!strcmp(current_mode, "rlsource"))
	{
		add_source();
		pthread_create(&thread, NULL, source_poster, NULL);
		CFRunLoopRun();
	}
	else if (!strcmp(current_mode, "maindisp"))
	{
		pthread_create(&thread, NULL, main_queue_poster, NULL);
		CFRunLoopRun();
	}
	else if (!strcmp(current_mode, "dispmain"))
	{
		pthread_create(&thread, NULL, main_queue_poster, NULL);
		dispatch_main();
	}
	else if (!strcmp(current_mode, "rltimer"))
	{
		CFRunLoopTimerRef timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 0.3, 0.002, 0, 0, timer_fired, NULL);
		CFRunLoopAddTimer(loop, timer, kCFRunLoopDefaultMode);
		t_start = now() + 0.3;
		CFRunLoopRun();
	}
	else if (!strcmp(current_mode, "mixed"))
	{
		add_source();
		CFRunLoopTimerRef timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent() + 0.3, 0.05, 0, 0, timer_fired, NULL);
		CFRunLoopAddTimer(loop, timer, kCFRunLoopDefaultMode);
		fires = -1000000000;
		pthread_create(&thread, NULL, mixed_poster, NULL);
		CFRunLoopRun();
	}
	else
		return 2;
	return 0;
}
