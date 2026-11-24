// Copyright (C) 2026 Lubos Dolezel
//
// The legacy ServiceManagement C API: SMJobSubmit / SMJobRemove / SMJobCopyDictionary /
// SMCopyAllJobDictionaries, SMJobBless (helper installation, replacement, privileged Mach service
// lookup over NSXPCConnection and plain XPC) and SMLoginItemSetEnabled.
//
// One executable plays every role. It must be linked with the two property lists of a privileged
// helper (-sectcreate __TEXT __info_plist <file> -sectcreate __TEXT __launchd_plist <file>, see the
// plists in servicemanagement_legacy/). Started on its own it builds an app bundle in a temporary
// directory (the executable, a privileged helper and a login item are copies of itself) and re-runs
// itself inside it; the role is chosen from the program name.
//
// Build (the two -sectcreate options embed the helper's plists, found beside this file):
//   clang <target flags> servicemanagement_legacy.m -framework Foundation -framework ServiceManagement \
//     -Wl,-sectcreate,__TEXT,__info_plist,servicemanagement_legacy/helper-info.plist \
//     -Wl,-sectcreate,__TEXT,__launchd_plist,servicemanagement_legacy/helper-launchd.plist
//
// Extra arguments for the bundled app: "install" (bless and leave the helper installed), "check"
// (the helper installed earlier answers; for runs across a container restart) and "cleanup".
#import <Foundation/Foundation.h>
#import <ServiceManagement/ServiceManagement.h>
#include <xpc/xpc.h>
#include <crt_externs.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define HELPER_LABEL "org.darlinghq.tests.smlegacy.helper"
#define HELPER_RAW_SERVICE HELPER_LABEL ".raw"
#define UNLISTED_LABEL "org.darlinghq.tests.smlegacy.unlisted"
#define JOB_LABEL "org.darlinghq.tests.smlegacy.job"
#define ITEM_ID "org.darlinghq.tests.smlegacy.item"
#define ITEM_MARKER "/tmp/" ITEM_ID ".pid"

@protocol SMTestHelper
- (void)pingWithReply:(void (^)(NSString *answer, int uid))reply;
@end

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// ---- helper role: serves HELPER_LABEL over NSXPC and HELPER_RAW_SERVICE over plain XPC ----

@interface SMTestHelperService : NSObject <SMTestHelper, NSXPCListenerDelegate>
@end
@implementation SMTestHelperService
- (void)pingWithReply:(void (^)(NSString *, int))reply { reply(@"pong", (int) getuid()); }
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)connection
{
	connection.exportedInterface = [NSXPCInterface interfaceWithProtocol: @protocol(SMTestHelper)];
	connection.exportedObject = self;
	[connection resume];
	return YES;
}
@end

static int runHelper(void)
{
	@autoreleasepool {
		SMTestHelperService *service = [[SMTestHelperService alloc] init];
		NSXPCListener *listener = [[NSXPCListener alloc] initWithMachServiceName: @HELPER_LABEL];
		listener.delegate = service;
		[listener resume];

		xpc_connection_t raw = xpc_connection_create_mach_service(HELPER_RAW_SERVICE, dispatch_get_main_queue(), XPC_CONNECTION_MACH_SERVICE_LISTENER);
		xpc_connection_set_event_handler(raw, ^(xpc_object_t peer) {
			if (xpc_get_type(peer) != XPC_TYPE_CONNECTION)
				return;
			xpc_connection_set_event_handler(peer, ^(xpc_object_t message) {
				if (xpc_get_type(message) != XPC_TYPE_DICTIONARY)
					return;
				xpc_object_t reply = xpc_dictionary_create_reply(message);
				xpc_dictionary_set_string(reply, "answer", "pong");
				xpc_dictionary_set_int64(reply, "uid", getuid());
				xpc_connection_send_message(xpc_dictionary_get_remote_connection(message), reply);
			});
			xpc_connection_resume(peer);
		});
		xpc_connection_resume(raw);
		[[NSRunLoop currentRunLoop] run];
	}
	return 0;
}

// ---- login item role ----

static int runLoginItem(void)
{
	char text[32];
	snprintf(text, sizeof(text), "%d\n", (int) getpid());
	FILE *marker = fopen(ITEM_MARKER ".tmp", "w");
	if (!marker)
		return 1;
	fputs(text, marker);
	fclose(marker);
	rename(ITEM_MARKER ".tmp", ITEM_MARKER);
	for (;;)
		sleep(60);
}

