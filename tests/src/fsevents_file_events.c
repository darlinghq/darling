// Copyright (C) 2026 Lubos Dolezel
//
// A file-level FSEvents stream (kFSEventStreamCreateFlagFileEvents)
// reports every change below the watched directory:
// - 50 files created in quick succession are all reported, each with
//   ItemCreated and ItemIsFile;
// - a rename is reported for both the old and the new path, each with
//   ItemRenamed;
// - a subdirectory created after the stream started is reported with
//   ItemCreated and ItemIsDir, and a file created in it right away with
//   ItemCreated and ItemIsFile.
// Event IDs increase and the callback never receives zero events.
//
// Stream lifetime:
// - 200 streams (alternately on a run loop and a dispatch queue) that are
//   created, started, stopped, started, stopped, invalidated and released
//   are freed: the context release callback runs once for each, and the
//   number of open descriptors (and, where /proc/self/fdinfo exists, of
//   inotify watches) returns to where it was;
// - quirks observed on macOS 27.0.1: a stream released while started is
//   not freed and keeps calling back; a stream with a dispatch queue that
//   is released without FSEventStreamInvalidate() is not freed.
// The program passes unchanged on macOS 27.0.1.
#include <CoreServices/CoreServices.h>
#include <dirent.h>
#include <dispatch/dispatch.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_COUNT 50
#define WAIT_SECONDS 10.0
#define LIFECYCLE_COUNT 200

struct event
{
	char* path;
	FSEventStreamEventFlags flags;
};

static struct event* g_events;
static size_t g_eventCount;
static FSEventStreamEventId g_lastId;
static bool g_idsIncrease = true;
static bool g_emptyCallback = false;
static char g_dir[PATH_MAX];
static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

static void callback(ConstFSEventStreamRef stream, void* info, size_t count, void* paths,
	const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
	(void) stream;
	(void) info;

	if (count == 0)
		g_emptyCallback = true;

	g_events = realloc(g_events, sizeof(*g_events) * (g_eventCount + count));
	for (size_t i = 0; i < count; i++)
	{
		g_events[g_eventCount].path = strdup(((char**) paths)[i]);
		g_events[g_eventCount].flags = flags[i];
		g_eventCount++;

		if (ids[i] <= g_lastId)
			g_idsIncrease = false;
		g_lastId = ids[i];
	}
}

static int g_contextReleases;
static int g_lifecycleCallbacks;

static const void* context_retain(const void* info)
{
	return info;
}

static void context_release(const void* info)
{
	(void) info;
	__atomic_add_fetch(&g_contextReleases, 1, __ATOMIC_SEQ_CST);
}

static void lifecycle_callback(ConstFSEventStreamRef stream, void* info, size_t count, void* paths,
	const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
	(void) stream;
	(void) info;
	(void) count;
	(void) paths;
	(void) flags;
	(void) ids;
	__atomic_add_fetch(&g_lifecycleCallbacks, 1, __ATOMIC_SEQ_CST);
}

static int count_fds(void)
{
	int count = 0;
	DIR* d = opendir("/dev/fd");
	if (d == NULL)
		return -1;
	struct dirent* ent;
	while ((ent = readdir(d)) != NULL)
	{
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			count++;
	}
	closedir(d);
	return count;
}

// Number of inotify watches the process holds, or -1 where the system
// does not show them (macOS has no /proc).
static int count_inotify_watches(void)
{
	DIR* d = opendir("/proc/self/fdinfo");
	if (d == NULL)
		return -1;

	int count = 0;
	struct dirent* ent;
	while ((ent = readdir(d)) != NULL)
	{
		char path[PATH_MAX];
		char line[512];
		snprintf(path, sizeof(path), "/proc/self/fdinfo/%s", ent->d_name);
		FILE* f = fopen(path, "r");
		if (f == NULL)
			continue;
		while (fgets(line, sizeof(line), f) != NULL)
		{
			if (strncmp(line, "inotify wd:", 11) == 0)
				count++;
		}
		fclose(f);
	}
	closedir(d);
	return count;
}

static void run_for(double seconds)
{
	CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + seconds;
	while (CFAbsoluteTimeGetCurrent() < deadline)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.05, true);
}

static void make_path(char* out, const char* name)
{
	snprintf(out, PATH_MAX, "%s/%s", g_dir, name);
}

// Whether an event for `name` (relative to the watched directory) carried
// all of `flags`. Flags of coalesced events for one path are combined.
static bool has_event(const char* name, FSEventStreamEventFlags flags)
{
	char path[PATH_MAX];
	make_path(path, name);

	for (size_t i = 0; i < g_eventCount; i++)
	{
		if (strcmp(g_events[i].path, path) == 0 && (g_events[i].flags & flags) == flags)
			return true;
	}
	return false;
}

static int count_created_files(void)
{
	int found = 0;
	for (int i = 0; i < FILE_COUNT; i++)
	{
		char name[32];
		snprintf(name, sizeof(name), "file%02d", i);
		if (has_event(name, kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemIsFile))
			found++;
	}
	return found;
}

static bool all_files_created(void* ctx)
{
	(void) ctx;
	return count_created_files() == FILE_COUNT;
}

