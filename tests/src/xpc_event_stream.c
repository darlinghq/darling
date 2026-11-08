// Copyright (C) 2026 Lubos Dolezel
//
// A job that lists an event under LaunchEvents in its launchd.plist receives
// that event with xpc_set_event_stream_handler() (xpc/xpc.h): launchd starts
// the job when the event arrives, and the handler gets a dictionary whose
// XPC_EVENT_KEY_NAME is the name of the LaunchEvents entry.
//
// The stream is com.apple.notifyd.matching with a {Notification: name}
// descriptor: the job is started when the notification is posted.
//
// Darling's launchd serves event streams through the one job marked
// EventMonitor. If no other job holds that role, the test loads its own
// provider job (this program, "provider" role) that serves the stream the
// way the notifyd.matching provider does: it checks in with launchd for the
// stream's events, registers each descriptor's notification with notify(3),
// and when one is posted, sends the event to the job's event channel. If
// another job is the event monitor, the test relies on it to serve the
// stream.
//
// The job (the "consumer" role) checks the event's name and payload, and
// that xpc_copy_event() returns its descriptor.
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <xpc/launchd_defs.h>
#include <dispatch/dispatch.h>
#include <mach-o/dyld.h>
#include <servers/bootstrap.h>
#include <notify.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

#define STREAM "com.apple.notifyd.matching"
#define EVENT_NAME "test-event"
#define NOTIFICATION_KEY "Notification"

static char g_dir[] = "/private/tmp/xpcevents.XXXXXX";
static char g_label_provider[128];
static char g_label_consumer[128];
static char g_notification[128];
static bool g_provider_loaded, g_consumer_loaded;

//
// helpers
//

static void path_in(char* out, size_t size, const char* dir, const char* name)
{
	snprintf(out, size, "%s/%s", dir, name);
}

static void write_file(const char* dir, const char* name, const char* content)
{
	char path[PATH_MAX], tmp[PATH_MAX];
	path_in(path, sizeof(path), dir, name);
	snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	FILE* f = fopen(tmp, "w");
	if (f == NULL)
	{
		// a job's stderr goes to its StandardErrorPath in the test directory
		fprintf(stderr, "cannot write %s: %s\n", tmp, strerror(errno));
		return;
	}
	fputs(content, f);
	fclose(f);
	// launchctl refuses group- or world-writable plists, so the mode must
	// not depend on the caller's umask.
	chmod(tmp, 0644);
	rename(tmp, path);
}

static bool read_file(const char* dir, const char* name, char* out, size_t size)
{
	char path[PATH_MAX];
	path_in(path, sizeof(path), dir, name);
	FILE* f = fopen(path, "r");
	if (f == NULL)
		return false;
	size_t n = fread(out, 1, size - 1, f);
	out[n] = '\0';
	fclose(f);
	return true;
}

static bool file_exists(const char* dir, const char* name)
{
	char path[PATH_MAX];
	struct stat st;
	path_in(path, sizeof(path), dir, name);
	return stat(path, &st) == 0;
}

// Waits up to `seconds` for one of the files to appear; returns its index.
static int wait_for(const char* const* names, int count, int seconds)
{
	for (int i = 0; i < seconds * 10; i++)
	{
		for (int n = 0; n < count; n++)
		{
			if (file_exists(g_dir, names[n]))
				return n;
		}
		usleep(100000);
	}
	return -1;
}

static int run_launchctl(const char* verb, const char* arg)
{
	const char* argv[] = { "/bin/launchctl", verb, arg, NULL };
	pid_t pid;
	if (posix_spawn(&pid, argv[0], NULL, NULL, (char* const*) argv, environ) != 0)
		return -1;
	int status;
	if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status))
		return -1;
	return WEXITSTATUS(status);
}