// ---- app role ----

static BOOL waitFor(BOOL (^condition)(void), int seconds)
{
	for (int i = 0; i < seconds * 10; i++) {
		if (condition())
			return YES;
		usleep(100000);
	}
	return condition();
}

static NSString *errorSummary(CFErrorRef error)
{
	return error ? [NSString stringWithFormat: @"%@/%ld", (NSString *) CFErrorGetDomain(error), (long) CFErrorGetCode(error)] : @"none";
}

static void checkErrorTable(void)
{
	CFErrorRef error = NULL;
	CHECK(!SMJobBless(kSMDomainSystemLaunchd, CFSTR(UNLISTED_LABEL), NULL, &error));
	// macOS reports 4 (NULL AuthorizationRef) or 2 (reference without the right) before it looks at the
	// label; Darling never enforces authorization, so the codes of the structural refusals are a
	// choice without Mac evidence: kSMErrorJobNotFound for a label the app does not list, and
	// kSMErrorInternalFailure for a domain other than the system one.
	CHECK(error && CFErrorGetCode(error) == kSMErrorJobNotFound && [(NSString *) CFErrorGetDomain(error) isEqualToString: @"CFErrorDomainLaunchd"]);
	if (error) CFRelease(error);
	error = NULL;
	CHECK(!SMJobBless(kSMDomainUserLaunchd, CFSTR(HELPER_LABEL), NULL, &error));
	CHECK(error && CFErrorGetCode(error) == kSMErrorInternalFailure && [(NSString *) CFErrorGetDomain(error) isEqualToString: @"CFErrorDomainLaunchd"]);
	if (error) CFRelease(error);
	error = NULL;
	CHECK(!SMJobBless(CFSTR("bogus"), CFSTR(HELPER_LABEL), NULL, &error));
	CHECK(error && CFErrorGetCode(error) == kSMErrorInternalFailure);
	if (error) CFRelease(error);

	// Observed on macOS 26.7.3 and 10.14.4: a job without a Label fails with code 10 of CFErrorDomainLaunchd.
	error = NULL;
	NSDictionary *unlabeled = @{ @"ProgramArguments": @[@"/bin/sleep", @"60"] };
	CHECK(!SMJobSubmit(kSMDomainUserLaunchd, (CFDictionaryRef) unlabeled, NULL, &error));
	CHECK(error && CFErrorGetCode(error) == 10 && [(NSString *) CFErrorGetDomain(error) isEqualToString: @"CFErrorDomainLaunchd"]);
	if (error) CFRelease(error);

	error = NULL;
	CHECK(SMJobRemove(kSMDomainUserLaunchd, CFSTR(UNLISTED_LABEL), NULL, false, &error));
	CHECK(!error);
	CFDictionaryRef missing = SMJobCopyDictionary(kSMDomainUserLaunchd, CFSTR(UNLISTED_LABEL));
	CHECK(missing == NULL);
	if (missing) CFRelease(missing);
	CHECK(!SMLoginItemSetEnabled(CFSTR(UNLISTED_LABEL), true));
	CHECK(!SMLoginItemSetEnabled(CFSTR(UNLISTED_LABEL), false));
}

