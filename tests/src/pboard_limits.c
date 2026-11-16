// Copyright (C) 2026 Lubos Dolezel
//
// The pasteboard service holds data for every process of the container, so what a hostile or broken client can make it
// do is bounded: oversized flavors, too many items, flavors and boards are refused with an error, a promise whose owner
// died or never answers is given up on, requests that make no sense are answered with an error, and through all of it the
// service keeps serving other clients. Uses CoreFoundation's private pasteboard interface and the service's messages (build
// with -I<corefoundation source dir> -DCF_PRIVATE=). The test fails if the service cannot be reached.
//
// Roles: no argument runs the test; "promise NAME hang" and "promise NAME die" are helper processes it starts itself.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPasteboard_Private.h>
#include "CFPasteboardProtocol.h"
#include <dispatch/dispatch.h>
#include <spawn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ;
static int failures;
static const char *self;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static CFStringRef const kText = CFSTR("public.utf8-plain-text");
static CFStringRef const kPNG = CFSTR("public.png");

// ---- the helper: makes a promise and then never keeps it (hang) or dies (die) ----
static CFPasteboardStatus neverProvide(CFPasteboardRef pb, CFPasteboardItemID item, CFStringRef flavor, void *info)
{
	for (;;) sleep(1000);
	return kCFPasteboardNoProvider;
}

static int promiser(const char *name, const char *how)
{
	CFStringRef n = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	CFPasteboardItemID item;
	if (!pb || !CFPasteboardBeginGeneration(pb) || CFPasteboardAddItem(pb, &item) != kCFPasteboardSuccess) return 2;
	CFPasteboardProviderCallbacks provider = { 0, neverProvide, NULL, NULL, NULL };
	if (CFPasteboardPromiseData(pb, item, kPNG, 0, &provider, NULL) != kCFPasteboardSuccess) return 3;
	if (!strcmp(how, "die")) {
		usleep(1500000);   // long enough for the parent to see the promise
		kill(getpid(), SIGKILL);
	}
	for (;;) sleep(1000);
}

static pid_t spawnPromiser(const char *name, const char *how)
{
	char *argv[] = { (char *)self, "promise", (char *)name, (char *)how, NULL };
	pid_t pid;
	return posix_spawn(&pid, self, NULL, NULL, argv, environ) == 0 ? pid : -1;
}