// The same, with launchctl's output discarded (for removing jobs that may
// not exist).
static int run_launchctl_quiet(const char* verb, const char* arg)
{
	const char* argv[] = { "/bin/launchctl", verb, arg, NULL };
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_addopen(&actions, 1, "/dev/null", O_WRONLY, 0);
	posix_spawn_file_actions_addopen(&actions, 2, "/dev/null", O_WRONLY, 0);
	pid_t pid;
	int status = -1;
	if (posix_spawn(&pid, argv[0], &actions, NULL, (char* const*) argv, environ) == 0)
	{
		if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status))
			status = -1;
		else
			status = WEXITSTATUS(status);
	}
	posix_spawn_file_actions_destroy(&actions);
	return status;
}

// Removes a job and waits until launchd has dropped its label. Removing a
// running job only stops it; the label stays until the job has exited, and
// loading a job with the same label before that is refused.
static void remove_job(const char* label)
{
	run_launchctl_quiet("remove", label);
	for (int i = 0; i < 100 && run_launchctl_quiet("list", label) == 0; i++)
		usleep(100000);
}

//
// provider role
//

static xpc_pipe_t g_pipe;
static const char* g_pdir;

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

static void provider_fire(uint64_t token, const char* notification)
{
	char status[256];
	write_file(g_pdir, "provider-notified", notification);

	// The event's name, as launchd knows it (XPC_EVENT_GET_NAME).
	event_name_t name;
	int err = xpc_get_event_name(STREAM, token, name);
	if (err != 0)
	{
		snprintf(status, sizeof(status), "xpc_get_event_name failed: %d\n", err);
		write_file(g_pdir, "provider-error", status);
		return;
	}

	// The job's event channel (XPC_EVENT_LOOK_UP).
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_STREAM, STREAM);
	xpc_dictionary_set_uint64(msg, XPC_EVENT_ROUTINE_KEY_TOKEN, token);
	xpc_object_t reply = NULL;
	err = launchd_routine(XPC_EVENT_LOOK_UP, msg, &reply);
	xpc_release(msg);
	if (err != 0)
	{
		snprintf(status, sizeof(status), "XPC_EVENT_LOOK_UP failed: %d\n", err);
		write_file(g_pdir, "provider-error", status);
		return;
	}
	mach_port_t port = xpc_dictionary_copy_mach_send(reply, XPC_EVENT_ROUTINE_KEY_PORT);
	xpc_release(reply);

	xpc_pipe_t channel = xpc_pipe_create_from_port(port, 0);
	mach_port_deallocate(mach_task_self(), port);
	if (channel == NULL)
	{
		write_file(g_pdir, "provider-error", "cannot open the event channel\n");
		return;
	}

	xpc_object_t event = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(event, XPC_EVENT_KEY_NAME, name);
	xpc_dictionary_set_string(event, NOTIFICATION_KEY, notification);
	err = xpc_pipe_simpleroutine(channel, event);
	xpc_release(event);
	xpc_release(channel);

	if (err != 0)
	{
		snprintf(status, sizeof(status), "sending the event failed: %d\n", err);
		write_file(g_pdir, "provider-error", status);
		return;
	}
	write_file(g_pdir, "provider-fired", name);
}

struct registered
{
	uint64_t token;
	struct registered* next;
};
static struct registered* g_registered;

