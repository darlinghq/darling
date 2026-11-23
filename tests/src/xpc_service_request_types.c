// Copyright (C) 2026 Lubos Dolezel
//
// Requests with varied contents to a bundled XPC service, and their replies.
//
// A service started with xpc_main() receives requests on the connection it is
// handed and answers each one with xpc_dictionary_create_reply() and
// xpc_connection_send_message() to the remote connection of the request
// (xpc/connection.h, xpc/xpc.h). The client may wait for the answer with
// xpc_connection_send_message_with_reply_sync() or take it in a reply handler
// (xpc_connection_send_message_with_reply()).
//
// The test builds an application bundle with one service in a temporary
// directory from copies of its own executable (the program picks its role from
// the name it was started under). The application sends one request per case,
// with contents of increasing complexity: strings and integers, data, arrays
// and nested dictionaries, a file descriptor, an endpoint of an anonymous
// listener, and everything together; each case with a synchronous and an
// asynchronous send. The service checks what it received, the application
// checks the reply. The driver kills the application if it makes no progress,
// so a lost reply is a failure and not a hang.
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
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xpc/xpc.h>

extern char** environ;

// exported by libxpc on macOS and in Darling, without a declaration in the
// public SDK headers
extern void xpc_dictionary_set_connection(xpc_object_t xdict, const char* key, xpc_connection_t connection);
extern xpc_object_t xpc_dictionary_get_array(xpc_object_t xdict, const char* key);

#define SERVICE_ID "org.darlinghq.test.xpcreplyservice"
#define STEP_TIMEOUT_SEC 20

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static const char* kAppInfo =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
	"<plist version=\"1.0\"><dict>\n"
	"<key>CFBundleExecutable</key><string>fixtureapp</string>\n"
	"<key>CFBundleIdentifier</key><string>org.darlinghq.test.xpcreplyapp</string>\n"
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

enum {
	CASE_SCALARS = 0,
	CASE_DATA,
	CASE_ARRAY,
	CASE_FD,
	CASE_ENDPOINT,
	CASE_CONNECTION,
	CASE_DISPATCH_DATA,
	CASE_STRINGS_FD,
	CASE_NESTED_REPLY,
	CASE_STRING_LENGTHS,
	CASE_REPLY_REMOTE,
	CASE_COMBINED,
	CASE_COUNT
};

static const char* const kCaseNames[CASE_COUNT] = {
	"scalars", "data", "array", "fd", "endpoint", "connection", "dispatch-data", "strings-fd", "nested-reply", "string-lengths", "reply-remote", "combined"
};