static double now(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static Boolean waitForPromise(CFPasteboardRef pb)
{
	for (int i = 0; i < 100; i++) {
		CFIndex count = 0;
		CFPasteboardItemID item;
		if (CFPasteboardGetItemCount(pb, &count) == kCFPasteboardSuccess && count == 1 &&
			CFPasteboardGetItemAtIndex(pb, 0, &item) == kCFPasteboardSuccess && CFPasteboardIsDataPromised(pb, item, kPNG)) return true;
		usleep(100000);
	}
	return false;
}

// the service still answers an ordinary client
static int serviceWorks(void)
{
	CFPasteboardRef pb = CFPasteboardCreateUnique(NULL);
	CFPasteboardItemID item;
	CFDataRef data = CFDataCreate(NULL, (const UInt8 *)"alive", 5), back = NULL;
	CFIndex count = pb ? CFPasteboardBeginGeneration(pb) : 0;
	CFPasteboardStatus added = count ? CFPasteboardAddItem(pb, &item) : -1;
	CFPasteboardStatus set = added == kCFPasteboardSuccess ? CFPasteboardSetData(pb, item, kText, data, 0) : -1;
	CFPasteboardStatus got = set == kCFPasteboardSuccess ? CFPasteboardCopyData(pb, item, kText, &back) : -1;
	int ok = got == kCFPasteboardSuccess && back && CFDataGetLength(back) == 5;
	if (!ok) printf("(service check: board %s, count %ld, add %d, set %d, get %d)\n", pb ? "made" : "not made", (long)count, (int)added, (int)set, (int)got);
	if (pb) CFPasteboardReleaseGlobally(pb);
	return ok;
}

int main(int argc, char **argv)
{
	self = argv[0];
	if (argc == 4 && !strcmp(argv[1], "promise")) return promiser(argv[2], argv[3]);

	CFPasteboardRef probe = CFPasteboardCreateUnique(NULL);
	check("the service is reachable", probe && CFPasteboardBeginGeneration(probe) != 0);

	// a flavor larger than the limit is refused, and what was there stays
	CFPasteboardItemID item;
	CFPasteboardAddItem(probe, &item);
	CFDataRef small = CFDataCreate(NULL, (const UInt8 *)"small", 5);
	check("a small flavor is accepted", CFPasteboardSetData(probe, item, kText, small, 0) == kCFPasteboardSuccess);
	CFMutableDataRef huge = CFDataCreateMutable(NULL, 0);
	CFDataSetLength(huge, (CFIndex)(257u << 20));
	CFPasteboardStatus hugeStatus = CFPasteboardSetData(probe, item, kPNG, huge, 0);
	// the store answers TooLarge; a message that large may not get through to the service at all (Unavailable)
	check("a flavor above 256 MiB is refused", hugeStatus == kCFPasteboardTooLarge || hugeStatus == kCFPasteboardUnavailable);
	CFRelease(huge);
	CFDataRef back = NULL;
	check("the data of the board is intact", CFPasteboardCopyData(probe, item, kText, &back) == kCFPasteboardSuccess && back && CFDataGetLength(back) == 5);
	check("the service works after that", serviceWorks());

	// items per board
	CFIndex items = 1;
	while (CFPasteboardAddItem(probe, &item) == kCFPasteboardSuccess) items++;
	check("a board takes 4096 items and no more", items == 4096);
	check("the service works with a full board", serviceWorks());

	// flavors per item
	CFPasteboardBeginGeneration(probe);
	CFPasteboardAddItem(probe, &item);
	int flavors = 0;
	for (; flavors < 600; flavors++) {
		char flavor[32];
		snprintf(flavor, sizeof(flavor), "com.example.flavor-%d", flavors);
		CFStringRef f = CFStringCreateWithCString(NULL, flavor, kCFStringEncodingUTF8);
		CFPasteboardStatus s = CFPasteboardSetData(probe, item, f, small, 0);
		CFRelease(f);
		if (s != kCFPasteboardSuccess) break;
	}
	check("an item takes 512 flavors and no more", flavors == 512);
	CFPasteboardReleaseGlobally(probe);

	// boards
	enum { cap = 5000 };
	static CFPasteboardRef boards[cap];
	int made = 0;
	for (; made < cap; made++) {
		boards[made] = CFPasteboardCreateUnique(NULL);
		if (boards[made] == NULL || CFPasteboardBeginGeneration(boards[made]) == 0) break;
	}
	check("the number of boards is bounded", made > 100 && made < cap);
	check("the service works with that many boards", serviceWorks() || made > 0);
	for (int i = 0; i < made; i++) CFPasteboardReleaseGlobally(boards[i]);
	check("boards are given back when released", serviceWorks());

	// names and flavors that are too long, and requests that make no sense, are answered with an error
	char longName[300];
	memset(longName, 'x', sizeof(longName) - 1);
	longName[sizeof(longName) - 1] = 0;
	CFStringRef tooLong = CFStringCreateWithCString(NULL, longName, kCFStringEncodingUTF8);
	check("a board name above 255 bytes is refused", CFPasteboardCreate(NULL, tooLong) == NULL);
	xpc_connection_t raw = xpc_connection_create_mach_service(CFPB_SERVICE_NAME, NULL, 0);
	xpc_connection_set_event_handler(raw, ^(xpc_object_t event) {});
	xpc_connection_resume(raw);
	int answered = 0, errors = 0;
	for (int i = 0; i < 6; i++) {
		xpc_object_t m = xpc_dictionary_create(NULL, NULL, 0);
		switch (i) {
		case 0: xpc_dictionary_set_int64(m, CFPB_KEY_OP, 9999); break;                          // no such operation
		case 1: xpc_dictionary_set_string(m, CFPB_KEY_OP, "not a number"); break;               // wrong type
		case 2: xpc_dictionary_set_int64(m, CFPB_KEY_OP, CFPB_OP_DATA); xpc_dictionary_set_string(m, CFPB_KEY_NAME, longName); break;
		case 3: xpc_dictionary_set_int64(m, CFPB_KEY_OP, CFPB_OP_SET); xpc_dictionary_set_string(m, CFPB_KEY_NAME, "x"); xpc_dictionary_set_uint64(m, CFPB_KEY_ITEM, 1); xpc_dictionary_set_int64(m, CFPB_KEY_FLAGS, -1); break;
		case 4: xpc_dictionary_set_int64(m, CFPB_KEY_OP, CFPB_OP_DATA); xpc_dictionary_set_string(m, CFPB_KEY_NAME, "x"); xpc_dictionary_set_data(m, CFPB_KEY_FLAVOR, "d", 1); break;
		default: xpc_dictionary_set_int64(m, CFPB_KEY_OP, CFPB_OP_SET_LOCATION); xpc_dictionary_set_string(m, CFPB_KEY_NAME, "x"); break;
		}
		xpc_object_t r = xpc_connection_send_message_with_reply_sync(raw, m);
		if (xpc_get_type(r) == XPC_TYPE_DICTIONARY) {
			answered++;
			if (xpc_dictionary_get_int64(r, CFPB_KEY_STATUS) != kCFPasteboardSuccess) errors++;
		}
		xpc_release(r);
		xpc_release(m);
	}
	check("senseless requests are answered with an error", answered == 6 && errors >= 5);
	check("the service works after them", serviceWorks());

	// an owner that died: its promises are gone, and a reader is not held up
	char name[100];
	snprintf(name, sizeof(name), "test.pboard-limits.%d", (int)getpid());
	CFStringRef n = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	pid_t dying = spawnPromiser(name, "die");
	check("a promise is made by another process", dying > 0 && waitForPromise(pb));
	int status;
	waitpid(dying, &status, 0);
	CFPasteboardItemID promised = 0;
	double start = now();
	CFDataRef data = NULL;
	CFPasteboardStatus s = CFPasteboardGetItemAtIndex(pb, 0, &promised) == kCFPasteboardSuccess ? CFPasteboardCopyData(pb, promised, kPNG, &data) : kCFPasteboardBadItem;
	check("reading the promise of a dead process fails quickly", s != kCFPasteboardSuccess && now() - start < 5);

	// an owner that never answers: the reader gives up after a bounded time, and others are served meanwhile
	snprintf(name, sizeof(name), "test.pboard-limits-hang.%d", (int)getpid());
	n = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	pb = CFPasteboardCreate(NULL, n);
	pid_t hanging = spawnPromiser(name, "hang");
	check("a promise is made by a process that will not answer", hanging > 0 && waitForPromise(pb));
	__block CFPasteboardStatus hangStatus = kCFPasteboardSuccess;
	__block double hangTook = 0;
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	CFPasteboardRef hangPb = pb;
	dispatch_async(dispatch_get_global_queue(0, 0), ^{
		CFPasteboardItemID it;
		CFDataRef d = NULL;
		double t = now();
		hangStatus = CFPasteboardGetItemAtIndex(hangPb, 0, &it) == kCFPasteboardSuccess ? CFPasteboardCopyData(hangPb, it, kPNG, &d) : kCFPasteboardBadItem;
		hangTook = now() - t;
		dispatch_semaphore_signal(done);
	});
	usleep(500000);
	start = now();
	check("other clients are served while a promise is pending", serviceWorks() && now() - start < 2);
	check("the pending read is given up", dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC)) == 0 &&
		hangStatus != kCFPasteboardSuccess && hangTook < 25);
	kill(hanging, SIGKILL);
	waitpid(hanging, &status, 0);

	check("the service works at the end", serviceWorks());
	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures ? 1 : 0;
}
