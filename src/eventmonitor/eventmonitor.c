/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// launchd's event monitor.
//
// launchctl turns the WatchPaths, QueueDirectories and KeepAlive PathState
// keys of a job into LaunchEvents of the "com.apple.fsevents.matching"
// stream. launchd keeps a boolean state for every such event and starts the
// job while the state is true, but it does not watch the file system
// itself: it hands the events to the one job that has EventMonitor set and
// lets it set the states.
//
// This daemon is that job. It checks in for the stream with launchd,
// watches the parent directories of the listed paths with FSEvents, and on
// every change re-examines the paths:
//
//  - WatchPaths: the job is started when a path is created, modified,
//    removed or renamed. The state is raised and lowered again, so the job
//    is started once per change.
//  - QueueDirectories: the state is true while one of the directories is
//    not empty, so launchd keeps the job running until it has emptied them.
//  - PathState: the state is true while a path exists (true) or does not
//    exist (false), as launchd.plist(5) describes for KeepAlive.
//
// launchd sends SIGUSR1 when events are added or removed; the daemon then
// checks in again and gets the full list.

#include <CoreFoundation/CoreFoundation.h>
#include <FSEvents/FSEvents.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <dispatch/dispatch.h>
#include <servers/bootstrap.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define FSEVENTS_STREAM "com.apple.fsevents.matching"

#define KEY_WATCHPATHS "WatchPaths"
#define KEY_QUEUEDIRECTORIES "QueueDirectories"
#define KEY_PATHSTATE "PathState"

struct path_snapshot {
	bool exists;
	dev_t dev;
	ino_t ino;
	mode_t mode;
	nlink_t nlink;
	off_t size;
	struct timespec mtime;
	struct timespec ctime;
};

struct watched_path {
	char* path;
	struct path_snapshot snap;
	// FSEvents reported an item event for exactly this path since the last
	// evaluation.
	bool reported;
};

struct path_state {
	char* path;
	bool want_exists;
};

struct fs_event {
	struct fs_event* next;
	uint64_t token;
	bool seen;

	struct watched_path* watch;
	size_t nwatch;
	char** queue;
	size_t nqueue;
	struct path_state* pstate;
	size_t npstate;

	// The level last sent to launchd, valid once state_sent is set.
	bool state_sent;
	bool state;
};

static dispatch_queue_t g_queue;
static xpc_pipe_t g_pipe;
static struct fs_event* g_events;

static FSEventStreamRef g_stream;
static CFArrayRef g_watched_dirs;
static bool g_evaluate_pending;

static void evaluate_all(void);

static int launchd_routine(uint64_t op, xpc_object_t msg, xpc_object_t* reply_out)
{
	xpc_object_t reply = NULL;

	xpc_dictionary_set_uint64(msg, XPC_EVENT_ROUTINE_KEY_OP, op);

	int err = xpc_pipe_routine(g_pipe, msg, &reply);
	if (err != 0)
		return err;

	err = (int) xpc_dictionary_get_uint64(reply, XPC_EVENT_ROUTINE_KEY_ERROR);
	if (err != 0 || reply_out == NULL)
		xpc_release(reply);
	else
		*reply_out = reply;
	return err;
}

static void send_state(struct fs_event* ev, bool state)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_STREAM, FSEVENTS_STREAM);
	xpc_dictionary_set_uint64(msg, XPC_EVENT_ROUTINE_KEY_TOKEN, ev->token);
	xpc_dictionary_set_bool(msg, XPC_EVENT_ROUTINE_KEY_STATE, state);

	int err = launchd_routine(XPC_EVENT_PROVIDER_SET_STATE, msg, NULL);
	xpc_release(msg);

	// ESRCH: the job went away; launchd signals us and the event is
	// dropped on the next check-in.
	if (err != 0 && err != ESRCH)
		syslog(LOG_ERR, "Setting the state of event %llu failed: %d", (unsigned long long) ev->token, err);
}

