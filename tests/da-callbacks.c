// Copyright (C) 2026 Lubos Dolezel
// DiskArbitration disk appeared, disappeared and description-changed callbacks with disk images:
//  existing <mount point>
//      appeared with kDADiskDescriptionMatchVolumeMountable on the run loop reports the volume's leaf exactly once
//      and its whole disk not at all;
//  attach-detach <image> <mount point>
//      on a dispatch queue, attaching gives appeared diskNs1 then diskN (macOS's order); detaching gives changed
//      diskNs1 (keys with DAVolumePath), then disappeared diskN, then disappeared diskNs1;
//  unscheduled
//      a session that is never scheduled gets no callbacks;
//  unregister
//      after DAUnregisterCallback (here from inside the first callback) nothing more is delivered;
//  restart <image> <mount point> <shell command that kills diskarbitrationd>
//      a session keeps getting callbacks after the daemon restarts (not on macOS: it needs to restart the daemon).
// Link with -framework DiskArbitration -framework CoreFoundation. Except for restart, the program also passes on
// macOS 26.7.3.
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

// what the callbacks saw, in order
struct event
{
	char kind; // 'a'ppeared, 'd'isappeared, 'c'hanged
	char name[64];
	int volumePath; // for 'c': the changed keys contain DAVolumePath
};
static struct event events[4096];
static int eventCount;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void record(char kind, DADiskRef disk, CFArrayRef keys)
{
	pthread_mutex_lock(&lock);
	if (eventCount < (int) (sizeof(events) / sizeof(events[0])))
	{
		struct event* e = &events[eventCount++];
		e->kind = kind;
		snprintf(e->name, sizeof(e->name), "%s", DADiskGetBSDName(disk) ? DADiskGetBSDName(disk) : "?");
		e->volumePath = keys != NULL
			&& CFArrayContainsValue(keys, CFRangeMake(0, CFArrayGetCount(keys)), kDADiskDescriptionVolumePathKey);
	}
	pthread_mutex_unlock(&lock);
}

static int count(char kind, const char* name)
{
	int n = 0;
	pthread_mutex_lock(&lock);
	for (int i = 0; i < eventCount; i++)
		n += events[i].kind == kind && (name == NULL || strcmp(events[i].name, name) == 0);
	pthread_mutex_unlock(&lock);
	return n;
}

static void dump(void)
{
	pthread_mutex_lock(&lock);
	for (int i = 0; i < eventCount; i++)
		printf("  %c %s%s\n", events[i].kind, events[i].name, events[i].volumePath ? " [DAVolumePath]" : "");
	pthread_mutex_unlock(&lock);
}

static void appeared(DADiskRef disk, void* context)
{
	record('a', disk, NULL);
}

static void disappeared(DADiskRef disk, void* context)
{
	record('d', disk, NULL);
}

static void changed(DADiskRef disk, CFArrayRef keys, void* context)
{
	record('c', disk, keys);
}

static void appearedOnce(DADiskRef disk, void* context)
{
	record('a', disk, NULL);
	DAUnregisterCallback((DASessionRef) context, appearedOnce, context);
}

static void runLoop(double seconds)
{
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
}

static int run(const char* const argv[])
{
	pid_t pid;
	int status;
	if (posix_spawn(&pid, argv[0], NULL, NULL, (char* const*) argv, environ) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int hdiutil(const char* verb, const char* path)
{
	const char* argv[] = { "/usr/bin/hdiutil", verb, "-quiet", path, NULL };
	return run(argv);
}

// The BSD names of the volume mounted on mp and of its whole disk.
static int names(DASessionRef session, const char* mp, char* leaf, char* whole)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) mp, strlen(mp), true);
	DADiskRef disk = DADiskCreateFromVolumePath(NULL, session, url);
	CFRelease(url);
	if (disk == NULL)
		return 0;
	DADiskRef wholeDisk = DADiskCopyWholeDisk(disk);
	snprintf(leaf, 64, "%s", DADiskGetBSDName(disk));
	snprintf(whole, 64, "%s", wholeDisk ? DADiskGetBSDName(wholeDisk) : "");
	if (wholeDisk != NULL)
		CFRelease(wholeDisk);
	CFRelease(disk);
	return 1;
}

static void existing(DASessionRef session, const char* mp)
{
	char leaf[64], whole[64];
	CHECK(names(session, mp, leaf, whole));

	DARegisterDiskAppearedCallback(session, kDADiskDescriptionMatchVolumeMountable, appeared, NULL);
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	runLoop(2);
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	CHECK(count('a', leaf) == 1);
	CHECK(count('a', whole) == 0);
}

