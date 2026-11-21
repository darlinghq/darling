// Copyright (C) 2026 Lubos Dolezel
//
// Distributed notifications between processes. The program starts itself as observer and poster processes for each
// phase (so it can run on macOS unchanged) and compares what the processes printed with what macOS 26.7.3 and
// Mojave 10.14.4 (x86_64 and i386) printed for the same scenario: observer filtering by name and object string,
// suspension behaviors, the userInfo and object rules of NSDistributedNotificationCenter, delivery on the main
// thread, delivery to the posting process, and removal. The phases p3 and p7 pin the suspension QUIRK
// (test_quirk_suspended_coalesce_hold): while a process is suspended, registrations that coalesce still receive
// notifications at once and registrations that hold receive none, see CFDN_ShouldDeliver in
// CoreFoundation/CFDistributedNotification_Private.h.
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
extern int pthread_main_np(void);
#include <dlfcn.h>
// Resolved at run time: the pair is not declared in the public CoreFoundation headers.
static void setSuspended(CFNotificationCenterRef c, Boolean s)
{
	((void (*)(CFNotificationCenterRef, Boolean))dlsym(RTLD_DEFAULT, "CFNotificationCenterSetSuspended"))(c, s);
}
static Boolean isSuspended(CFNotificationCenterRef c)
{
	return ((Boolean (*)(CFNotificationCenterRef))dlsym(RTLD_DEFAULT, "CFNotificationCenterIsSuspended"))(c);
}

static CFNotificationCenterRef dc(void) { return CFNotificationCenterGetDistributedCenter(); }

static void deliver(CFNotificationCenterRef c, void *obs, CFStringRef name, const void *object, CFDictionaryRef info)
{
	char n[64] = "(null)", o[64] = "(null)", k[64] = "-";
	if (name) CFStringGetCString(name, n, sizeof n, kCFStringEncodingUTF8);
	if (object) CFStringGetCString(object, o, sizeof o, kCFStringEncodingUTF8);
	int count = -1;
	if (info) {
		count = (int)CFDictionaryGetCount(info);
		CFTypeRef v = CFDictionaryGetValue(info, CFSTR("seq"));
		long long q = 0;
		if (v && CFGetTypeID(v) == CFNumberGetTypeID()) { CFNumberGetValue(v, kCFNumberLongLongType, &q); snprintf(k, sizeof k, "seq=%lld", q); }
		else if ((v = CFDictionaryGetValue(info, CFSTR("k"))) && CFGetTypeID(v) == CFStringGetTypeID()) CFStringGetCString(v, k, sizeof k, kCFStringEncodingUTF8);
	}
	printf("  recv %s: name=%s object=%s keys=%d %s main=%d\n", (const char *)obs, n, o, count, k, pthread_main_np());
	fflush(stdout);
}

static void add(const char *tag, const char *name, const char *object, CFNotificationSuspensionBehavior b)
{
	CFNotificationCenterAddObserver(dc(), tag, deliver, name ? CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8) : NULL,
		object ? (const void *)CFStringCreateWithCString(NULL, object, kCFStringEncodingUTF8) : NULL, b);
}

static void run(double seconds) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false); }

static void post(const char *name, const char *object, int seq, Boolean immediate)
{
	CFNumberRef num = CFNumberCreate(NULL, kCFNumberIntType, &seq);
	CFDictionaryRef info = CFDictionaryCreate(NULL, (const void *[]){ CFSTR("seq"), CFSTR("k") }, (const void *[]){ num, CFSTR("v") }, 2,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNotificationCenterPostNotification(dc(), CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8),
		object ? (const void *)CFStringCreateWithCString(NULL, object, kCFStringEncodingUTF8) : NULL, info, immediate);
}