static const char kBlob[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char kFileText[] = "contents of the passed file";

// ---- helpers --------------------------------------------------------------

static const char* base_name(const char* path)
{
	const char* slash = strrchr(path, '/');
	return slash ? slash + 1 : path;
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

// ---- request construction (application) and checks (service) ---------------

static void add_scalars(xpc_object_t msg)
{
	xpc_dictionary_set_string(msg, "text", "hello service");
	xpc_dictionary_set_int64(msg, "i64", -1234567890123LL);
	xpc_dictionary_set_uint64(msg, "u64", 18000000000000000000ULL);
	xpc_dictionary_set_bool(msg, "flag", true);
	xpc_dictionary_set_double(msg, "dbl", 2.5);
}

static bool check_scalars(xpc_object_t msg)
{
	const char* text = xpc_dictionary_get_string(msg, "text");
	return text && strcmp(text, "hello service") == 0
		&& xpc_dictionary_get_int64(msg, "i64") == -1234567890123LL
		&& xpc_dictionary_get_uint64(msg, "u64") == 18000000000000000000ULL
		&& xpc_dictionary_get_bool(msg, "flag")
		&& xpc_dictionary_get_double(msg, "dbl") == 2.5;
}

#define BIG_SIZE (300 * 1024)

static void add_data(xpc_object_t msg)
{
	xpc_dictionary_set_data(msg, "blob", kBlob, sizeof(kBlob));
	// a larger payload, as a tag or artwork block would be
	char* buf = malloc(BIG_SIZE);
	for (size_t i = 0; i < BIG_SIZE; i++) {
		buf[i] = (char)(i * 31 + 7);
	}
	xpc_dictionary_set_data(msg, "big", buf, BIG_SIZE);
	free(buf);
}

static bool check_data(xpc_object_t msg)
{
	size_t len = 0, biglen = 0;
	const void* blob = xpc_dictionary_get_data(msg, "blob", &len);
	const char* big = xpc_dictionary_get_data(msg, "big", &biglen);
	if (!blob || len != sizeof(kBlob) || memcmp(blob, kBlob, len) != 0) {
		return false;
	}
	if (!big || biglen != BIG_SIZE) {
		return false;
	}
	for (size_t i = 0; i < biglen; i++) {
		if (big[i] != (char)(i * 31 + 7)) {
			return false;
		}
	}
	return true;
}

static void add_array(xpc_object_t msg)
{
	xpc_object_t arr = xpc_array_create(NULL, 0);
	xpc_array_set_string(arr, XPC_ARRAY_APPEND, "one");
	xpc_object_t two = xpc_int64_create(2);
	xpc_array_set_value(arr, XPC_ARRAY_APPEND, two);
	xpc_release(two);
	xpc_object_t yes = xpc_bool_create(true);
	xpc_array_set_value(arr, XPC_ARRAY_APPEND, yes);
	xpc_release(yes);
	xpc_object_t nested = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(nested, "inner", "value");
	xpc_array_set_value(arr, XPC_ARRAY_APPEND, nested);
	xpc_release(nested);
	xpc_dictionary_set_value(msg, "arr", arr);
	xpc_release(arr);
}

static bool check_array(xpc_object_t msg)
{
	xpc_object_t arr = xpc_dictionary_get_value(msg, "arr");
	if (!arr || xpc_get_type(arr) != XPC_TYPE_ARRAY || xpc_array_get_count(arr) != 4) {
		return false;
	}
	const char* one = xpc_array_get_string(arr, 0);
	xpc_object_t nested = xpc_array_get_value(arr, 3);
	const char* inner = (nested && xpc_get_type(nested) == XPC_TYPE_DICTIONARY)
		? xpc_dictionary_get_string(nested, "inner") : NULL;
	return one && strcmp(one, "one") == 0 && xpc_array_get_int64(arr, 1) == 2
		&& xpc_bool_get_value(xpc_array_get_value(arr, 2)) && inner && strcmp(inner, "value") == 0;
}

// the application's file descriptor case: the file passed in the request
static void add_fd(xpc_object_t msg, const char* path)
{
	int fd = open(path, O_RDONLY);
	if (fd >= 0) {
		xpc_dictionary_set_fd(msg, "fd", fd);
		close(fd);
	}
}

// reads the passed file in the service and puts the duplicate descriptor in
// the reply
static bool check_fd_and_reply(xpc_object_t msg, xpc_object_t reply)
{
	int fd = xpc_dictionary_dup_fd(msg, "fd");
	if (fd < 0) {
		return false;
	}
	char buf[128] = { 0 };
	ssize_t n = pread(fd, buf, sizeof(buf) - 1, 0);
	xpc_dictionary_set_fd(reply, "fd", fd);
	close(fd);
	return n == (ssize_t)strlen(kFileText) && strcmp(buf, kFileText) == 0;
}

static xpc_connection_t g_listener;

// application side of the anonymous listener used by the endpoint case
static xpc_endpoint_t make_listener_endpoint(void)
{
	g_listener = xpc_connection_create(NULL, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0));
	xpc_connection_set_event_handler(g_listener, ^(xpc_object_t event) {
		if (xpc_get_type(event) != XPC_TYPE_CONNECTION) {
			return;
		}
		xpc_connection_t peer = event;
		xpc_connection_set_event_handler(peer, ^(xpc_object_t msg) {
			if (xpc_get_type(msg) != XPC_TYPE_DICTIONARY) {
				return;
			}
			const char* ask = xpc_dictionary_get_string(msg, "ask");
			xpc_object_t reply = xpc_dictionary_create_reply(msg);
			xpc_dictionary_set_string(reply, "answer", ask ? ask : "");
			xpc_connection_send_message(xpc_dictionary_get_remote_connection(msg), reply);
			xpc_release(reply);
		});
		xpc_connection_resume(peer);
	});
	xpc_connection_resume(g_listener);
	return xpc_endpoint_create(g_listener);
}

// service side: connect to the application's listener and ask it something
static bool check_endpoint(xpc_object_t msg)
{
	xpc_object_t value = xpc_dictionary_get_value(msg, "endpoint");
	if (!value || xpc_get_type(value) != XPC_TYPE_ENDPOINT) {
		return false;
	}
	xpc_connection_t c = xpc_connection_create_from_endpoint(value);
	xpc_connection_set_event_handler(c, ^(xpc_object_t event) { (void)event; });
	xpc_connection_resume(c);
	xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(req, "ask", "who is there");
	xpc_object_t answer = xpc_connection_send_message_with_reply_sync(c, req);
	bool ok = xpc_get_type(answer) == XPC_TYPE_DICTIONARY
		&& xpc_dictionary_get_string(answer, "answer")
		&& strcmp(xpc_dictionary_get_string(answer, "answer"), "who is there") == 0;
	xpc_release(req);
	xpc_release(answer);
	xpc_connection_cancel(c);
	xpc_release(c);
	return ok;
}

// a connection passed in a request (xpc_dictionary_set_connection) is used by
// the service as a connection to the sender
static bool check_connection(xpc_object_t msg)
{
	xpc_connection_t c = xpc_dictionary_create_connection(msg, "conn");
	if (!c || xpc_get_type(c) != XPC_TYPE_CONNECTION) {
		return false;
	}
	xpc_connection_set_event_handler(c, ^(xpc_object_t event) { (void)event; });
	xpc_connection_resume(c);
	xpc_object_t req = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(req, "ask", "who is there");
	xpc_object_t answer = xpc_connection_send_message_with_reply_sync(c, req);
	bool ok = xpc_get_type(answer) == XPC_TYPE_DICTIONARY
		&& xpc_dictionary_get_string(answer, "answer")
		&& strcmp(xpc_dictionary_get_string(answer, "answer"), "who is there") == 0;
	xpc_release(req);
	xpc_release(answer);
	xpc_connection_cancel(c);
	xpc_release(c);
	return ok;
}

// data created from a dispatch_data object, and an array filled with
// xpc_array_append_value
static void add_dispatch_data(xpc_object_t msg)
{
	size_t n = 64 * 1024;
	char* buf = malloc(n);
	for (size_t i = 0; i < n; i++) {
		buf[i] = (char)(i * 13 + 5);
	}
	dispatch_data_t dd = dispatch_data_create(buf, n, NULL, DISPATCH_DATA_DESTRUCTOR_FREE);
	xpc_object_t data = xpc_data_create_with_dispatch_data(dd);
	xpc_dictionary_set_value(msg, "ddata", data);
	xpc_object_t arr = xpc_array_create(NULL, 0);
	xpc_array_append_value(arr, data);
	xpc_object_t num = xpc_uint64_create(77);
	xpc_array_append_value(arr, num);
	xpc_dictionary_set_value(msg, "list", arr);
	xpc_release(num);
	xpc_release(arr);
	xpc_release(data);
}

static bool check_dispatch_data(xpc_object_t msg)
{
	size_t len = 0;
	const char* p = xpc_dictionary_get_data(msg, "ddata", &len);
	if (!p || len != 64 * 1024) {
		return false;
	}
	for (size_t i = 0; i < len; i++) {
		if (p[i] != (char)(i * 13 + 5)) {
			return false;
		}
	}
	xpc_object_t arr = xpc_dictionary_get_array(msg, "list");
	return arr && xpc_array_get_count(arr) == 2 && xpc_array_get_uint64(arr, 1) == 77;
}

// a request of a few strings and a file descriptor, sent by a worker thread
// while the main thread does something else (a tag-reading request, say)
static void add_strings_fd(xpc_object_t msg, const char* path)
{
	xpc_dictionary_set_string(msg, "first", "/");
	xpc_dictionary_set_string(msg, "second", "/");
	xpc_dictionary_set_string(msg, "third", ",");
	xpc_dictionary_set_string(msg, "path", path);
	add_fd(msg, path);
}

static bool check_strings_fd(xpc_object_t msg, xpc_object_t reply)
{
	const char* first = xpc_dictionary_get_string(msg, "first");
	const char* third = xpc_dictionary_get_string(msg, "third");
	const char* path = xpc_dictionary_get_string(msg, "path");
	return first && strcmp(first, "/") == 0 && third && strcmp(third, ",") == 0
		&& path && strstr(path, "passed.txt") != NULL && check_fd_and_reply(msg, reply);
}

// the reply of the nested-reply case: a dictionary of numbers, a double, strings
// and a false bool inside the reply dictionary
static void add_nested_reply(xpc_object_t reply, const char* path)
{
	xpc_object_t track = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(track, "millisecondLength", 3030);
	xpc_dictionary_set_double(track, "length", 3.03);
	xpc_dictionary_set_int64(track, "channels", 1);
	xpc_dictionary_set_int64(track, "sampleRate", 44100);
	xpc_dictionary_set_int64(track, "bitRate", 34);
	xpc_dictionary_set_string(track, "artist", "An Artist");
	xpc_dictionary_set_string(track, "album", "An Album");
	xpc_dictionary_set_string(track, "title", "A Title");
	xpc_dictionary_set_bool(track, "compilation", false);
	xpc_dictionary_set_value(reply, "track", track);
	xpc_release(track);
	xpc_dictionary_set_string(reply, "path", path);
}

static bool check_nested_reply(xpc_object_t reply)
{
	xpc_object_t track = xpc_dictionary_get_value(reply, "track");
	const char* artist = track && xpc_get_type(track) == XPC_TYPE_DICTIONARY
		? xpc_dictionary_get_string(track, "artist") : NULL;
	return track && xpc_get_type(track) == XPC_TYPE_DICTIONARY
		&& xpc_dictionary_get_int64(track, "millisecondLength") == 3030
		&& xpc_dictionary_get_double(track, "length") == 3.03
		&& xpc_dictionary_get_int64(track, "sampleRate") == 44100
		&& !xpc_dictionary_get_bool(track, "compilation")
		&& artist && strcmp(artist, "An Artist") == 0
		&& xpc_dictionary_get_string(reply, "path") != NULL;
}

// strings of every length from 0 to 80 under keys of every length from 1 to 80,
// with a descriptor in the same message
static void add_string_lengths(xpc_object_t msg, const char* path)
{
	for (int i = 0; i <= 80; i++) {
		char key[128] = "k";
		char val[128];
		memset(key + 1, 'x', i);
		key[i + 1] = 0;
		memset(val, 'a' + i % 26, i);
		val[i] = 0;
		xpc_dictionary_set_string(msg, key, val);
	}
	add_fd(msg, path);
}

static bool check_string_lengths(xpc_object_t msg)
{
	for (int i = 0; i <= 80; i++) {
		char key[128] = "k";
		memset(key + 1, 'x', i);
		key[i + 1] = 0;
		const char* val = xpc_dictionary_get_string(msg, key);
		if (!val || strlen(val) != (size_t)i || (i > 0 && val[i - 1] != 'a' + i % 26)) {
			return false;
		}
	}
	return xpc_dictionary_dup_fd(msg, "fd") >= 0;
}

// ---- service --------------------------------------------------------------

static void service_message(xpc_object_t msg)
{
	int64_t which = xpc_dictionary_get_int64(msg, "case");
	xpc_object_t reply = xpc_dictionary_create_reply(msg);
	bool ok = false;

	switch (which) {
		case CASE_SCALARS:
			ok = check_scalars(msg);
			break;
		case CASE_DATA:
			ok = check_data(msg);
			break;
		case CASE_ARRAY:
			ok = check_array(msg);
			break;
		case CASE_FD:
			ok = check_fd_and_reply(msg, reply);
			break;
		case CASE_ENDPOINT:
			ok = check_endpoint(msg);
			break;
		case CASE_CONNECTION:
			ok = check_connection(msg);
			break;
		case CASE_DISPATCH_DATA:
			ok = check_dispatch_data(msg);
			break;
		case CASE_STRINGS_FD:
			ok = check_strings_fd(msg, reply);
			break;
		case CASE_NESTED_REPLY:
			add_nested_reply(reply, "/some/path");
			ok = true;
			break;
		case CASE_STRING_LENGTHS:
			ok = check_string_lengths(msg);
			break;
		case CASE_REPLY_REMOTE:
			ok = check_scalars(msg);
			break;
		case CASE_COMBINED:
			ok = check_scalars(msg) && check_data(msg) && check_array(msg)
				&& check_fd_and_reply(msg, reply) && check_endpoint(msg)
				&& check_connection(msg) && check_dispatch_data(msg);
			break;
	}
	xpc_dictionary_set_bool(reply, "ok", ok);
	xpc_dictionary_set_int64(reply, "pid", getpid());
	xpc_dictionary_set_int64(reply, "case", which);
	xpc_dictionary_set_data(reply, "blob", kBlob, sizeof(kBlob));
	// the reply dictionary of a request knows the connection the request came
	// in on: a service may address the reply through it (xpc_dictionary_get_remote_connection
	// on the reply) instead of on the request
	xpc_connection_t conn = (which == CASE_REPLY_REMOTE)
		? xpc_dictionary_get_remote_connection(reply)
		: xpc_dictionary_get_remote_connection(msg);
	xpc_connection_send_message(conn, reply);
	xpc_release(reply);
}

static void service_connection(xpc_connection_t peer)
{
	xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
		if (xpc_get_type(event) == XPC_TYPE_DICTIONARY) {
			service_message(event);
		}
	});
	xpc_connection_resume(peer);
}

