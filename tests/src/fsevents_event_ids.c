// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// FSEvents event IDs and flushing:
// - FSEventsGetCurrentEventId is nonzero and never decreases;
// - event IDs reported by different streams are drawn from one increasing sequence;
// - FSEventStreamFlushAsync and FSEventStreamFlushSync deliver events that are waiting for the
//   stream latency, and FSEventStreamCopyDescription returns a string;
// - device-level calls report nothing for an unknown device. Where the system keeps no
//   persistent event log (no volume UUID), there is no history to look up or purge.
#include <CoreServices/CoreServices.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static char g_dir[PATH_MAX];
static size_t g_callbacks;
static FSEventStreamEventId g_firstId, g_lastId;

static void callback(ConstFSEventStreamRef stream, void *info, size_t count, void *paths,
	const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
	(void) stream;
	(void) info;
	(void) paths;
	(void) flags;
	for (size_t i = 0; i < count; i++) {
		if (g_firstId == 0)
			g_firstId = ids[i];
		g_lastId = ids[i];
	}
	g_callbacks++;
}

static FSEventStreamRef makeStream(double latency)
{
	CFStringRef dir = CFStringCreateWithFileSystemRepresentation(NULL, g_dir);
	CFArrayRef paths = CFArrayCreate(NULL, (const void **) &dir, 1, &kCFTypeArrayCallBacks);
	FSEventStreamRef stream = FSEventStreamCreate(NULL, callback, NULL, paths, kFSEventStreamEventIdSinceNow,
		latency, kFSEventStreamCreateFlagFileEvents);
	CFRelease(paths);
	CFRelease(dir);
	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	FSEventStreamStart(stream);
	return stream;
}

static void touch(const char *name)
{
	char path[PATH_MAX + 16];
	snprintf(path, sizeof(path), "%s/%s", g_dir, name);
	int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0)
		close(fd);
}

static void cleanup(void)
{
	char command[PATH_MAX + 16];
	snprintf(command, sizeof(command), "rm -rf '%s'", g_dir);
	system(command);
}

static void finishStream(FSEventStreamRef stream)
{
	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);
}

int main(void)
{
	snprintf(g_dir, sizeof(g_dir), "/tmp/fseventids.XXXXXX");
	if (!mkdtemp(g_dir)) {
		perror("mkdtemp");
		return 1;
	}

	FSEventStreamEventId before = FSEventsGetCurrentEventId();
	CHECK(before != 0);
	CHECK(FSEventsGetCurrentEventId() >= before);

	// Flushing delivers events without waiting for the (long) latency.
	FSEventStreamRef a = makeStream(30.0);
	touch("one");
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
	CHECK(g_callbacks == 0);
	FSEventStreamFlushSync(a);
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, false);
	CHECK(g_callbacks >= 1);
	CHECK(g_lastId > before);

	// A second stream continues the same sequence.
	FSEventStreamEventId afterFirst = g_lastId;
	g_firstId = 0;
	FSEventStreamRef b = makeStream(30.0);
	touch("two");
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, false);
	FSEventStreamEventId flushed = FSEventStreamFlushAsync(b);
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, false);
	CHECK(g_firstId > afterFirst);
	CHECK(flushed == 0 || flushed >= g_firstId);
	CHECK(FSEventsGetCurrentEventId() >= g_lastId);

	CFStringRef description = FSEventStreamCopyDescription(b);
	CHECK(description != NULL && CFGetTypeID(description) == CFStringGetTypeID());
	if (description)
		CFRelease(description);
	CHECK(FSEventStreamCopyDescription(NULL) == NULL);
	CHECK(FSEventStreamFlushAsync(NULL) == 0);

	finishStream(a);
	finishStream(b);

	// Device-level calls.
	struct stat st;
	CHECK(stat(g_dir, &st) == 0);
	const dev_t unknown = (dev_t) 0x7ffffff0;
	CHECK(FSEventsCopyUUIDForDevice(unknown) == NULL);
	CHECK(FSEventsGetLastEventIdForDeviceBeforeTime(unknown, CFAbsoluteTimeGetCurrent()) == 0);
	CFUUIDRef uuid = FSEventsCopyUUIDForDevice(st.st_dev);
	if (uuid == NULL) {
		// No persistent event log: there is no history, and nothing to purge.
		CHECK(FSEventsGetLastEventIdForDeviceBeforeTime(st.st_dev, CFAbsoluteTimeGetCurrent()) == 0);
		CHECK(!FSEventsPurgeEventsForDeviceUpToEventId(st.st_dev, 1));
	} else {
		CFUUIDRef again = FSEventsCopyUUIDForDevice(st.st_dev);
		CHECK(again != NULL && CFEqual(uuid, again));
		if (again)
			CFRelease(again);
		CFRelease(uuid);
	}

	cleanup();
	if (failures == 0)
		puts("OK");
	return failures != 0;
}