static bool event_seen(void* ctx)
{
	const char** spec = ctx;
	return has_event(spec[0], (FSEventStreamEventFlags) (uintptr_t) spec[1]);
}

static bool both_seen(void* ctx)
{
	const char** spec = ctx;
	return has_event(spec[0], (FSEventStreamEventFlags) (uintptr_t) spec[1])
		&& has_event(spec[2], (FSEventStreamEventFlags) (uintptr_t) spec[3]);
}

// Runs the run loop until `done` returns true or WAIT_SECONDS pass.
static bool wait_for(bool (*done)(void*), void* ctx)
{
	CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + WAIT_SECONDS;
	while (!done(ctx))
	{
		if (CFAbsoluteTimeGetCurrent() > deadline)
			return false;
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
	}
	return true;
}

static void create_file(const char* name)
{
	char path[PATH_MAX];
	make_path(path, name);
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (fd < 0)
	{
		printf("FAIL: cannot create %s\n", path);
		finish(1);
	}
	close(fd);
}

static void touch_file(const char* name)
{
	char path[PATH_MAX];
	make_path(path, name);
	int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0)
		close(fd);
	unlink(path);
}

static FSEventStreamRef create_lifecycle_stream(void)
{
	FSEventStreamContext context = { 0, NULL, context_retain, context_release, NULL };
	CFStringRef dir = CFStringCreateWithFileSystemRepresentation(NULL, g_dir);
	CFArrayRef paths = CFArrayCreate(NULL, (const void**) &dir, 1, &kCFTypeArrayCallBacks);
	FSEventStreamRef stream = FSEventStreamCreate(NULL, lifecycle_callback, &context, paths,
		kFSEventStreamEventIdSinceNow, 0.05, kFSEventStreamCreateFlagFileEvents);
	CFRelease(paths);
	CFRelease(dir);
	return stream;
}

static void check_stream_lifecycle(dispatch_queue_t queue)
{
	const int baseFds = count_fds();
	const int baseWatches = count_inotify_watches();
	int created = 0;

	__atomic_store_n(&g_contextReleases, 0, __ATOMIC_SEQ_CST);

	for (int i = 0; i < LIFECYCLE_COUNT; i++)
	{
		FSEventStreamRef stream = create_lifecycle_stream();
		if (stream == NULL)
		{
			printf("FAIL: FSEventStreamCreate returned NULL for stream %d\n", i);
			g_failures++;
			break;
		}
		created++;

		if (i % 2 == 0)
			FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		else
			FSEventStreamSetDispatchQueue(stream, queue);

		FSEventStreamStart(stream);
		touch_file("lifecycle");
		FSEventStreamStop(stream);
		FSEventStreamStart(stream);
		FSEventStreamStop(stream);
		FSEventStreamInvalidate(stream);
		FSEventStreamRelease(stream);
	}

	// A delivery that is still in flight may keep a stream alive briefly.
	CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + WAIT_SECONDS;
	while ((__atomic_load_n(&g_contextReleases, __ATOMIC_SEQ_CST) != created || count_fds() != baseFds)
		&& CFAbsoluteTimeGetCurrent() < deadline)
	{
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.05, true);
	}

	const int releases = __atomic_load_n(&g_contextReleases, __ATOMIC_SEQ_CST);
	const int fds = count_fds();
	const int watches = count_inotify_watches();

	printf("lifecycle: %d streams, %d context releases, descriptors %d -> %d\n",
		created, releases, baseFds, fds);
	CHECK(releases == LIFECYCLE_COUNT);
	CHECK(fds == baseFds);

	if (baseWatches < 0)
	{
		printf("lifecycle: inotify watches not visible here, not checked\n");
	}
	else
	{
		printf("lifecycle: inotify watches %d -> %d\n", baseWatches, watches);
		CHECK(watches == baseWatches);
	}
}

// QUIRK (macOS 27.0.1): a stream released while it is started stays alive
// and keeps calling back. This stream is never freed.
static void check_quirk_started_keeps_stream(void)
{
	__atomic_store_n(&g_contextReleases, 0, __ATOMIC_SEQ_CST);
	__atomic_store_n(&g_lifecycleCallbacks, 0, __ATOMIC_SEQ_CST);

	FSEventStreamRef stream = create_lifecycle_stream();
	CHECK(stream != NULL);
	if (stream == NULL)
		return;

	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	FSEventStreamStart(stream);
	FSEventStreamRelease(stream);

	touch_file("quirk-started");
	CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + WAIT_SECONDS;
	while (__atomic_load_n(&g_lifecycleCallbacks, __ATOMIC_SEQ_CST) == 0 && CFAbsoluteTimeGetCurrent() < deadline)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.05, true);

	const int releases = __atomic_load_n(&g_contextReleases, __ATOMIC_SEQ_CST);
	const int callbacks = __atomic_load_n(&g_lifecycleCallbacks, __ATOMIC_SEQ_CST);
	printf("released while started: %d context releases, %d callbacks\n", releases, callbacks);
	CHECK(releases == 0);
	CHECK(callbacks > 0);
}

