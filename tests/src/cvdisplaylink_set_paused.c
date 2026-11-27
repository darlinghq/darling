// Copyright (C) 2026 Lubos Dolezel
//
// CVDisplayLinkSetPaused suspends and resumes the callbacks of a link without
// changing whether it is running: a paused link still reports running, resuming
// a stopped link does not start it, and the paused state outlasts a stop and a
// start. A NULL link is rejected with kCVReturnInvalidArgument. The behaviour
// was observed on macOS 26.7.3 (black-box).
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CoreGraphics.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>


static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig) { (void) sig; printf("FAIL: timed out\n"); fflush(stdout); _exit(1); }

static int g_count;

static CVReturn output(CVDisplayLinkRef link, const CVTimeStamp* now, const CVTimeStamp* out,
	CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* info)
{
	__atomic_add_fetch(&g_count, 1, __ATOMIC_SEQ_CST);
	return kCVReturnSuccess;
}

static int get(void) { return __atomic_load_n(&g_count, __ATOMIC_SEQ_CST); }

static int waitFor(int n)
{
	for (int i = 0; i < 500; i++) {
		if (get() >= n)
			return 1;
		usleep(10000);
	}
	return 0;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(40);

	CHECK(CVDisplayLinkSetPaused(NULL, true) == kCVReturnInvalidArgument);

	CVDisplayLinkRef link = NULL;
	CHECK(CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &link) == kCVReturnSuccess && link);
	if (!link)
		return 1;
	CVDisplayLinkSetOutputCallback(link, output, NULL);

	// Pausing and resuming a stopped link neither starts it nor fails.
	CHECK(CVDisplayLinkSetPaused(link, true) == kCVReturnSuccess);
	CHECK(!CVDisplayLinkIsRunning(link));
	CHECK(CVDisplayLinkSetPaused(link, false) == kCVReturnSuccess);
	CHECK(!CVDisplayLinkIsRunning(link));
	usleep(200000);
	CHECK(get() == 0);

	// A running link keeps running while paused but delivers nothing.
	CHECK(CVDisplayLinkStart(link) == kCVReturnSuccess);
	CHECK(waitFor(5));
	CHECK(CVDisplayLinkSetPaused(link, true) == kCVReturnSuccess);
	CHECK(CVDisplayLinkIsRunning(link));
	usleep(100000);  // a callback in flight may still complete
	int frozen = get();
	usleep(400000);
	CHECK(get() == frozen);
	CHECK(CVDisplayLinkIsRunning(link));

	// Resuming delivers again.
	CHECK(CVDisplayLinkSetPaused(link, false) == kCVReturnSuccess);
	CHECK(CVDisplayLinkIsRunning(link));
	CHECK(waitFor(frozen + 5));

	// Stop wins over pause, and resuming after a stop does not restart the link.
	CHECK(CVDisplayLinkStop(link) == kCVReturnSuccess);
	CHECK(CVDisplayLinkSetPaused(link, true) == kCVReturnSuccess);
	CHECK(CVDisplayLinkSetPaused(link, false) == kCVReturnSuccess);
	CHECK(!CVDisplayLinkIsRunning(link));
	int stopped = get();
	usleep(300000);
	CHECK(get() == stopped);

	// The paused state survives a stop and a start: the restarted link is silent until resumed.
	CHECK(CVDisplayLinkSetPaused(link, true) == kCVReturnSuccess);
	CHECK(CVDisplayLinkStart(link) == kCVReturnSuccess);
	CHECK(CVDisplayLinkIsRunning(link));
	usleep(100000);
	stopped = get();
	usleep(400000);
	CHECK(get() == stopped);
	CHECK(CVDisplayLinkSetPaused(link, false) == kCVReturnSuccess);
	CHECK(waitFor(stopped + 5));

	// A paused link can be stopped and released.
	CHECK(CVDisplayLinkSetPaused(link, true) == kCVReturnSuccess);
	CHECK(CVDisplayLinkStop(link) == kCVReturnSuccess);
	CHECK(!CVDisplayLinkIsRunning(link));
	CVDisplayLinkRelease(link);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