// The position of the first event of that kind and name at or after start, or -1.
static int find(int start, char kind, const char* name, int volumePath)
{
	int found = -1;
	pthread_mutex_lock(&lock);
	for (int i = start; i < eventCount && found < 0; i++)
	{
		if (events[i].kind == kind && strcmp(events[i].name, name) == 0 && (!volumePath || events[i].volumePath))
			found = i;
	}
	pthread_mutex_unlock(&lock);
	return found;
}

static void attachDetach(DASessionRef session, const char* image, const char* mp)
{
	char leaf[64], whole[64];
	dispatch_queue_t queue = dispatch_queue_create("da-callbacks", NULL);

	DARegisterDiskAppearedCallback(session, NULL, appeared, NULL);
	DARegisterDiskDisappearedCallback(session, NULL, disappeared, NULL);
	DARegisterDiskDescriptionChangedCallback(session, NULL, NULL, changed, NULL);
	DASessionSetDispatchQueue(session, queue);
	sleep(2); // the disks that are already there

	pthread_mutex_lock(&lock);
	int attachStart = eventCount;
	pthread_mutex_unlock(&lock);

	CHECK(hdiutil("attach", image) == 0);
	sleep(2);
	if (!names(session, mp, leaf, whole))
	{
		CHECK(!"the image's volume has a disk");
		return;
	}
	pthread_mutex_lock(&lock);
	int detachStart = eventCount;
	pthread_mutex_unlock(&lock);

	CHECK(hdiutil("detach", mp) == 0);
	sleep(2);
	DASessionSetDispatchQueue(session, NULL);

	int appearedWhole = find(attachStart, 'a', whole, 0);
	int appearedLeaf = find(attachStart, 'a', leaf, 0);
	CHECK(appearedLeaf >= 0 && appearedLeaf < detachStart);
	CHECK(appearedWhole > appearedLeaf && appearedWhole < detachStart);

	int changedLeaf = find(detachStart, 'c', leaf, 1);
	int disappearedWhole = find(detachStart, 'd', whole, 0);
	int disappearedLeaf = find(detachStart, 'd', leaf, 0);
	CHECK(changedLeaf >= 0);
	CHECK(disappearedWhole > changedLeaf);
	CHECK(disappearedLeaf > disappearedWhole);
	if (failures)
		dump();
}

static void unscheduled(DASessionRef session)
{
	DARegisterDiskAppearedCallback(session, NULL, appeared, NULL);
	runLoop(2);
	CHECK(count('a', NULL) == 0);
}

static void unregister(DASessionRef session)
{
	DARegisterDiskAppearedCallback(session, NULL, appearedOnce, session);
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	runLoop(2);
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CHECK(count('a', NULL) == 1);
}

static void restart(DASessionRef session, const char* image, const char* mp, const char* command)
{
	char leaf[64], whole[64];
	const char* argv[] = { "/bin/sh", "-c", command, NULL };

	DARegisterDiskAppearedCallback(session, NULL, appeared, NULL);
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	runLoop(1);

	CHECK(run(argv) == 0);
	runLoop(2);
	CHECK(hdiutil("attach", image) == 0);
	runLoop(2);

	if (names(session, mp, leaf, whole))
	{
		CHECK(count('a', leaf) == 1);
		CHECK(count('a', whole) == 1);
		hdiutil("detach", mp);
	}
	else
	{
		CHECK(!"the image's volume has a disk after the restart");
	}
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	if (failures)
		dump();
}

int main(int argc, char** argv)
{
	DASessionRef session = DASessionCreate(NULL);
	if (session == NULL)
	{
		printf("FAIL: DASessionCreate\n");
		return 1;
	}

	if (argc == 3 && strcmp(argv[1], "existing") == 0)
		existing(session, argv[2]);
	else if (argc == 4 && strcmp(argv[1], "attach-detach") == 0)
		attachDetach(session, argv[2], argv[3]);
	else if (argc == 2 && strcmp(argv[1], "unscheduled") == 0)
		unscheduled(session);
	else if (argc == 2 && strcmp(argv[1], "unregister") == 0)
		unregister(session);
	else if (argc == 5 && strcmp(argv[1], "restart") == 0)
		restart(session, argv[2], argv[3], argv[4]);
	else
	{
		fprintf(stderr, "usage: da-callbacks existing <mp> | attach-detach <image> <mp> | unscheduled | unregister"
			" | restart <image> <mp> <command>\n");
		return 2;
	}

	CFRelease(session);
	printf(failures ? "FAILED\n" : "PASSED\n");
	return failures ? 1 : 0;
}