static void *bgAdd(void *arg)
{
	add("bg", "P5", NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
	return NULL;
}

@interface Obs : NSObject
- (void)got:(NSNotification *)n;
@end
@implementation Obs
- (void)got:(NSNotification *)n
{
	printf("  recv ns: name=%s object=%s seq=%s main=%d\n", [n.name UTF8String], [[n object] UTF8String], [[[n.userInfo objectForKey:@"seq"] description] UTF8String], pthread_main_np());
	fflush(stdout);
}
@end

static void observe(const char *phase)
{
	if (!strcmp(phase, "p1")) {
		add("A", "P1", NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
		add("B", "P1", "x", CFNotificationSuspensionBehaviorDeliverImmediately);
		add("C", NULL, "x", CFNotificationSuspensionBehaviorDeliverImmediately);
		run(2.5);
	} else if (!strcmp(phase, "p3")) {
		add("drop", "Pd", "o", CFNotificationSuspensionBehaviorDrop);
		add("coalesce", "Pc", "o", CFNotificationSuspensionBehaviorCoalesce);
		add("hold", "Ph", "o", CFNotificationSuspensionBehaviorHold);
		add("immediate", "Pi", "o", CFNotificationSuspensionBehaviorDeliverImmediately);
		setSuspended(dc(), true);
		printf("  suspended=%d\n", isSuspended(dc()));
		run(2.5);
		printf("  -- resume\n");
		setSuspended(dc(), false);
		printf("  suspended=%d\n", isSuspended(dc()));
		run(3.0);
	} else if (!strcmp(phase, "p7")) {
		@autoreleasepool {
			NSDistributedNotificationCenter *c = [NSDistributedNotificationCenter defaultCenter];
			Obs *o = [[Obs alloc] init];
			const char *names[] = { "Pd", "Pc", "Ph", "Pi" };
			NSNotificationSuspensionBehavior b[] = { NSNotificationSuspensionBehaviorDrop, NSNotificationSuspensionBehaviorCoalesce, NSNotificationSuspensionBehaviorHold, NSNotificationSuspensionBehaviorDeliverImmediately };
			for (int i = 0; i < 4; i++)
				[c addObserver:o selector:@selector(got:) name:[NSString stringWithUTF8String:names[i]] object:@"o" suspensionBehavior:b[i]];
			c.suspended = YES;
			printf("  suspended=%d\n", c.suspended);
			run(2.5);
			printf("  -- resume\n");
			c.suspended = NO;
			run(3.0);
		}
	} else if (!strcmp(phase, "p4")) {
		add("A", "P4", NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
		run(2.5);
	} else if (!strcmp(phase, "p5")) {
		pthread_t t;
		pthread_create(&t, NULL, bgAdd, NULL);
		pthread_join(t, NULL);
		run(2.5);
	} else if (!strcmp(phase, "p6")) { // posted from this process too, and removal
		add("self", "P6", NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
		post("P6", "o", 1, false);
		run(0.5);
		CFNotificationCenterRemoveObserver(dc(), "self", CFSTR("P6"), NULL);
		printf("  -- removed\n");
		post("P6", "o", 2, false);
		run(0.5);
	}
}

static void poster(const char *phase)
{
	if (!strcmp(phase, "p1")) {
		post("P1", "x", 1, false); post("P1", "y", 2, false); post("P2", "x", 3, false); post("P1", NULL, 4, false);
		usleep(300000);
	} else if (!strcmp(phase, "p3") || !strcmp(phase, "p7")) {
		for (int s = 1; s <= 3; s++) { post("Pd", "o", s, false); post("Pc", "o", s, false); post("Ph", "o", s, false); post("Pi", "o", s, false); }
		post("Pd", "o", 99, true); post("Pc", "o", 99, true); post("Ph", "o", 99, true);
		usleep(300000);
	} else if (!strcmp(phase, "p4")) {
		@autoreleasepool {
			NSDistributedNotificationCenter *c = [NSDistributedNotificationCenter defaultCenter];
			@try {
				[c postNotificationName:@"P4" object:@"o" userInfo:@{ @"bad": [[NSObject alloc] init] } deliverImmediately:YES];
				printf("  post non-plist userInfo: no exception\n");
			} @catch (NSException *e) { printf("  post non-plist userInfo: %s\n", [[e name] UTF8String]); }
			@try {
				[c postNotificationName:@"P4" object:(id)@42 userInfo:nil deliverImmediately:YES];
				printf("  post non-string object: no exception\n");
			} @catch (NSException *e) { printf("  post non-string object: %s\n", [[e name] UTF8String]); }
			[c postNotificationName:@"P4" object:@"o" userInfo:@{ @"k": @"plist", @"seq": @7, @"d": [NSDate dateWithTimeIntervalSince1970:0], @"a": @[@1] } deliverImmediately:YES];
			usleep(300000);
		}
	} else if (!strcmp(phase, "p5")) {
		post("P5", NULL, 1, true);
		usleep(300000);
	}
	fflush(stdout);
}

static pid_t spawnSelf(const char *self, const char *role, const char *phase, int outFd)
{
	pid_t pid = 0;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	if (outFd >= 0)
		posix_spawn_file_actions_adddup2(&actions, outFd, 1);
	char *argv[] = { (char *)self, (char *)role, (char *)phase, NULL };
	posix_spawn(&pid, self, &actions, NULL, argv, environ);
	posix_spawn_file_actions_destroy(&actions);
	return pid;
}

static int g_failures = 0;

static int compareLines(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

// Reads everything from fd and returns the lines, sorted (the order between observers is not defined).
static int readLines(int fd, char **lines, int max)
{
	static char buffer[16384];
	size_t used = 0;
	ssize_t n;
	while (used < sizeof(buffer) - 1 && (n = read(fd, buffer + used, sizeof(buffer) - 1 - used)) > 0)
		used += (size_t)n;
	buffer[used] = 0;
	int count = 0;
	for (char *line = strtok(buffer, "\n"); line != NULL && count < max; line = strtok(NULL, "\n"))
		lines[count++] = strdup(line);
	qsort(lines, count, sizeof(char *), compareLines);
	return count;
}

static void expectPhase(const char *self, const char *phase, int withPoster, const char *const *expected, int expectedCount)
{
	int fds[2];
	pipe(fds);
	pid_t o = spawnSelf(self, "obs", phase, fds[1]);
	usleep(1200000);
	if (withPoster) {
		pid_t p = spawnSelf(self, "post", phase, fds[1]);
		int st;
		waitpid(p, &st, 0);
	}
	close(fds[1]);
	char *lines[64];
	int count = readLines(fds[0], lines, 64);
	close(fds[0]);
	int st;
	waitpid(o, &st, 0);
	char *want[64];
	for (int i = 0; i < expectedCount; i++) want[i] = strdup(expected[i]);
	qsort(want, expectedCount, sizeof(char *), compareLines);
	Boolean same = count == expectedCount;
	for (int i = 0; same && i < count; i++) same = strcmp(lines[i], want[i]) == 0;
	if (!same) {
		printf("FAIL phase %s\n  expected:\n", phase);
		for (int i = 0; i < expectedCount; i++) printf("    %s\n", want[i]);
		printf("  got:\n");
		for (int i = 0; i < count; i++) printf("    %s\n", lines[i]);
		g_failures++;
	}
}

#define PHASE(name, withPoster, ...) do { const char *e[] = { __VA_ARGS__ }; expectPhase(argv[0], name, withPoster, e, (int)(sizeof(e) / sizeof(e[0]))); } while (0)

int main(int argc, char **argv)
{
	@autoreleasepool {
		setvbuf(stdout, NULL, _IOLBF, 0);
		if (argc >= 3 && !strcmp(argv[1], "obs")) { observe(argv[2]); return 0; }
		if (argc >= 3 && !strcmp(argv[1], "post")) { poster(argv[2]); return 0; }

		// Observers filter by name and by object string; an observer without an object gets every object,
		// one with an object gets only that one, and a notification without an object only reaches observers without.
		PHASE("p1", 1,
			"  recv A: name=P1 object=x keys=2 seq=1 main=1", "  recv A: name=P1 object=y keys=2 seq=2 main=1",
			"  recv A: name=P1 object=(null) keys=2 seq=4 main=1", "  recv B: name=P1 object=x keys=2 seq=1 main=1",
			"  recv C: name=P1 object=x keys=2 seq=1 main=1", "  recv C: name=P2 object=x keys=2 seq=3 main=1");
		// test_quirk_suspended_coalesce_hold, through CoreFoundation: drop drops, immediate delivers, coalesce delivers
		// at once, hold delivers nothing; notifications posted for immediate delivery reach every registration.
		PHASE("p3", 1,
			"  suspended=1", "  -- resume", "  suspended=0",
			"  recv coalesce: name=Pc object=o keys=2 seq=1 main=1", "  recv coalesce: name=Pc object=o keys=2 seq=2 main=1",
			"  recv coalesce: name=Pc object=o keys=2 seq=3 main=1", "  recv coalesce: name=Pc object=o keys=2 seq=99 main=1",
			"  recv immediate: name=Pi object=o keys=2 seq=1 main=1", "  recv immediate: name=Pi object=o keys=2 seq=2 main=1",
			"  recv immediate: name=Pi object=o keys=2 seq=3 main=1", "  recv drop: name=Pd object=o keys=2 seq=99 main=1",
			"  recv hold: name=Ph object=o keys=2 seq=99 main=1");
		// The same through NSDistributedNotificationCenter.
		PHASE("p7", 1,
			"  suspended=1", "  -- resume",
			"  recv ns: name=Pc object=o seq=1 main=1", "  recv ns: name=Pc object=o seq=2 main=1", "  recv ns: name=Pc object=o seq=3 main=1",
			"  recv ns: name=Pc object=o seq=99 main=1", "  recv ns: name=Pi object=o seq=1 main=1", "  recv ns: name=Pi object=o seq=2 main=1",
			"  recv ns: name=Pi object=o seq=3 main=1", "  recv ns: name=Pd object=o seq=99 main=1", "  recv ns: name=Ph object=o seq=99 main=1");
		// A userInfo that is not a property list is not delivered (nothing is raised); an object that is not a string raises.
		PHASE("p4", 1,
			"  post non-plist userInfo: no exception", "  post non-string object: NSInvalidArgumentException",
			"  recv A: name=P4 object=o keys=4 seq=7 main=1");
		// Callbacks run on the main thread even for an observer added from another thread.
		PHASE("p5", 1, "  recv bg: name=P5 object=(null) keys=2 seq=1 main=1");
		// A process receives what it posts itself, and nothing after removing the observer.
		PHASE("p6", 0, "  recv self: name=P6 object=o keys=2 seq=1 main=1", "  -- removed");
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
