// Copyright (C) 2026 Lubos Dolezel
//
// CVDisplayLink lifecycle: several links run at once and are independent
// (each delivers its own callbacks to its own user data, stopping one leaves
// the others running); CVDisplayLinkIsRunning follows Start and Stop (repeating either changes nothing); CVDisplayLinkStop and the last CVDisplayLinkRelease may be
// called from inside the link's own output callback; links can be started
// and stopped repeatedly; and releasing a running link from another thread
// stops its callbacks.
#include <CoreVideo/CoreVideo.h>
#include <CoreGraphics/CoreGraphics.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

typedef struct {
	int count;
	int stopAt;     // call CVDisplayLinkStop from the callback at this count
	int releaseAt;  // call CVDisplayLinkRelease from the callback at this count
} Counter;

static int get(Counter* c) { return __atomic_load_n(&c->count, __ATOMIC_SEQ_CST); }

static CVReturn output(CVDisplayLinkRef link, const CVTimeStamp* now, const CVTimeStamp* out,
	CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* info)
{
	Counter* c = (Counter*) info;
	int n = __atomic_add_fetch(&c->count, 1, __ATOMIC_SEQ_CST);
	if (c->stopAt && n == c->stopAt)
		CVDisplayLinkStop(link);
	if (c->releaseAt && n == c->releaseAt)
		CVDisplayLinkRelease(link);
	return kCVReturnSuccess;
}

static CVDisplayLinkRef make(Counter* c)
{
	CVDisplayLinkRef link = NULL;
	if (CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &link) != kCVReturnSuccess || !link)
		return NULL;
	CVDisplayLinkSetOutputCallback(link, output, c);
	return link;
}

static int waitFor(Counter* c, int n)
{
	for (int i = 0; i < 500; i++) {
		if (get(c) >= n)
			return 1;
		usleep(10000);
	}
	return 0;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(40);

	// Independent links.
	Counter cs[3] = { {0}, {0}, {0} };
	CVDisplayLinkRef links[3];
	for (int i = 0; i < 3; i++) {
		links[i] = make(&cs[i]);
		CHECK(links[i] != NULL);
		CHECK(!CVDisplayLinkIsRunning(links[i]));
	}
	for (int i = 0; i < 3; i++) {
		CHECK(CVDisplayLinkStart(links[i]) == kCVReturnSuccess);
		CVDisplayLinkStart(links[i]);  // already running: the return value is not specified
		CHECK(CVDisplayLinkIsRunning(links[i]));
	}
	for (int i = 0; i < 3; i++)
		CHECK(waitFor(&cs[i], 5));
	CHECK(CVDisplayLinkStop(links[1]) == kCVReturnSuccess);
	CVDisplayLinkStop(links[1]);  // already stopped: the return value is not specified
	CHECK(!CVDisplayLinkIsRunning(links[1]));
	CHECK(CVDisplayLinkIsRunning(links[0]) && CVDisplayLinkIsRunning(links[2]));
	int frozen = get(&cs[1]);
	int a = get(&cs[0]), b = get(&cs[2]);
	usleep(300000);
	CHECK(get(&cs[1]) == frozen);
	CHECK(get(&cs[0]) >= a + 5);
	CHECK(get(&cs[2]) >= b + 5);
	// The stopped link can be started again without disturbing the others.
	CHECK(CVDisplayLinkStart(links[1]) == kCVReturnSuccess);
	CHECK(waitFor(&cs[1], frozen + 3));
	// Releasing one running link leaves the others delivering.
	CVDisplayLinkRelease(links[0]);
	a = get(&cs[0]);
	usleep(200000);
	CHECK(get(&cs[0]) == a);
	b = get(&cs[2]);
	CHECK(waitFor(&cs[2], b + 3));
	CVDisplayLinkStop(links[1]);
	CVDisplayLinkStop(links[2]);
	CVDisplayLinkRelease(links[1]);
	CVDisplayLinkRelease(links[2]);

	// Stop from inside the callback.
	Counter stopper = { 0, 3, 0 };
	CVDisplayLinkRef s = make(&stopper);
	CHECK(CVDisplayLinkStart(s) == kCVReturnSuccess);
	CHECK(waitFor(&stopper, 3));
	for (int i = 0; i < 100 && CVDisplayLinkIsRunning(s); i++)
		usleep(10000);
	CHECK(!CVDisplayLinkIsRunning(s));
	int n = get(&stopper);
	usleep(200000);
	CHECK(get(&stopper) == n);
	// And it can be restarted afterwards (stopAt only fires at count 3).
	CHECK(CVDisplayLinkStart(s) == kCVReturnSuccess);
	CHECK(waitFor(&stopper, n + 3));
	CHECK(CVDisplayLinkIsRunning(s));
	CVDisplayLinkStop(s);
	CVDisplayLinkRelease(s);

	// Releasing the only reference from inside the callback.
	Counter releaser = { 0, 0, 3 };
	CVDisplayLinkRef r = make(&releaser);
	CHECK(CVDisplayLinkStart(r) == kCVReturnSuccess);
	CHECK(waitFor(&releaser, 3));
	usleep(300000);
	n = get(&releaser);
	usleep(200000);
	CHECK(get(&releaser) == n);
	CHECK(n == 3);

	// Start/stop cycles on one link.
	Counter cyc = { 0, 0, 0 };
	CVDisplayLinkRef c = make(&cyc);
	for (int i = 0; i < 20; i++) {
		int before = get(&cyc);
		CHECK(CVDisplayLinkStart(c) == kCVReturnSuccess);
		CHECK(CVDisplayLinkIsRunning(c));
		CHECK(waitFor(&cyc, before + 1));
		CHECK(CVDisplayLinkStop(c) == kCVReturnSuccess);
		CHECK(!CVDisplayLinkIsRunning(c));
	}
	n = get(&cyc);
	usleep(100000);
	CHECK(get(&cyc) == n);

	// Release while running (with a pending retain balanced first).
	CVDisplayLinkRetain(c);
	CHECK(CVDisplayLinkStart(c) == kCVReturnSuccess);
	CHECK(waitFor(&cyc, n + 2));
	CVDisplayLinkRelease(c);
	CHECK(CVDisplayLinkIsRunning(c));
	CVDisplayLinkRelease(c);
	n = get(&cyc);
	usleep(200000);
	CHECK(get(&cyc) == n);

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