static void provider_check_in(void)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_STREAM, STREAM);
	xpc_object_t reply = NULL;
	int err = launchd_routine(XPC_EVENT_PROVIDER_CHECK_IN, msg, &reply);
	xpc_release(msg);

	if (err == EPERM)
	{
		// Another job is the event monitor.
		write_file(g_pdir, "provider-eperm", "");
		fflush(stdout);
		_exit(0);
	}
	if (err != 0)
	{
		char status[64];
		snprintf(status, sizeof(status), "check-in failed: %d\n", err);
		write_file(g_pdir, "provider-error", status);
		fflush(stdout);
		_exit(1);
	}
	write_file(g_pdir, "provider-checked-in", "");

	xpc_object_t events = xpc_dictionary_get_value(reply, XPC_EVENT_ROUTINE_KEY_EVENTS);
	size_t count = events ? xpc_array_get_count(events) : 0;
	for (size_t i = 0; i + 1 < count; i += 2)
	{
		uint64_t token = xpc_array_get_uint64(events, i);
		xpc_object_t desc = xpc_array_get_value(events, i + 1);
		const char* notification = xpc_dictionary_get_string(desc, NOTIFICATION_KEY);
		if (notification == NULL)
			continue;

		bool known = false;
		for (struct registered* r = g_registered; r != NULL; r = r->next)
			known |= (r->token == token);
		if (known)
			continue;

		struct registered* r = calloc(1, sizeof(*r));
		r->token = token;
		r->next = g_registered;
		g_registered = r;

		char* note = strdup(notification);
		int t;
		notify_register_dispatch(note, &t, dispatch_get_main_queue(), ^(int unused) {
			(void) unused;
			provider_fire(token, note);
		});
		write_file(g_pdir, "provider-ready", note);
	}
	xpc_release(reply);
}

static void provider_main(const char* dir)
{
	g_pdir = dir;
	write_file(g_pdir, "provider-started", "");
	// Removed with launchctl remove; this is a safety net.
	alarm(120);

	g_pipe = xpc_pipe_create_from_port(bootstrap_port, 0);

	// launchd sends SIGUSR1 once we have checked in, when the stream's
	// events change.
	signal(SIGUSR1, SIG_IGN);
	dispatch_source_t sig = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGUSR1, 0, dispatch_get_main_queue());
	dispatch_source_set_event_handler(sig, ^{
		write_file(g_pdir, "provider-signaled", "");
		provider_check_in();
	});
	dispatch_resume(sig);

	dispatch_async(dispatch_get_main_queue(), ^{
		provider_check_in();
	});
	dispatch_main();
}

//
// consumer role
//

static void consumer_main(const char* dir)
{
	alarm(60);

	char* cdir = strdup(dir);
	xpc_set_event_stream_handler(STREAM, dispatch_get_main_queue(), ^(xpc_object_t event) {
		char result[1024];
		const char* name = xpc_dictionary_get_string(event, XPC_EVENT_KEY_NAME);
		const char* notification = xpc_dictionary_get_string(event, NOTIFICATION_KEY);

		// The descriptor from the launchd.plist, by name and as part of
		// all of the stream's events.
		const char* copied = NULL;
		xpc_object_t desc = name ? xpc_copy_event(STREAM, name) : NULL;
		if (desc != NULL)
			copied = xpc_dictionary_get_string(desc, NOTIFICATION_KEY);
		xpc_object_t all = xpc_copy_event(STREAM, NULL);
		bool in_all = all != NULL && xpc_get_type(all) == XPC_TYPE_DICTIONARY
			&& name != NULL && xpc_dictionary_get_value(all, name) != NULL;

		snprintf(result, sizeof(result), "name=%s\nnotification=%s\ncopy=%s\nall=%s\n",
			name ? name : "(none)", notification ? notification : "(none)",
			copied ? copied : "(none)", in_all ? "yes" : "no");
		write_file(cdir, "consumer-result", result);

		fflush(stdout);
		_exit(0);
	});
	dispatch_main();
}

//
// test driver
//

static void cleanup(void)
{
	if (g_consumer_loaded)
		run_launchctl("remove", g_label_consumer);
	if (g_provider_loaded)
		run_launchctl("remove", g_label_provider);

	static const char* const files[] = {
		"provider.plist", "consumer.plist", "provider-stderr", "provider-started", "provider-checked-in", "provider-eperm", "provider-signaled",
		"provider-ready", "provider-notified", "provider-fired", "provider-error", "consumer-result",
	};
	for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++)
	{
		char path[PATH_MAX];
		path_in(path, sizeof(path), g_dir, files[i]);
		unlink(path);
	}
	rmdir(g_dir);
}