static void take_snapshot(const char* path, struct path_snapshot* snap)
{
	struct stat st;

	memset(snap, 0, sizeof(*snap));

	// A symlink is followed; a dangling one still exists as an entry.
	if (stat(path, &st) != 0 && lstat(path, &st) != 0)
		return;

	snap->exists = true;
	snap->dev = st.st_dev;
	snap->ino = st.st_ino;
	snap->mode = st.st_mode;
	snap->nlink = st.st_nlink;
	snap->size = st.st_size;
	snap->mtime = st.st_mtimespec;
	snap->ctime = st.st_ctimespec;
}

static bool snapshot_equal(const struct path_snapshot* a, const struct path_snapshot* b)
{
	if (a->exists != b->exists)
		return false;
	if (!a->exists)
		return true;
	return a->dev == b->dev && a->ino == b->ino && a->mode == b->mode
		&& a->nlink == b->nlink && a->size == b->size
		&& a->mtime.tv_sec == b->mtime.tv_sec && a->mtime.tv_nsec == b->mtime.tv_nsec
		&& a->ctime.tv_sec == b->ctime.tv_sec && a->ctime.tv_nsec == b->ctime.tv_nsec;
}

static bool directory_has_entries(const char* path)
{
	DIR* dir = opendir(path);
	if (dir == NULL)
		return false;

	bool found = false;
	struct dirent* ent;
	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
		{
			found = true;
			break;
		}
	}

	closedir(dir);
	return found;
}

static bool path_exists(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0;
}

// The level part of an event: true while a queue directory has entries or
// a PathState condition holds.
static bool event_level(const struct fs_event* ev)
{
	for (size_t i = 0; i < ev->nqueue; i++)
	{
		if (directory_has_entries(ev->queue[i]))
			return true;
	}
	for (size_t i = 0; i < ev->npstate; i++)
	{
		if (path_exists(ev->pstate[i].path) == ev->pstate[i].want_exists)
			return true;
	}
	return false;
}

// Re-examines the paths of one event and tells launchd if the job should
// be started.
static void evaluate_event(struct fs_event* ev)
{
	bool changed = false;

	for (size_t i = 0; i < ev->nwatch; i++)
	{
		struct path_snapshot snap;
		take_snapshot(ev->watch[i].path, &snap);
		// A rewrite that keeps the size within the file system's timestamp
		// granularity leaves the stat data unchanged, so an item event for
		// the path counts as a change too.
		if (ev->watch[i].reported || !snapshot_equal(&snap, &ev->watch[i].snap))
		{
			ev->watch[i].snap = snap;
			changed = true;
		}
		ev->watch[i].reported = false;
	}

	bool level = event_level(ev);

	if (!ev->state_sent || ev->state != level)
	{
		send_state(ev, level);
		ev->state_sent = true;
		ev->state = level;
	}
	else if (changed && !level)
	{
		// launchd starts the job (or schedules a throttled start) while
		// handling the true state, so lowering it right away starts the job
		// once for this change instead of keeping it alive.
		send_state(ev, true);
		send_state(ev, false);
	}
}

