// Copyright (C) 2026 Lubos Dolezel
//
// launchd.plist(5) WatchPaths, QueueDirectories and KeepAlive PathState.
// launchctl turns these keys into events of the com.apple.fsevents.matching
// stream, which launchd's event monitor job serves. Without a monitor the
// jobs are loaded but never started.
//
// The test loads three jobs with `launchctl load` and checks:
//  - WatchPaths: no start at load; one start each when the path is created,
//    written to and removed.
//  - QueueDirectories: no start while the directory is empty; started while
//    it has entries, until the job has emptied it.
//  - PathState: no start while the path is missing; kept running while it
//    exists; no more starts once it is removed.
//
// The jobs use ThrottleInterval 1 so that consecutive starts are not delayed
// by launchd's default ten-second respawn throttle.
//
// launchd tells its event monitor about a new job asynchronously (SIGUSR1,
// then the monitor checks in), and launchd.plist(5) only promises a start
// for modifications made after the job is watched. Before it touches the
// watched path, the test therefore loads a probe job with a PathState
// condition that already holds. The monitor learns of every event in one
// check-in and takes the WatchPaths baseline before it sets any state, so
// once the probe has started, the WatchPaths job is being watched.
// Before each further change the test also waits for the previous run of
// the job to exit, because launchd does not start a job that is running.
//
// If `launchctl load` is refused for the current user, run the test as root.
#include <errno.h>
#include <launch.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

static int g_failures = 0;
static char g_dir[] = "/private/tmp/watchpaths.XXXXXX";
static char g_labels[4][128];
static bool g_loaded[4];

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void path_in_dir(char* buf, size_t size, const char* name)
{
	snprintf(buf, size, "%s/%s", g_dir, name);
}

static int run_launchctl(const char* verb, const char* arg)
{
	const char* argv[] = { "launchctl", verb, arg, NULL };
	pid_t pid;
	int status;

	if (posix_spawn(&pid, "/bin/launchctl", NULL, NULL, (char* const*) argv, environ) != 0)
		return -1;
	if (waitpid(pid, &status, 0) != pid)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void remove_tree(void)
{
	static const char* const names[] = {
		"watched", "watch.log", "queue.log", "pstate.log", "flag",
		"watch.plist", "queue.plist", "pstate.plist",
		"probe", "probe.log", "probe.plist",
	};
	char path[256];

	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		path_in_dir(path, sizeof(path), names[i]);
		unlink(path);
	}
	// The queue job moves the items into done/.
	static const char* const items[] = { "queue/item1", "queue/item2", "queue/item3",
		"done/item1", "done/item2", "done/item3" };
	for (size_t i = 0; i < sizeof(items) / sizeof(items[0]); i++)
	{
		path_in_dir(path, sizeof(path), items[i]);
		unlink(path);
	}
	path_in_dir(path, sizeof(path), "queue");
	rmdir(path);
	path_in_dir(path, sizeof(path), "done");
	rmdir(path);
	rmdir(g_dir);
}

static void finish(int status)
{
	for (int i = 0; i < 4; i++)
	{
		if (g_loaded[i])
			run_launchctl("remove", g_labels[i]);
	}
	remove_tree();
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static int count_lines(const char* name)
{
	char path[256];
	path_in_dir(path, sizeof(path), name);

	FILE* f = fopen(path, "r");
	if (f == NULL)
		return 0;

	int lines = 0, c;
	while ((c = fgetc(f)) != EOF)
	{
		if (c == '\n')
			lines++;
	}
	fclose(f);
	return lines;
}

// Waits up to `ms` milliseconds for the log to reach `want` lines.
static int wait_lines(const char* name, int want, int ms)
{
	int lines = count_lines(name);
	for (int waited = 0; lines < want && waited < ms; waited += 100)
	{
		usleep(100 * 1000);
		lines = count_lines(name);
	}
	return lines;
}

static long now_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000L + tv.tv_usec / 1000;
}

// Asks launchd whether the job has a running process.
static bool job_running(const char* label)
{
	launch_data_t msg = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(msg, launch_data_new_string(label), LAUNCH_KEY_GETJOB);

	launch_data_t resp = launch_msg(msg);
	launch_data_free(msg);
	if (resp == NULL)
		return false;

	bool running = launch_data_get_type(resp) == LAUNCH_DATA_DICTIONARY
		&& launch_data_dict_lookup(resp, LAUNCH_JOBKEY_PID) != NULL;
	launch_data_free(resp);
	return running;
}

