// Copyright (C) 2026 Lubos Dolezel
//
// XPC services bundled in an application (Contents/XPCServices/*.xpc).
//
// Applications connect to such a service with xpc_connection_create() and the
// service bundle's CFBundleIdentifier. The services are private to the process
// that owns the bundle: launchctl(1) describes a "pid/<pid>" domain per
// process, holding the XPC services reachable from it with
// xpc_connection_create(3). Services whose ServiceType is Application are
// started on demand, per the daemons and services programming guide.
//
// The test builds a small application bundle in a temporary directory from
// copies of its own executable (the program picks its role from the name it
// was started under): the application, one bundled service, and an outsider
// that is not part of any bundle. It checks that
//  - a process that does not own the bundle cannot reach the service,
//  - the application reaches the service by bundle identifier, gets a reply
//    from a different process, and its connections share one instance,
//  - a child of the application (another process) cannot reach it either,
//  - a service that is not bundled anywhere is not found,
//  - the service goes away with the application.
#include <dispatch/dispatch.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xpc/xpc.h>

extern char** environ;

#define SERVICE_ID "org.darlinghq.test.bundledxpcservice"
#define MISSING_ID "org.darlinghq.test.bundledxpcservice.missing"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const char* kAppInfo =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
	"<plist version=\"1.0\"><dict>\n"
	"<key>CFBundleExecutable</key><string>fixtureapp</string>\n"
	"<key>CFBundleIdentifier</key><string>org.darlinghq.test.bundledxpcapp</string>\n"
	"<key>CFBundleName</key><string>fixtureapp</string>\n"
	"<key>CFBundlePackageType</key><string>APPL</string>\n"
	"<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\n"
	"</dict></plist>\n";

static const char* kServiceInfo =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
	"<plist version=\"1.0\"><dict>\n"
	"<key>CFBundleExecutable</key><string>fixturesvc</string>\n"
	"<key>CFBundleIdentifier</key><string>" SERVICE_ID "</string>\n"
	"<key>CFBundleName</key><string>fixturesvc</string>\n"
	"<key>CFBundlePackageType</key><string>XPC!</string>\n"
	"<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\n"
	"<key>XPCService</key><dict><key>ServiceType</key><string>Application</string></dict>\n"
	"</dict></plist>\n";

// ---- helpers --------------------------------------------------------------

static const char* base_name(const char* path)
{
	const char* slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
}

// Sends one request over a new connection to `name`. Returns true and fills
// *pid when the peer replied; false when it did not (*invalid tells whether
// the connection was reported invalid).
static bool ask_service(const char* name, const char* text, long long* pid, bool* invalid)
{
	xpc_connection_t c = xpc_connection_create(name, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0));
	xpc_connection_set_event_handler(c, ^(xpc_object_t event) { (void)event; });
	xpc_connection_resume(c);

	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(msg, "text", text);
	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(c, msg);

	bool ok = false;
	if (invalid) {
		*invalid = (reply == XPC_ERROR_CONNECTION_INVALID);
	}
	if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
		const char* echo = xpc_dictionary_get_string(reply, "echo");
		if (echo && strcmp(echo, text) == 0) {
			*pid = xpc_dictionary_get_int64(reply, "pid");
			ok = true;
		}
	}
	xpc_release(msg);
	xpc_release(reply);
	xpc_connection_cancel(c);
	xpc_release(c);
	return ok;
}

static int copy_file(const char* from, const char* to)
{
	int in = open(from, O_RDONLY);
	int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	char buf[65536];
	ssize_t n;
	if (in < 0 || out < 0) {
		return -1;
	}
	while ((n = read(in, buf, sizeof(buf))) > 0) {
		if (write(out, buf, n) != n) {
			return -1;
		}
	}
	close(in);
	close(out);
	return n < 0 ? -1 : 0;
}

static int write_file(const char* path, const char* content)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		return -1;
	}
	ssize_t n = write(fd, content, strlen(content));
	close(fd);
	return n == (ssize_t)strlen(content) ? 0 : -1;
}

static int remove_entry(const char* path, const struct stat* sb, int flag, struct FTW* ftw)
{
	(void)sb; (void)flag; (void)ftw;
	return remove(path);
}

static int run_and_wait(const char* path, const char* arg, int* status)
{
	char* argv[] = { (char*)path, (char*)arg, NULL };
	pid_t pid;
	if (posix_spawn(&pid, path, NULL, NULL, argv, environ) != 0) {
		return -1;
	}
	if (waitpid(pid, status, 0) != pid) {
		return -1;
	}
	return 0;
}

// ---- roles ----------------------------------------------------------------

static void service_connection(xpc_connection_t peer)
{
	xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
		if (xpc_get_type(event) != XPC_TYPE_DICTIONARY) {
			return;
		}
		xpc_object_t reply = xpc_dictionary_create_reply(event);
		const char* text = xpc_dictionary_get_string(event, "text");
		xpc_dictionary_set_string(reply, "echo", text ? text : "");
		xpc_dictionary_set_int64(reply, "pid", getpid());
		xpc_connection_send_message(xpc_dictionary_get_remote_connection(event), reply);
		xpc_release(reply);
	});
	xpc_connection_resume(peer);
}

static int role_service(void)
{
	xpc_main(service_connection);
	return 1;
}