static bool is_directory(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

// Adds the nearest existing ancestor of `path` (the parent directory if it
// exists) to `dirs`. Watching the parent shows the path being created,
// removed, renamed or written to; once a missing parent appears, the next
// rebuild moves the watch down to it.
static void add_parent_dir(CFMutableSetRef dirs, const char* path)
{
	char buf[MAXPATHLEN];

	if (strlcpy(buf, path, sizeof(buf)) >= sizeof(buf))
		return;

	for (;;)
	{
		char* slash = strrchr(buf, '/');
		if (slash == NULL)
			return;
		if (slash == buf)
		{
			buf[1] = '\0';
			break;
		}
		*slash = '\0';
		if (is_directory(buf))
			break;
	}

	CFStringRef str = CFStringCreateWithFileSystemRepresentation(NULL, buf);
	if (str != NULL)
	{
		CFSetAddValue(dirs, str);
		CFRelease(str);
	}
}

static void add_dir(CFMutableSetRef dirs, const char* path)
{
	if (!is_directory(path))
		return;

	CFStringRef str = CFStringCreateWithFileSystemRepresentation(NULL, path);
	if (str != NULL)
	{
		CFSetAddValue(dirs, str);
		CFRelease(str);
	}
}

static CFComparisonResult compare_strings(const void* a, const void* b, void* context)
{
	return CFStringCompare((CFStringRef) a, (CFStringRef) b, 0);
}

static CFArrayRef copy_watch_dirs(void)
{
	CFMutableSetRef dirs = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	for (struct fs_event* ev = g_events; ev != NULL; ev = ev->next)
	{
		// A watched directory is modified when its entries change, which
		// only a watch on the directory itself reports.
		for (size_t i = 0; i < ev->nwatch; i++)
		{
			add_parent_dir(dirs, ev->watch[i].path);
			add_dir(dirs, ev->watch[i].path);
		}
		for (size_t i = 0; i < ev->nqueue; i++)
		{
			add_parent_dir(dirs, ev->queue[i]);
			add_dir(dirs, ev->queue[i]);
		}
		for (size_t i = 0; i < ev->npstate; i++)
			add_parent_dir(dirs, ev->pstate[i].path);
	}

	CFIndex count = CFSetGetCount(dirs);
	const void** values = calloc(count > 0 ? count : 1, sizeof(*values));
	CFSetGetValues(dirs, values);

	CFMutableArrayRef sorted = CFArrayCreateMutable(NULL, count, &kCFTypeArrayCallBacks);
	for (CFIndex i = 0; i < count; i++)
		CFArrayAppendValue(sorted, values[i]);
	CFArraySortValues(sorted, CFRangeMake(0, count), compare_strings, NULL);

	free(values);
	CFRelease(dirs);
	return sorted;
}

static void schedule_evaluate(void)
{
	if (g_evaluate_pending)
		return;
	g_evaluate_pending = true;
	dispatch_async(g_queue, ^{
		g_evaluate_pending = false;
		evaluate_all();
	});
}

// Compares two absolute paths, treating runs of slashes as one ("/" + "/x"
// is how an entry of the root directory is reported).
static bool same_path(const char* a, const char* b)
{
	for (;;)
	{
		if (*a == '/' && *b == '/')
		{
			while (*a == '/')
				a++;
			while (*b == '/')
				b++;
			continue;
		}
		if (*a != *b)
			return false;
		if (*a == '\0')
			return true;
		a++;
		b++;
	}
}

static void mark_reported(const char* path)
{
	for (struct fs_event* ev = g_events; ev != NULL; ev = ev->next)
	{
		for (size_t i = 0; i < ev->nwatch; i++)
		{
			if (same_path(ev->watch[i].path, path))
				ev->watch[i].reported = true;
		}
	}
}

static void stream_callback(ConstFSEventStreamRef stream, void* info, size_t count, void* paths,
	const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
	// Events from a stream that has already been replaced are ignored.
	if (stream != g_stream)
		return;

	// Every change is handled by looking at the paths again. Item events
	// that name a WatchPaths path also count as a change of that path,
	// because the stat data can stay the same across a rewrite.
	const char* const* names = (const char* const*) paths;
	for (size_t i = 0; i < count; i++)
	{
		if (names[i] != NULL)
			mark_reported(names[i]);
	}
	schedule_evaluate();
}

static void stop_stream(void)
{
	if (g_stream == NULL)
		return;
	FSEventStreamStop(g_stream);
	FSEventStreamInvalidate(g_stream);
	FSEventStreamRelease(g_stream);
	g_stream = NULL;
}

// Returns true if the set of watched directories changed.
static bool update_watches(void)
{
	CFArrayRef dirs = copy_watch_dirs();

	if (g_watched_dirs != NULL && CFEqual(dirs, g_watched_dirs))
	{
		CFRelease(dirs);
		return false;
	}

	stop_stream();
	if (g_watched_dirs != NULL)
		CFRelease(g_watched_dirs);
	g_watched_dirs = dirs;

	if (CFArrayGetCount(dirs) == 0)
		return true;

	g_stream = FSEventStreamCreate(NULL, stream_callback, NULL, dirs, kFSEventStreamEventIdSinceNow,
		0.1, kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents);
	if (g_stream == NULL)
	{
		syslog(LOG_ERR, "Cannot create an FSEvents stream");
		return true;
	}

	FSEventStreamSetDispatchQueue(g_stream, g_queue);
	FSEventStreamStart(g_stream);
	return true;
}

static void evaluate_all(void)
{
	// A new watch only reports what happens after it was added, so after
	// every rebuild the paths are examined again. The number of rounds is
	// bounded because a path tree can keep changing.
	for (int round = 0; round < 8; round++)
	{
		for (struct fs_event* ev = g_events; ev != NULL; ev = ev->next)
			evaluate_event(ev);
		if (!update_watches())
			break;
	}
}

static char* copy_path(xpc_object_t value)
{
	if (xpc_get_type(value) != XPC_TYPE_STRING)
		return NULL;

	const char* path = xpc_string_get_string_ptr(value);
	// launchd.plist(5) paths are absolute.
	if (path == NULL || path[0] != '/')
	{
		syslog(LOG_WARNING, "Ignoring path that is not absolute: %s", path ? path : "(null)");
		return NULL;
	}
	return strdup(path);
}

static char** copy_path_array(xpc_object_t array, size_t* count)
{
	*count = 0;
	if (array == NULL)
		return NULL;

	// A single string is accepted as a one-element list.
	if (xpc_get_type(array) == XPC_TYPE_STRING)
	{
		char* path = copy_path(array);
		if (path == NULL)
			return NULL;
		char** result = malloc(sizeof(char*));
		result[0] = path;
		*count = 1;
		return result;
	}

	if (xpc_get_type(array) != XPC_TYPE_ARRAY)
		return NULL;

	size_t n = xpc_array_get_count(array);
	char** result = calloc(n > 0 ? n : 1, sizeof(char*));
	for (size_t i = 0; i < n; i++)
	{
		char* path = copy_path(xpc_array_get_value(array, i));
		if (path != NULL)
			result[(*count)++] = path;
	}
	return result;
}

static struct fs_event* event_create(uint64_t token, xpc_object_t desc)
{
	struct fs_event* ev = calloc(1, sizeof(*ev));
	ev->token = token;

	size_t nwatch;
	char** watch = copy_path_array(xpc_dictionary_get_value(desc, KEY_WATCHPATHS), &nwatch);
	if (nwatch > 0)
	{
		ev->watch = calloc(nwatch, sizeof(*ev->watch));
		for (size_t i = 0; i < nwatch; i++)
		{
			ev->watch[i].path = watch[i];
			take_snapshot(watch[i], &ev->watch[i].snap);
		}
		ev->nwatch = nwatch;
	}
	free(watch);

	ev->queue = copy_path_array(xpc_dictionary_get_value(desc, KEY_QUEUEDIRECTORIES), &ev->nqueue);

	xpc_object_t pstate = xpc_dictionary_get_value(desc, KEY_PATHSTATE);
	if (pstate != NULL && xpc_get_type(pstate) == XPC_TYPE_DICTIONARY)
	{
		size_t n = xpc_dictionary_get_count(pstate);
		ev->pstate = calloc(n > 0 ? n : 1, sizeof(*ev->pstate));
		xpc_dictionary_apply(pstate, ^bool(const char* key, xpc_object_t value) {
			if (xpc_get_type(value) == XPC_TYPE_BOOL && key[0] == '/')
			{
				ev->pstate[ev->npstate].path = strdup(key);
				ev->pstate[ev->npstate].want_exists = xpc_bool_get_value(value);
				ev->npstate++;
			}
			return true;
		});
	}

	return ev;
}

static void event_free(struct fs_event* ev)
{
	for (size_t i = 0; i < ev->nwatch; i++)
		free(ev->watch[i].path);
	free(ev->watch);
	for (size_t i = 0; i < ev->nqueue; i++)
		free(ev->queue[i]);
	free(ev->queue);
	for (size_t i = 0; i < ev->npstate; i++)
		free(ev->pstate[i].path);
	free(ev->pstate);
	free(ev);
}

static struct fs_event* event_find(uint64_t token)
{
	for (struct fs_event* ev = g_events; ev != NULL; ev = ev->next)
	{
		if (ev->token == token)
			return ev;
	}
	return NULL;
}

// Gets the stream's events from launchd and brings the list up to date.
// launchd answers with an array of alternating tokens and descriptors.
//
// A WatchPaths job is started by changes made after it is watched, and the
// monitor watches a job from the moment it first learns of its token, which
// is here. So the baseline of every new WatchPaths event is taken while the
// reply is processed, before any state from this check-in is sent and
// before the watches are rebuilt; evaluate_all() then examines the paths
// again after each rebuild, so nothing between the baseline and the new
// watch is lost. launchd passes neither a load time nor the changes it saw,
// so a change made before this point cannot be told apart from one made
// before the job was loaded, and it is part of the baseline.
static void check_in(void)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_STREAM, FSEVENTS_STREAM);

	xpc_object_t reply = NULL;
	int err = launchd_routine(XPC_EVENT_PROVIDER_CHECK_IN, msg, &reply);
	xpc_release(msg);

	if (err == EPERM)
	{
		syslog(LOG_ERR, "launchd did not accept this job as its event monitor");
		exit(EXIT_SUCCESS);
	}
	if (err != 0)
	{
		// Without a check-in launchd does not signal changes either, so
		// exit and let launchd start the monitor again.
		syslog(LOG_ERR, "Checking in with launchd failed: %d", err);
		exit(EXIT_FAILURE);
	}

	for (struct fs_event* ev = g_events; ev != NULL; ev = ev->next)
		ev->seen = false;

	xpc_object_t events = xpc_dictionary_get_value(reply, XPC_EVENT_ROUTINE_KEY_EVENTS);
	if (events != NULL && xpc_get_type(events) == XPC_TYPE_ARRAY)
	{
		size_t count = xpc_array_get_count(events);
		for (size_t i = 0; i + 1 < count; i += 2)
		{
			xpc_object_t xtoken = xpc_array_get_value(events, i);
			xpc_object_t desc = xpc_array_get_value(events, i + 1);
			if (xpc_get_type(xtoken) != XPC_TYPE_UINT64 || xpc_get_type(desc) != XPC_TYPE_DICTIONARY)
				continue;

			uint64_t token = xpc_uint64_get_value(xtoken);
			struct fs_event* ev = event_find(token);
			if (ev == NULL)
			{
				ev = event_create(token, desc);
				ev->next = g_events;
				g_events = ev;
			}
			ev->seen = true;
		}
	}
	xpc_release(reply);

	struct fs_event** link = &g_events;
	while (*link != NULL)
	{
		struct fs_event* ev = *link;
		if (!ev->seen)
		{
			*link = ev->next;
			event_free(ev);
		}
		else
			link = &ev->next;
	}

	evaluate_all();
}

int main(int argc, const char** argv)
{
	openlog("eventmonitor", LOG_PID, LOG_DAEMON);

	g_queue = dispatch_queue_create("org.darlinghq.eventmonitor", DISPATCH_QUEUE_SERIAL);
	g_pipe = xpc_pipe_create_from_port(bootstrap_port, 0);
	if (g_pipe == NULL)
	{
		syslog(LOG_ERR, "Cannot create a pipe to launchd");
		return EXIT_FAILURE;
	}

	// launchd sends SIGUSR1 once we have checked in, whenever the events
	// change. It must be handled before the first check-in.
	signal(SIGUSR1, SIG_IGN);
	dispatch_source_t sig = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGUSR1, 0, g_queue);
	dispatch_source_set_event_handler(sig, ^{
		check_in();
	});
	dispatch_resume(sig);

	dispatch_async(g_queue, ^{
		check_in();
	});

	dispatch_main();
}