static int role_service(void)
{
	xpc_main(service_connection);
	return 1;
}

// ---- application ----------------------------------------------------------

static xpc_object_t build_request(int which, const char* file, xpc_endpoint_t endpoint)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(msg, "case", which);
	switch (which) {
		case CASE_SCALARS: add_scalars(msg); break;
		case CASE_DATA: add_data(msg); break;
		case CASE_ARRAY: add_array(msg); break;
		case CASE_FD: add_fd(msg, file); break;
		case CASE_ENDPOINT: xpc_dictionary_set_value(msg, "endpoint", endpoint); break;
		case CASE_CONNECTION: xpc_dictionary_set_connection(msg, "conn", g_listener); break;
		case CASE_DISPATCH_DATA: add_dispatch_data(msg); break;
		case CASE_STRINGS_FD: add_strings_fd(msg, file); break;
		case CASE_STRING_LENGTHS: add_string_lengths(msg, file); break;
		case CASE_REPLY_REMOTE: add_scalars(msg); break;
		case CASE_COMBINED:
			add_scalars(msg);
			add_data(msg);
			add_array(msg);
			add_fd(msg, file);
			xpc_dictionary_set_value(msg, "endpoint", endpoint);
			xpc_dictionary_set_connection(msg, "conn", g_listener);
			add_dispatch_data(msg);
			break;
	}
	return msg;
}