static void finish(int status)
{
	cleanup();
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static void write_plist(const char* file, const char* label, const char* self, const char* role, const char* extra)
{
	char content[4096];
	snprintf(content, sizeof(content),
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n"
		"\t<key>Label</key>\n\t<string>%s</string>\n"
		"\t<key>ProgramArguments</key>\n\t<array>\n"
		"\t\t<string>%s</string>\n\t\t<string>%s</string>\n\t\t<string>%s</string>\n"
		"\t</array>\n%s</dict>\n</plist>\n",
		label, self, role, g_dir, extra);
	write_file(g_dir, file, content);
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "provider") == 0)
		provider_main(argv[2]);
	if (argc == 3 && strcmp(argv[1], "consumer") == 0)
		consumer_main(argv[2]);

	signal(SIGALRM, on_alarm);
	alarm(90);

	if (mkdtemp(g_dir) == NULL)
	{
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		fflush(stdout);
		_exit(1);
	}

	char exe[PATH_MAX], self[PATH_MAX];
	uint32_t size = sizeof(exe);
	if (_NSGetExecutablePath(exe, &size) != 0 || realpath(exe, self) == NULL)
	{
		printf("FAIL: cannot find this program's path\n");
		finish(1);
	}

	int pid = (int) getpid();
	// Fixed labels: jobs left behind by an interrupted run (a stale event
	// monitor, a consumer whose event the provider would also see) are
	// removed first.
	snprintf(g_label_provider, sizeof(g_label_provider), "org.darlinghq.tests.xpc-event-stream.provider");
	snprintf(g_label_consumer, sizeof(g_label_consumer), "org.darlinghq.tests.xpc-event-stream.consumer");
	snprintf(g_notification, sizeof(g_notification), "org.darlinghq.tests.xpc-event-stream.%d", pid);
	remove_job(g_label_consumer);
	remove_job(g_label_provider);

	char path[PATH_MAX], extra[1024];
	int failures = 0;

	// The provider, unless another job is the event monitor.
	char provider_extra[1024];
	snprintf(provider_extra, sizeof(provider_extra),
		"\t<key>EventMonitor</key>\n\t<true/>\n\t<key>RunAtLoad</key>\n\t<true/>\n"
		"\t<key>StandardErrorPath</key>\n\t<string>%s/provider-stderr</string>\n", g_dir);
	write_plist("provider.plist", g_label_provider, self, "provider", provider_extra);
	path_in(path, sizeof(path), g_dir, "provider.plist");
	int rc = run_launchctl("load", path);
	if (rc != 0)
	{
		printf("FAIL: launchctl load %s exited with %d\n", path, rc);
		finish(1);
	}
	g_provider_loaded = true;

	static const char* const provider_states[] = { "provider-checked-in", "provider-eperm", "provider-error" };
	int state = wait_for(provider_states, 3, 15);
	bool own_provider = (state == 0);
	if (state == 1)
	{
		printf("note: another job is the event monitor; it has to serve %s\n", STREAM);
		run_launchctl("remove", g_label_provider);
		g_provider_loaded = false;
	}
	else if (state != 0)
	{
		char err[256];
		snprintf(err, sizeof(err), "no check-in within 15 s (%s)\n", file_exists(g_dir, "provider-started")
			? "it started; launchd did not answer its check-in" : "launchd did not start it");
		read_file(g_dir, "provider-error", err, sizeof(err));
		printf("FAIL: the provider could not check in with launchd: %s", err);
		// launchd creates StandardErrorPath in its child before it executes
		// the program: a missing file means the child failed before that.
		char stderr_path[PATH_MAX], output[2048];
		struct stat st;
		path_in(stderr_path, sizeof(stderr_path), g_dir, "provider-stderr");
		if (stat(stderr_path, &st) != 0)
			printf("provider stderr: no %s (%s)\n", stderr_path, strerror(errno));
		else
		{
			printf("provider stderr: %lld bytes\n", (long long) st.st_size);
			if (read_file(g_dir, "provider-stderr", output, sizeof(output)) && output[0])
				printf("%s\n", output);
		}
		fflush(stdout);
		run_launchctl("list", g_label_provider);
		finish(1);
	}
	else
		printf("ok: the test provider checked in for %s\n", STREAM);

	// The job, started only by its event.
	snprintf(extra, sizeof(extra),
		"\t<key>LaunchEvents</key>\n\t<dict>\n"
		"\t\t<key>%s</key>\n\t\t<dict>\n"
		"\t\t\t<key>%s</key>\n\t\t\t<dict>\n"
		"\t\t\t\t<key>%s</key>\n\t\t\t\t<string>%s</string>\n"
		"\t\t\t</dict>\n\t\t</dict>\n\t</dict>\n",
		STREAM, EVENT_NAME, NOTIFICATION_KEY, g_notification);
	write_plist("consumer.plist", g_label_consumer, self, "consumer", extra);
	path_in(path, sizeof(path), g_dir, "consumer.plist");
	rc = run_launchctl("load", path);
	if (rc != 0)
	{
		printf("FAIL: launchctl load %s exited with %d\n", path, rc);
		finish(1);
	}
	g_consumer_loaded = true;

	if (own_provider)
	{
		static const char* const ready[] = { "provider-ready", "provider-error" };
		if (wait_for(ready, 2, 15) != 0)
		{
			char err[256];
			if (read_file(g_dir, "provider-error", err, sizeof(err)))
				printf("FAIL: the provider could not check in again: %s", err);
			else if (!file_exists(g_dir, "provider-signaled"))
				printf("FAIL: the provider did not get the job's event: launchd's SIGUSR1 never reached it\n");
			else
				printf("FAIL: the provider did not get the job's event: it was signaled, but its check-in returned no event\n");
			finish(1);
		}
		printf("ok: the provider got the job's event\n");
	}
	else
		sleep(3);

	// Nothing may start the job before its event.
	sleep(1);
	if (file_exists(g_dir, "consumer-result"))
	{
		printf("FAIL: the job ran before its event was posted\n");
		failures++;
	}

	uint32_t status = notify_post(g_notification);
	if (status != NOTIFY_STATUS_OK)
	{
		printf("FAIL: notify_post returned %u\n", status);
		finish(1);
	}

	static const char* const done[] = { "consumer-result" };
	if (wait_for(done, 1, 20) != 0)
	{
		char err[256];
		if (own_provider && read_file(g_dir, "provider-error", err, sizeof(err)))
			printf("FAIL: the provider could not deliver the event: %s", err);
		else if (own_provider && !file_exists(g_dir, "provider-notified"))
			printf("FAIL: the provider never saw the notification being posted (notifyd)\n");
		else if (own_provider && !file_exists(g_dir, "provider-fired"))
			printf("FAIL: the provider got no answer from launchd while delivering the event (XPC_EVENT_GET_NAME or XPC_EVENT_LOOK_UP)\n");
		else
			printf("FAIL: the job did not receive the event within 20 s\n");
		finish(1);
	}

	char result[1024], expected[1024];
	read_file(g_dir, "consumer-result", result, sizeof(result));
	snprintf(expected, sizeof(expected), "name=%s\nnotification=%s\ncopy=%s\nall=yes\n",
		EVENT_NAME, g_notification, g_notification);
	if (strcmp(result, expected) != 0)
	{
		printf("FAIL: the job received\n%sexpected\n%s", result, expected);
		failures++;
	}
	else
		printf("ok: the job was started and received %s/%s with its payload; xpc_copy_event returned its descriptor\n",
			STREAM, EVENT_NAME);

	printf("%s\n", failures ? "FAIL" : "PASS");
	finish(failures ? 1 : 0);
}