static void checkJobs(void)
{
	NSDictionary *job = @{ @"Label": @JOB_LABEL, @"ProgramArguments": @[@"/bin/sleep", @"60"] };
	CFErrorRef error = NULL;
	CHECK(SMJobSubmit(kSMDomainUserLaunchd, (CFDictionaryRef) job, NULL, &error));
	CHECK(!error);
	if (error) { CFRelease(error); error = NULL; }

	// Observed on macOS: submitting the same label again succeeds.
	CHECK(SMJobSubmit(kSMDomainUserLaunchd, (CFDictionaryRef) job, NULL, &error));
	CHECK(!error);
	if (error) { CFRelease(error); error = NULL; }

	CFDictionaryRef copy = SMJobCopyDictionary(kSMDomainUserLaunchd, CFSTR(JOB_LABEL));
	CHECK(copy && [[(NSDictionary *) copy objectForKey: @"Label"] isEqual: @JOB_LABEL]);
	if (copy) CFRelease(copy);
	// Darling has one launchd, so the system domain sees the same job.
	copy = SMJobCopyDictionary(kSMDomainSystemLaunchd, CFSTR(JOB_LABEL));
	CHECK(copy != NULL);
	if (copy) CFRelease(copy);

	CFArrayRef all = SMCopyAllJobDictionaries(kSMDomainUserLaunchd);
	BOOL found = NO;
	for (NSDictionary *entry in (NSArray *) all)
		found |= [[entry objectForKey: @"Label"] isEqual: @JOB_LABEL];
	CHECK(found);
	if (all) CFRelease(all);

	// Observed on macOS: a domain string that is not one of the two constants behaves as the user domain.
	CFDictionaryRef bogus = SMJobCopyDictionary(CFSTR("bogus"), CFSTR(JOB_LABEL));
	CHECK(bogus != NULL);
	if (bogus) CFRelease(bogus);
	CHECK(SMJobSubmit(CFSTR("bogus"), (CFDictionaryRef) job, NULL, &error));
	CHECK(!error);
	if (error) { CFRelease(error); error = NULL; }
	CFArrayRef bogusAll = SMCopyAllJobDictionaries(CFSTR("bogus"));
	CHECK(bogusAll != NULL);
	if (bogusAll) CFRelease(bogusAll);

	CHECK(SMJobRemove(kSMDomainUserLaunchd, CFSTR(JOB_LABEL), NULL, true, &error));
	CHECK(!error);
	// Waiting for the removal means the job is already gone.
	copy = SMJobCopyDictionary(kSMDomainUserLaunchd, CFSTR(JOB_LABEL));
	CHECK(copy == NULL);
	if (copy) CFRelease(copy);
	CHECK(SMJobRemove(kSMDomainUserLaunchd, CFSTR(JOB_LABEL), NULL, true, &error));
}

static BOOL pingNSXPC(void)
{
	NSXPCConnection *connection = [[NSXPCConnection alloc] initWithMachServiceName: @HELPER_LABEL options: NSXPCConnectionPrivileged];
	connection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol: @protocol(SMTestHelper)];
	[connection resume];
	__block NSString *answer = nil;
	__block int uid = -1;
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	id<SMTestHelper> proxy = [connection remoteObjectProxyWithErrorHandler: ^(NSError *error) { printf("NSXPC error: %s\n", [[error description] UTF8String]); dispatch_semaphore_signal(done); }];
	[proxy pingWithReply: ^(NSString *a, int u) { answer = [a copy]; uid = u; dispatch_semaphore_signal(done); }];
	dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
	[connection invalidate];
	// A daemon installed by SMJobBless runs as root, like the other system daemons.
	return [answer isEqualToString: @"pong"] && uid == 0;
}

static BOOL pingXPC(void)
{
	xpc_connection_t connection = xpc_connection_create_mach_service(HELPER_RAW_SERVICE, NULL, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
	xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {});
	xpc_connection_resume(connection);
	xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(message, "op", "ping");
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	__block BOOL ok = NO;
	xpc_connection_send_message_with_reply(connection, message, dispatch_get_global_queue(0, 0), ^(xpc_object_t reply) {
		if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
			const char *answer = xpc_dictionary_get_string(reply, "answer");
			ok = answer && strcmp(answer, "pong") == 0 && xpc_dictionary_get_int64(reply, "uid") == 0;
		}
		dispatch_semaphore_signal(done);
	});
	dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
	xpc_connection_cancel(connection);
	return ok;
}

static void cleanupHelper(void)
{
	CFErrorRef error = NULL;
	SMJobRemove(kSMDomainSystemLaunchd, CFSTR(HELPER_LABEL), NULL, true, &error);
	if (error) CFRelease(error);
	unlink("/Library/LaunchDaemons/" HELPER_LABEL ".plist");
	unlink("/Library/PrivilegedHelperTools/" HELPER_LABEL);
}