static bool check_reply(xpc_object_t reply, int which)
{
	if (xpc_get_type(reply) != XPC_TYPE_DICTIONARY) {
		char* desc = xpc_copy_description(reply);
		printf("INFO %s: reply is %s\n", kCaseNames[which], desc);
		free(desc);
		return false;
	}
	bool ok = xpc_dictionary_get_bool(reply, "ok")
		&& xpc_dictionary_get_int64(reply, "case") == which
		&& xpc_dictionary_get_int64(reply, "pid") > 0
		&& xpc_dictionary_get_int64(reply, "pid") != getpid();
	size_t len = 0;
	const void* blob = xpc_dictionary_get_data(reply, "blob", &len);
	ok = ok && blob && len == sizeof(kBlob) && memcmp(blob, kBlob, len) == 0;
	if (which == CASE_NESTED_REPLY) {
		ok = ok && check_nested_reply(reply);
	}
	if (which == CASE_FD || which == CASE_STRINGS_FD || which == CASE_COMBINED) {
		int fd = xpc_dictionary_dup_fd(reply, "fd");
		char buf[128] = { 0 };
		bool fdok = fd >= 0 && pread(fd, buf, sizeof(buf) - 1, 0) == (ssize_t)strlen(kFileText)
			&& strcmp(buf, kFileText) == 0;
		if (fd >= 0) {
			close(fd);
		}
		ok = ok && fdok;
	}
	return ok;
}