// Waits up to `ms` milliseconds for the job's process to exit.
static bool wait_job_exited(const char* label, int ms)
{
	for (int waited = 0; job_running(label); waited += 50)
	{
		if (waited >= ms)
			return false;
		usleep(50 * 1000);
	}
	return true;
}

static bool exists(const char* name)
{
	char path[256];
	struct stat st;
	path_in_dir(path, sizeof(path), name);
	return stat(path, &st) == 0;
}

static void touch(const char* name, const char* text)
{
	char path[256];
	path_in_dir(path, sizeof(path), name);

	int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd < 0)
	{
		printf("FAIL: creating %s: %s\n", path, strerror(errno));
		finish(1);
	}
	if (text != NULL)
		write(fd, text, strlen(text));
	close(fd);
}

static void remove_file(const char* name)
{
	char path[256];
	path_in_dir(path, sizeof(path), name);
	unlink(path);
}

// Writes a job plist; `keys` is inserted into the top-level dictionary.
static void write_plist(const char* file, const char* label, const char* script, const char* keys)
{
	char path[256];
	path_in_dir(path, sizeof(path), file);

	FILE* f = fopen(path, "w");
	if (f == NULL)
	{
		printf("FAIL: writing %s: %s\n", path, strerror(errno));
		finish(1);
	}
	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n"
		"\t<key>Label</key><string>%s</string>\n"
		"\t<key>ProgramArguments</key><array><string>/bin/sh</string><string>-c</string><string>%s</string></array>\n"
		"\t<key>ThrottleInterval</key><integer>1</integer>\n"
		"%s"
		"</dict>\n</plist>\n", label, script, keys);
	fclose(f);
}

static void load(int index, const char* file)
{
	char path[256];
	path_in_dir(path, sizeof(path), file);

	int status = run_launchctl("load", path);
	if (status != 0)
	{
		printf("FAIL: launchctl load %s exited with %d (run the test as root)\n", path, status);
		finish(1);
	}
	g_loaded[index] = true;
}

// Returns once the event monitor has checked in after everything loaded so
// far: a job whose PathState condition already holds is started by the
// monitor at its first check-in that includes the job.
static void wait_for_monitor(void)
{
	char script[512], keys[512], probe[256];

	path_in_dir(probe, sizeof(probe), "probe");
	snprintf(script, sizeof(script), "echo run >> %s/probe.log", g_dir);
	snprintf(keys, sizeof(keys),
		"\t<key>KeepAlive</key><dict><key>PathState</key><dict><key>%s</key><true/></dict></dict>\n", probe);
	write_plist("probe.plist", g_labels[3], script, keys);
	touch("probe", NULL);

	long start = now_ms();
	load(3, "probe.plist");
	int lines = wait_lines("probe.log", 1, 15000);
	if (lines < 1)
	{
		printf("FAIL: the event monitor did not start the probe job\n");
		finish(1);
	}
	printf("Event monitor started the probe job %ld ms after its load\n", now_ms() - start);

	run_launchctl("remove", g_labels[3]);
	g_loaded[3] = false;
	remove_file("probe");
}

static void test_watchpaths(void)
{
	char script[512], keys[512], watched[256];

	path_in_dir(watched, sizeof(watched), "watched");
	snprintf(script, sizeof(script), "echo run >> %s/watch.log", g_dir);
	snprintf(keys, sizeof(keys), "\t<key>WatchPaths</key><array><string>%s</string></array>\n", watched);
	write_plist("watch.plist", g_labels[0], script, keys);
	load(0, "watch.plist");
	wait_for_monitor();

	// Loading does not start the job.
	usleep(2000 * 1000);
	int lines = count_lines("watch.log");
	printf("WatchPaths: %d starts after load\n", lines);
	CHECK(lines == 0);

	touch("watched", NULL);
	lines = wait_lines("watch.log", 1, 10000);
	printf("WatchPaths: %d starts after creating the path\n", lines);
	CHECK(lines == 1);

	// Past the throttle interval, so the next start is not delayed.
	CHECK(wait_job_exited(g_labels[0], 10000));
	usleep(1500 * 1000);
	touch("watched", "data\n");
	lines = wait_lines("watch.log", 2, 10000);
	printf("WatchPaths: %d starts after writing to the path\n", lines);
	CHECK(lines == 2);

	CHECK(wait_job_exited(g_labels[0], 10000));
	usleep(1500 * 1000);
	remove_file("watched");
	lines = wait_lines("watch.log", 3, 10000);
	printf("WatchPaths: %d starts after removing the path\n", lines);
	CHECK(lines == 3);

	// One start per change: nothing more without further changes.
	usleep(3000 * 1000);
	lines = count_lines("watch.log");
	printf("WatchPaths: %d starts after waiting\n", lines);
	CHECK(lines == 3);
}