static void checkBless(void)
{
	cleanupHelper();
	CFErrorRef error = NULL;
	CHECK(SMJobBless(kSMDomainSystemLaunchd, CFSTR(HELPER_LABEL), NULL, &error));
	CHECK(!error);
	if (error) { printf("bless error %s\n", [errorSummary(error) UTF8String]); CFRelease(error); error = NULL; }

	struct stat st;
	CHECK(stat("/Library/PrivilegedHelperTools/" HELPER_LABEL, &st) == 0 && (st.st_mode & 0111));
	NSDictionary *plist = [NSDictionary dictionaryWithContentsOfFile: @"/Library/LaunchDaemons/" HELPER_LABEL ".plist"];
	CHECK([[plist objectForKey: @"Label"] isEqual: @HELPER_LABEL]);
	CHECK([[plist objectForKey: @"Program"] isEqual: @"/Library/PrivilegedHelperTools/" HELPER_LABEL]);
	CHECK([[plist objectForKey: @"MachServices"] objectForKey: @HELPER_LABEL] != nil);
	CFDictionaryRef job = SMJobCopyDictionary(kSMDomainSystemLaunchd, CFSTR(HELPER_LABEL));
	CHECK(job != NULL);
	if (job) CFRelease(job);

	CHECK(pingNSXPC());
	CHECK(pingXPC());

	// Blessing again replaces the earlier installation.
	CHECK(SMJobBless(kSMDomainSystemLaunchd, CFSTR(HELPER_LABEL), NULL, &error));
	if (error) { CFRelease(error); error = NULL; }
	CHECK(pingNSXPC());
}

static void checkLoginItem(void)
{
	unlink(ITEM_MARKER);
	NSString *record = [NSHomeDirectory() stringByAppendingPathComponent: @"Library/LaunchAgents/org.darlinghq.loginitem." ITEM_ID ".plist"];
	CHECK(SMLoginItemSetEnabled(CFSTR(ITEM_ID), true));
	CHECK([[NSFileManager defaultManager] fileExistsAtPath: record]);
	BOOL started = waitFor(^BOOL { return access(ITEM_MARKER, F_OK) == 0; }, 30);
	CHECK(started);
	int pid = 0;
	FILE *f = fopen(ITEM_MARKER, "r");
	if (f) { fscanf(f, "%d", &pid); fclose(f); }
	CHECK(pid > 0 && kill(pid, 0) == 0);

	CHECK(SMLoginItemSetEnabled(CFSTR(ITEM_ID), false));
	CHECK(![[NSFileManager defaultManager] fileExistsAtPath: record]);
	CHECK(waitFor(^BOOL { return pid <= 0 || kill(pid, 0) != 0; }, 20));
	unlink(ITEM_MARKER);
}

static int runApp(int argc, char **argv)
{
	@autoreleasepool {
		const char *mode = argc > 1 ? argv[1] : "all";
		if (strcmp(mode, "install") == 0) {
			CFErrorRef error = NULL;
			BOOL ok = SMJobBless(kSMDomainSystemLaunchd, CFSTR(HELPER_LABEL), NULL, &error);
			printf("install: %s %s\n", ok ? "ok" : "failed", [errorSummary(error) UTF8String]);
			return ok ? 0 : 1;
		}
		if (strcmp(mode, "item-on") == 0) {
			// Enables the login item and leaves it registered (for runs across a container restart).
			unlink(ITEM_MARKER);
			CHECK(SMLoginItemSetEnabled(CFSTR(ITEM_ID), true));
			CHECK(waitFor(^BOOL { return access(ITEM_MARKER, F_OK) == 0; }, 30));
		} else if (strcmp(mode, "item-started") == 0) {
			// After a restart the registered login item starts again by itself.
			CHECK(waitFor(^BOOL { return access(ITEM_MARKER, F_OK) == 0; }, 60));
		} else if (strcmp(mode, "item-off") == 0) {
			CHECK(SMLoginItemSetEnabled(CFSTR(ITEM_ID), false));
		} else if (strcmp(mode, "check") == 0) {
			CHECK(pingNSXPC());
			CHECK(pingXPC());
		} else if (strcmp(mode, "cleanup") == 0) {
			cleanupHelper();
		} else {
			// "errors", "jobs", "bless" and "item" run a single part.
			BOOL all = strcmp(mode, "all") == 0;
			if (all || strcmp(mode, "errors") == 0)
				checkErrorTable();
			if (all || strcmp(mode, "jobs") == 0)
				checkJobs();
			if (all || strcmp(mode, "bless") == 0)
				checkBless();
			if (all || strcmp(mode, "item") == 0)
				checkLoginItem();
			if (all || strcmp(mode, "bless") == 0)
				cleanupHelper();
		}
	}
	if (g_failures)
		printf("%d failures\n", g_failures);
	else
		printf("PASS\n");
	return g_failures ? 1 : 0;
}