static int role_app(const char* dir)
{
	char file[1024];
	snprintf(file, sizeof(file), "%s/passed.txt", dir);
	CHECK(write_file(file, kFileText) == 0);

	xpc_endpoint_t endpoint = make_listener_endpoint();

	xpc_connection_t c = xpc_connection_create(SERVICE_ID, NULL);
	xpc_connection_set_target_queue(c, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0));
	xpc_connection_set_event_handler(c, ^(xpc_object_t event) { (void)event; });
	xpc_connection_resume(c);

	for (int which = 0; which < CASE_COUNT; which++) {
		printf("STEP %s sync\n", kCaseNames[which]);
		xpc_object_t msg = build_request(which, file, endpoint);
		__block xpc_object_t reply = NULL;
		if (which == CASE_STRINGS_FD) {
			dispatch_semaphore_t done = dispatch_semaphore_create(0);
			dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
				reply = xpc_connection_send_message_with_reply_sync(c, msg);
				dispatch_semaphore_signal(done);
			});
			dispatch_semaphore_wait(done, DISPATCH_TIME_FOREVER);
		} else {
			reply = xpc_connection_send_message_with_reply_sync(c, msg);
		}
		bool ok = check_reply(reply, which);
		printf("%s %s sync\n", ok ? "PASS" : "FAIL", kCaseNames[which]);
		if (!ok) {
			g_failures++;
		}
		xpc_release(reply);
		xpc_release(msg);

		printf("STEP %s async\n", kCaseNames[which]);
		msg = build_request(which, file, endpoint);
		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		__block bool aok = false;
		xpc_connection_send_message_with_reply(c, msg, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
			^(xpc_object_t r) {
				aok = check_reply(r, which);
				dispatch_semaphore_signal(sem);
			});
		long timed_out = dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, STEP_TIMEOUT_SEC * NSEC_PER_SEC));
		printf("%s %s async%s\n", (!timed_out && aok) ? "PASS" : "FAIL", kCaseNames[which],
			timed_out ? " (no reply)" : "");
		if (timed_out || !aok) {
			g_failures++;
		}
		xpc_release(msg);
	}

	xpc_connection_cancel(c);
	printf("%s app\n", g_failures ? "FAIL" : "PASS");
	return g_failures ? 1 : 0;
}