// dir: the test's temporary directory (the service's pid is left there)
static int role_app(const char* dir)
{
	long long pid1 = 0, pid2 = 0, pid_missing = 0;
	bool invalid = false;
	char outsider[1024];
	snprintf(outsider, sizeof(outsider), "%s/outsider", dir);

	CHECK(ask_service(SERVICE_ID, "hello", &pid1, NULL));
	CHECK(pid1 > 0 && pid1 != getpid());
	printf("INFO service pid %lld\n", pid1);

	// a second connection is served by the same instance: a service of type
	// Application has one instance per application, shared by its connections
	// (black-box observation on macOS 26.7.3)
	CHECK(ask_service(SERVICE_ID, "again", &pid2, NULL));
	CHECK(pid2 == pid1);

	// a name that no bundle declares is not found
	CHECK(!ask_service(MISSING_ID, "x", &pid_missing, &invalid));
	CHECK(invalid);

	// another process, even one started by the application, is outside the
	// application's domain
	int status = -1;
	CHECK(run_and_wait(outsider, "--probe", &status) == 0);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);

	char path[1024];
	snprintf(path, sizeof(path), "%s/service.pid", dir);
	char text[32];
	snprintf(text, sizeof(text), "%lld\n", pid1);
	CHECK(write_file(path, text) == 0);

	printf("%s app\n", g_failures ? "FAIL" : "PASS");
	return g_failures ? 1 : 0;
}

static int role_probe(void)
{
	long long pid = 0;
	bool invalid = false;
	bool ok = ask_service(SERVICE_ID, "probe", &pid, &invalid);
	printf("%s outsider: ok=%d invalid=%d\n", (!ok && invalid) ? "PASS" : "FAIL", ok, invalid);
	return (!ok && invalid) ? 0 : 1;
}

// ---- driver ---------------------------------------------------------------

static int role_driver(const char* self)
{
	char dir[] = "/private/tmp/xpcbundled.XXXXXX";
	char app[1024], appexe[1024], svcdir[1024], svcexe[1024], outsider[1024], path[1024];

	if (!mkdtemp(dir)) {
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		return 1;
	}
	snprintf(app, sizeof(app), "%s/Fixture.app", dir);
	snprintf(appexe, sizeof(appexe), "%s/Contents/MacOS/fixtureapp", app);
	snprintf(svcdir, sizeof(svcdir), "%s/Contents/XPCServices/%s.xpc", app, SERVICE_ID);
	snprintf(svcexe, sizeof(svcexe), "%s/Contents/MacOS/fixturesvc", svcdir);
	snprintf(outsider, sizeof(outsider), "%s/outsider", dir);

	const char* dirs[] = { "Fixture.app", "Fixture.app/Contents", "Fixture.app/Contents/MacOS",
		"Fixture.app/Contents/XPCServices", NULL };
	for (int i = 0; dirs[i]; i++) {
		snprintf(path, sizeof(path), "%s/%s", dir, dirs[i]);
		CHECK(mkdir(path, 0755) == 0);
	}
	CHECK(mkdir(svcdir, 0755) == 0);
	snprintf(path, sizeof(path), "%s/Contents", svcdir);
	CHECK(mkdir(path, 0755) == 0);
	snprintf(path, sizeof(path), "%s/Contents/MacOS", svcdir);
	CHECK(mkdir(path, 0755) == 0);

	snprintf(path, sizeof(path), "%s/Contents/Info.plist", app);
	CHECK(write_file(path, kAppInfo) == 0);
	snprintf(path, sizeof(path), "%s/Contents/Info.plist", svcdir);
	CHECK(write_file(path, kServiceInfo) == 0);
	CHECK(copy_file(self, appexe) == 0);
	CHECK(copy_file(self, svcexe) == 0);
	CHECK(copy_file(self, outsider) == 0);

	if (g_failures) {
		printf("FAIL: could not build the fixture\n");
		nftw(dir, remove_entry, 16, FTW_DEPTH | FTW_PHYS);
		return 1;
	}

	// 1. this process owns no bundle: the service is not reachable
	{
		long long pid = 0;
		bool invalid = false;
		CHECK(!ask_service(SERVICE_ID, "driver", &pid, &invalid));
		CHECK(invalid);
	}

	// 2. the application's checks
	int status = -1;
	CHECK(run_and_wait(appexe, dir, &status) == 0);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);

	// 3. the service ends with its application
	snprintf(path, sizeof(path), "%s/service.pid", dir);
	FILE* f = fopen(path, "r");
	long long svc_pid = 0;
	if (f) {
		CHECK(fscanf(f, "%lld", &svc_pid) == 1);
		fclose(f);
	}
	CHECK(svc_pid > 0);
	bool gone = false;
	for (int i = 0; svc_pid > 0 && i < 100 && !gone; i++) {
		gone = (kill((pid_t)svc_pid, 0) == -1 && errno == ESRCH);
		if (!gone) {
			usleep(100000);
		}
	}
	CHECK(gone);

	// 4. and is not reachable any more
	{
		long long pid = 0;
		bool invalid = false;
		CHECK(!ask_service(SERVICE_ID, "late", &pid, &invalid));
		CHECK(invalid);
	}

	if (!gone && svc_pid > 0) {
		kill((pid_t)svc_pid, SIGKILL);
	}
	nftw(dir, remove_entry, 16, FTW_DEPTH | FTW_PHYS);

	printf("%s xpc_bundled_services\n", g_failures ? "FAIL" : "PASS");
	return g_failures ? 1 : 0;
}

int main(int argc, char** argv)
{
	const char* name = base_name(argv[0]);

	if (strcmp(name, "fixturesvc") == 0) {
		return role_service();
	}
	if (strcmp(name, "fixtureapp") == 0 && argc >= 2) {
		return role_app(argv[1]);
	}
	if (argc >= 2 && strcmp(argv[1], "--probe") == 0) {
		return role_probe();
	}

	char self[1024];
	uint32_t size = sizeof(self);
	if (_NSGetExecutablePath(self, &size) != 0) {
		printf("FAIL: executable path\n");
		return 1;
	}
	return role_driver(self);
}