// ---- launcher: builds the bundle and re-runs itself inside ----

static void copyFile(NSString *from, NSString *to)
{
	[[NSFileManager defaultManager] createDirectoryAtPath: [to stringByDeletingLastPathComponent] withIntermediateDirectories: YES attributes: nil error: NULL];
	NSError *error = nil;
	if (![[NSFileManager defaultManager] copyItemAtPath: from toPath: to error: &error]) {
		printf("FAIL: copy %s: %s\n", [to UTF8String], [[error description] UTF8String]);
		exit(1);
	}
}

static int launch(int argc, char **argv)
{
	char path[1024];
	uint32_t size = sizeof(path);
	_NSGetExecutablePath(path, &size);
	char dirTemplate[1024] = "/tmp/smlegacy.XXXXXX";
	// SMLEGACY_DIR names an existing directory to build the bundle in, for runs across a container
	// restart (/tmp does not survive one).
	if (getenv("SMLEGACY_DIR"))
		snprintf(dirTemplate, sizeof(dirTemplate), "%s", getenv("SMLEGACY_DIR"));
	else if (!mkdtemp(dirTemplate)) {
		printf("FAIL: mkdtemp\n");
		return 1;
	}
	[[NSFileManager defaultManager] removeItemAtPath: [[NSString stringWithUTF8String: dirTemplate] stringByAppendingPathComponent: @"smlegacy.app"] error: NULL];
	NSString *self = [NSString stringWithUTF8String: path];
	NSString *app = [[NSString stringWithUTF8String: dirTemplate] stringByAppendingPathComponent: @"smlegacy.app/Contents"];
	copyFile(self, [app stringByAppendingPathComponent: @"MacOS/smlegacy"]);
	copyFile(self, [app stringByAppendingPathComponent: @"Library/LaunchServices/" HELPER_LABEL]);
	copyFile(self, [app stringByAppendingPathComponent: @"Library/LoginItems/Item.app/Contents/MacOS/smlegacy-item"]);

	NSDictionary *info = @{
		@"CFBundleIdentifier": @"org.darlinghq.tests.smlegacy",
		@"CFBundleExecutable": @"smlegacy",
		@"CFBundlePackageType": @"APPL",
		@"LSBackgroundOnly": @YES,
		@"SMPrivilegedExecutables": @{ @HELPER_LABEL: @"anchor apple" },
	};
	NSDictionary *itemInfo = @{
		@"CFBundleIdentifier": @ITEM_ID,
		@"CFBundleExecutable": @"smlegacy-item",
		@"CFBundlePackageType": @"APPL",
		@"LSBackgroundOnly": @YES,
	};
	[info writeToFile: [app stringByAppendingPathComponent: @"Info.plist"] atomically: NO];
	[itemInfo writeToFile: [app stringByAppendingPathComponent: @"Library/LoginItems/Item.app/Contents/Info.plist"] atomically: NO];

	NSString *bundled = [app stringByAppendingPathComponent: @"MacOS/smlegacy"];
	pid_t pid = fork();
	if (pid == 0) {
		char **args = calloc(argc + 1, sizeof(char *));
		args[0] = (char *) [bundled UTF8String];
		for (int i = 1; i < argc; i++)
			args[i] = argv[i];
		execv(args[0], args);
		_exit(127);
	}
	int status = 0;
	waitpid(pid, &status, 0);
	// The "install" step keeps the bundle (the helper has its own copy), everything else is temporary.
	BOOL keep = getenv("SMLEGACY_DIR") || argc > 1 && (strcmp(argv[1], "install") == 0 || strncmp(argv[1], "item-", 5) == 0);
	if (!keep)
		[[NSFileManager defaultManager] removeItemAtPath: [NSString stringWithUTF8String: dirTemplate] error: NULL];
	return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

int main(int argc, char **argv)
{
	char path[1024];
	uint32_t size = sizeof(path);
	_NSGetExecutablePath(path, &size);
	char *copy = strdup(path);
	const char *name = basename(copy);
	if (strcmp(name, HELPER_LABEL) == 0)
		return runHelper();
	if (strcmp(name, "smlegacy-item") == 0)
		return runLoginItem();
	if (strstr(path, ".app/Contents/MacOS/"))
		return runApp(argc, argv);
	return launch(argc, argv);
}