// ---- driver ---------------------------------------------------------------

// waits for the application; kills it when it prints nothing for too long
static int run_app(const char* appexe, const char* dir, int* status)
{
	int pipefd[2];
	if (pipe(pipefd) != 0) {
		return -1;
	}
	posix_spawn_file_actions_t fa;
	posix_spawn_file_actions_init(&fa);
	posix_spawn_file_actions_adddup2(&fa, pipefd[1], 1);
	posix_spawn_file_actions_addclose(&fa, pipefd[0]);
	char* argv[] = { (char*)appexe, (char*)dir, NULL };
	pid_t pid;
	if (posix_spawn(&pid, appexe, &fa, NULL, argv, environ) != 0) {
		return -1;
	}
	close(pipefd[1]);

	char line[4096];
	size_t used = 0;
	for (;;) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(pipefd[0], &rfds);
		struct timeval tv = { STEP_TIMEOUT_SEC + 10, 0 };
		int r = select(pipefd[0] + 1, &rfds, NULL, NULL, &tv);
		if (r == 0) {
			printf("FAIL: the application made no progress for %d s (a reply is missing); killing it\n",
				STEP_TIMEOUT_SEC + 10);
			g_failures++;
			kill(pid, SIGKILL);
			break;
		}
		char buf[512];
		ssize_t n = read(pipefd[0], buf, sizeof(buf));
		if (n <= 0) {
			break;
		}
		for (ssize_t i = 0; i < n; i++) {
			if (buf[i] == '\n' || used == sizeof(line) - 1) {
				line[used] = 0;
				used = 0;
				printf("app: %s\n", line);
				fflush(stdout);
			} else {
				line[used++] = buf[i];
			}
		}
	}
	close(pipefd[0]);
	posix_spawn_file_actions_destroy(&fa);
	if (waitpid(pid, status, 0) != pid) {
		return -1;
	}
	return 0;
}

static int role_driver(const char* self)
{
	char dir[] = "/private/tmp/xpcreply.XXXXXX";
	char app[1024], appexe[1024], svcdir[1024], svcexe[1024], path[1024];

	if (!mkdtemp(dir)) {
		printf("FAIL: mkdtemp: %s\n", strerror(errno));
		return 1;
	}
	snprintf(app, sizeof(app), "%s/Fixture.app", dir);
	snprintf(appexe, sizeof(appexe), "%s/Contents/MacOS/fixtureapp", app);
	snprintf(svcdir, sizeof(svcdir), "%s/Contents/XPCServices/%s.xpc", app, SERVICE_ID);
	snprintf(svcexe, sizeof(svcexe), "%s/Contents/MacOS/fixturesvc", svcdir);

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

	if (g_failures) {
		printf("FAIL: could not build the fixture\n");
		nftw(dir, remove_entry, 16, FTW_DEPTH | FTW_PHYS);
		return 1;
	}

	int status = -1;
	CHECK(run_app(appexe, dir, &status) == 0);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);

	nftw(dir, remove_entry, 16, FTW_DEPTH | FTW_PHYS);
	printf("%s xpc_service_request_types\n", g_failures ? "FAIL" : "PASS");
	return g_failures ? 1 : 0;
}

int main(int argc, char** argv)
{
	const char* name = base_name(argv[0]);

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (strcmp(name, "fixturesvc") == 0) {
		return role_service();
	}
	if (strcmp(name, "fixtureapp") == 0 && argc >= 2) {
		return role_app(argv[1]);
	}

	char self[1024];
	uint32_t size = sizeof(self);
	if (_NSGetExecutablePath(self, &size) != 0) {
		printf("FAIL: executable path\n");
		return 1;
	}
	return role_driver(self);
}