static void test_queuedirectories(void)
{
	char script[1024], keys[512], queue[256];

	path_in_dir(queue, sizeof(queue), "queue");
	// Takes one item per run, so the job has to be started again while
	// the directory is not empty.
	snprintf(script, sizeof(script),
		"for f in %s/queue/*; do [ -e \"$f\" ] &amp;&amp; mv \"$f\" %s/done/ &amp;&amp; break; done; echo run >> %s/queue.log",
		g_dir, g_dir, g_dir);
	snprintf(keys, sizeof(keys), "\t<key>QueueDirectories</key><array><string>%s</string></array>\n", queue);
	write_plist("queue.plist", g_labels[1], script, keys);
	load(1, "queue.plist");

	usleep(2000 * 1000);
	int lines = count_lines("queue.log");
	printf("QueueDirectories: %d starts while empty\n", lines);
	CHECK(lines == 0);

	touch("queue/item1", NULL);
	touch("queue/item2", NULL);
	touch("queue/item3", NULL);
	lines = wait_lines("queue.log", 3, 15000);
	printf("QueueDirectories: %d starts for three items\n", lines);
	CHECK(lines >= 3);
	CHECK(exists("done/item1"));
	CHECK(exists("done/item2"));
	CHECK(exists("done/item3"));
	CHECK(!exists("queue/item1") && !exists("queue/item2") && !exists("queue/item3"));

	// Once the directory is empty the job is no longer started.
	usleep(3000 * 1000);
	int settled = count_lines("queue.log");
	usleep(3000 * 1000);
	lines = count_lines("queue.log");
	printf("QueueDirectories: %d starts, then %d after the queue was emptied\n", settled, lines);
	CHECK(lines == settled);
}

static void test_pathstate(void)
{
	char script[512], keys[512], flag[256];

	path_in_dir(flag, sizeof(flag), "flag");
	snprintf(script, sizeof(script), "echo run >> %s/pstate.log", g_dir);
	snprintf(keys, sizeof(keys),
		"\t<key>KeepAlive</key><dict><key>PathState</key><dict><key>%s</key><true/></dict></dict>\n", flag);
	write_plist("pstate.plist", g_labels[2], script, keys);
	load(2, "pstate.plist");

	usleep(2000 * 1000);
	int lines = count_lines("pstate.log");
	printf("PathState: %d starts while the path is missing\n", lines);
	CHECK(lines == 0);

	// Kept alive while the path exists: started again after each exit.
	touch("flag", NULL);
	lines = wait_lines("pstate.log", 2, 10000);
	printf("PathState: %d starts while the path exists\n", lines);
	CHECK(lines >= 2);

	remove_file("flag");
	usleep(3000 * 1000);
	int settled = count_lines("pstate.log");
	usleep(3000 * 1000);
	lines = count_lines("pstate.log");
	printf("PathState: %d starts, then %d after the path was removed\n", settled, lines);
	CHECK(lines == settled);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(120);

	if (mkdtemp(g_dir) == NULL)
	{
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		finish(1);
	}

	char path[256];
	path_in_dir(path, sizeof(path), "queue");
	mkdir(path, 0755);
	path_in_dir(path, sizeof(path), "done");
	mkdir(path, 0755);

	static const char* const kinds[] = { "watchpaths", "queuedirectories", "pathstate", "monitorprobe" };
	for (int i = 0; i < 4; i++)
		snprintf(g_labels[i], sizeof(g_labels[i]), "org.darlinghq.tests.%s.%d", kinds[i], (int) getpid());

	test_watchpaths();
	test_queuedirectories();
	test_pathstate();

	printf("%s\n", g_failures ? "FAIL" : "PASS");
	finish(g_failures ? 1 : 0);
}