// QUIRK (macOS 27.0.1): a stream with a dispatch queue that is stopped and
// released without FSEventStreamInvalidate() stays alive. This stream is
// never freed.
static void check_quirk_queue_keeps_stream(dispatch_queue_t queue)
{
	__atomic_store_n(&g_contextReleases, 0, __ATOMIC_SEQ_CST);

	FSEventStreamRef stream = create_lifecycle_stream();
	CHECK(stream != NULL);
	if (stream == NULL)
		return;

	FSEventStreamSetDispatchQueue(stream, queue);
	FSEventStreamStart(stream);
	FSEventStreamStop(stream);
	FSEventStreamRelease(stream);
	run_for(0.5);

	const int releases = __atomic_load_n(&g_contextReleases, __ATOMIC_SEQ_CST);
	printf("released with a dispatch queue: %d context releases\n", releases);
	CHECK(releases == 0);
}

static void remove_tree(void)
{
	char path[PATH_MAX];
	for (int i = 0; i < FILE_COUNT; i++)
	{
		char name[32];
		snprintf(name, sizeof(name), "file%02d", i);
		make_path(path, name);
		unlink(path);
	}
	make_path(path, "renamed-new");
	unlink(path);
	make_path(path, "sub/inner");
	unlink(path);
	make_path(path, "sub");
	rmdir(path);
	rmdir(g_dir);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(120);

	char tmpl[] = "/tmp/fsevents_file_events.XXXXXX";
	if (mkdtemp(tmpl) == NULL || realpath(tmpl, g_dir) == NULL)
	{
		printf("FAIL: cannot create a temporary directory\n");
		finish(1);
	}

	CFStringRef dir = CFStringCreateWithFileSystemRepresentation(NULL, g_dir);
	CFArrayRef paths = CFArrayCreate(NULL, (const void**) &dir, 1, &kCFTypeArrayCallBacks);
	FSEventStreamRef stream = FSEventStreamCreate(NULL, callback, NULL, paths, kFSEventStreamEventIdSinceNow,
		0.2, kFSEventStreamCreateFlagFileEvents);
	CFRelease(paths);
	CFRelease(dir);
	if (stream == NULL)
	{
		printf("FAIL: FSEventStreamCreate returned NULL\n");
		remove_tree();
		finish(1);
	}

	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CHECK(FSEventStreamStart(stream));

	// Many files at once: inotify returns them packed into few reads.
	for (int i = 0; i < FILE_COUNT; i++)
	{
		char name[32];
		snprintf(name, sizeof(name), "file%02d", i);
		create_file(name);
	}
	wait_for(all_files_created, NULL);
	int created = count_created_files();
	printf("%d of %d created files reported\n", created, FILE_COUNT);
	CHECK(created == FILE_COUNT);

	// A rename: the old and the new path are both reported.
	const char* renameFrom[] = { "renamed-old", (const char*) (uintptr_t) kFSEventStreamEventFlagItemCreated };
	create_file("renamed-old");
	CHECK(wait_for(event_seen, renameFrom));
	char oldPath[PATH_MAX], newPath[PATH_MAX];
	make_path(oldPath, "renamed-old");
	make_path(newPath, "renamed-new");
	CHECK(rename(oldPath, newPath) == 0);
	const char* renamePair[] = {
		"renamed-old", (const char*) (uintptr_t) kFSEventStreamEventFlagItemRenamed,
		"renamed-new", (const char*) (uintptr_t) kFSEventStreamEventFlagItemRenamed,
	};
	bool renameSeen = wait_for(both_seen, renamePair);
	printf("rename: old path %s, new path %s\n",
		has_event("renamed-old", kFSEventStreamEventFlagItemRenamed) ? "reported" : "missing",
		has_event("renamed-new", kFSEventStreamEventFlagItemRenamed) ? "reported" : "missing");
	CHECK(renameSeen);

	// A subdirectory created after the stream started, and a file in it.
	char subPath[PATH_MAX];
	make_path(subPath, "sub");
	CHECK(mkdir(subPath, 0755) == 0);
	create_file("sub/inner");
	const char* subPair[] = {
		"sub", (const char*) (uintptr_t) (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemIsDir),
		"sub/inner", (const char*) (uintptr_t) (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemIsFile),
	};
	bool subSeen = wait_for(both_seen, subPair);
	printf("new subdirectory: directory %s, file in it %s\n",
		has_event("sub", kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemIsDir) ? "reported" : "missing",
		has_event("sub/inner", kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemIsFile) ? "reported" : "missing");
	CHECK(subSeen);

	CHECK(g_idsIncrease);
	CHECK(!g_emptyCallback);

	FSEventStreamStop(stream);
	FSEventStreamInvalidate(stream);
	FSEventStreamRelease(stream);

	dispatch_queue_t queue = dispatch_queue_create("fsevents test", DISPATCH_QUEUE_SERIAL);
	check_stream_lifecycle(queue);
	check_quirk_queue_keeps_stream(queue);
	check_quirk_started_keeps_stream();

	remove_tree();

	printf("%s\n", g_failures == 0 ? "PASS" : "FAIL");
	finish(g_failures == 0 ? 0 : 1);
}
