// SMAppService regression test. Run it plainly: it stages an app bundle holding a copy of itself, a
// LaunchAgent, a LaunchDaemon and a login item, then executes that copy so that [NSBundle mainBundle]
// is the app. Exits non-zero on failure. Build as an Objective-C executable linked against Foundation and
// ServiceManagement and run it with `darling shell`.
#import <Foundation/Foundation.h>
#import <ServiceManagement/ServiceManagement.h>
#include <launch.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
	if (!ok)
		failures++;
}

static void checkError(NSError *error, NSInteger code, NSString *what)
{
	expect([[error domain] isEqualToString: SMAppServiceErrorDomain] && [error code] == code,
		[NSString stringWithFormat: @"%@ (got %@ %ld)", what, [error domain], (long) [error code]]);
}

static NSString *jobProgram(NSString *label)
{
	launch_data_t request = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(request, launch_data_new_string([label UTF8String]), LAUNCH_KEY_GETJOB);
	launch_data_t reply = launch_msg(request);
	launch_data_free(request);
	NSString *program = nil;
	if (reply && launch_data_get_type(reply) == LAUNCH_DATA_DICTIONARY) {
		launch_data_t value = launch_data_dict_lookup(reply, LAUNCH_JOBKEY_PROGRAM);
		if (value)
			program = [NSString stringWithUTF8String: launch_data_get_string(value)];
	}
	if (reply)
		launch_data_free(reply);
	return program;
}

// NSFileManager's createDirectoryAtPath fails under Darling's /tmp symlink, so directories are made with mkdir(2).
static void makeParent(NSString *path)
{
	NSString *parent = [path stringByDeletingLastPathComponent];
	if ([parent length] > 1 && access([parent fileSystemRepresentation], F_OK) != 0) {
		makeParent(parent);
		mkdir([parent fileSystemRepresentation], 0755);
	}
}

static void writePlist(NSDictionary *plist, NSString *path)
{
	makeParent(path);
	[plist writeToFile: path atomically: YES];
}

static void writeScript(NSString *path)
{
	makeParent(path);
	[@"#!/bin/sh\nexec /bin/sleep 300\n" writeToFile: path atomically: YES encoding: NSUTF8StringEncoding error: NULL];
	chmod([path fileSystemRepresentation], 0755);
}

static int stageAndRun(void)
{
	char self[PATH_MAX];
	uint32_t size = sizeof(self);
	_NSGetExecutablePath(self, &size);

	NSString *app = [NSTemporaryDirectory() stringByAppendingPathComponent: [NSString stringWithFormat: @"SMAppServiceTest-%d.app", getpid()]];
	NSString *contents = [app stringByAppendingPathComponent: @"Contents"];
	NSString *executable = [contents stringByAppendingPathComponent: @"MacOS/smappservice"];
	makeParent(executable);
	if (![[NSData dataWithContentsOfFile: [NSString stringWithUTF8String: self]] writeToFile: executable atomically: NO]) {
		printf("FAIL: cannot stage %s from %s\n", [executable UTF8String], self);
		return 1;
	}
	chmod([executable fileSystemRepresentation], 0755);
	writePlist(@{ @"CFBundleIdentifier": @"org.darlinghq.smappservice-test", @"CFBundleExecutable": @"smappservice", @"CFBundlePackageType": @"APPL" },
		[contents stringByAppendingPathComponent: @"Info.plist"]);

	writeScript([contents stringByAppendingPathComponent: @"MacOS/helper"]);
	for (NSString *kind in @[@"Agent", @"Daemon"]) {
		writePlist(@{ @"Label": [@"org.darlinghq.smappservice-test." stringByAppendingString: [kind lowercaseString]], @"BundleProgram": @"Contents/MacOS/helper" },
			[contents stringByAppendingPathComponent: [NSString stringWithFormat: @"Library/Launch%@s/test-%@.plist", kind, [kind lowercaseString]]]);
	}

	NSString *loginItem = [contents stringByAppendingPathComponent: @"Library/LoginItems/Helper.app/Contents"];
	writePlist(@{ @"CFBundleIdentifier": @"org.darlinghq.smappservice-test.loginitem", @"CFBundleExecutable": @"Helper", @"CFBundlePackageType": @"APPL" },
		[loginItem stringByAppendingPathComponent: @"Info.plist"]);
	writeScript([loginItem stringByAppendingPathComponent: @"MacOS/Helper"]);

	execl([executable fileSystemRepresentation], "smappservice", (char *) NULL);
	printf("FAIL: cannot run %s\n", [executable UTF8String]);
	return 1;
}

static void testLaunchdJob(SMAppService *service, NSString *label, NSString *program, NSString *name)
{
	NSError *error = nil;
	expect([service status] == SMAppServiceStatusNotRegistered, [name stringByAppendingString: @" starts not registered"]);
	expect([service registerAndReturnError: &error] && error == nil, [NSString stringWithFormat: @"%@ registers (%@)", name, error]);
	expect([service status] == SMAppServiceStatusEnabled, [name stringByAppendingString: @" is enabled after register"]);
	expect([jobProgram(label) isEqualToString: program], [NSString stringWithFormat: @"%@ runs %@ (launchd has %@)", name, program, jobProgram(label)]);

	error = nil;
	expect(![service registerAndReturnError: &error], [name stringByAppendingString: @" cannot register twice"]);
	checkError(error, kSMErrorAlreadyRegistered, [name stringByAppendingString: @" second register reports kSMErrorAlreadyRegistered"]);

	error = nil;
	expect([service unregisterAndReturnError: &error] && error == nil, [NSString stringWithFormat: @"%@ unregisters (%@)", name, error]);
	expect([service status] == SMAppServiceStatusNotRegistered, [name stringByAppendingString: @" is not registered after unregister"]);
	expect(jobProgram(label) == nil, [name stringByAppendingString: @" is gone from launchd"]);

	error = nil;
	expect(![service unregisterAndReturnError: &error], [name stringByAppendingString: @" cannot unregister twice"]);
	checkError(error, kSMErrorJobNotFound, [name stringByAppendingString: @" second unregister reports kSMErrorJobNotFound"]);
}

static void runTests(void)
{
	NSString *bundle = [[NSBundle mainBundle] bundlePath];
	NSError *error = nil;

	testLaunchdJob([SMAppService agentServiceWithPlistName: @"test-agent.plist"], @"org.darlinghq.smappservice-test.agent",
		[bundle stringByAppendingPathComponent: @"Contents/MacOS/helper"], @"agent");
	testLaunchdJob([SMAppService daemonServiceWithPlistName: @"test-daemon.plist"], @"org.darlinghq.smappservice-test.daemon",
		[bundle stringByAppendingPathComponent: @"Contents/MacOS/helper"], @"daemon");
	testLaunchdJob([SMAppService loginItemServiceWithIdentifier: @"org.darlinghq.smappservice-test.loginitem"], @"org.darlinghq.smappservice-test.loginitem",
		[bundle stringByAppendingPathComponent: @"Contents/Library/LoginItems/Helper.app/Contents/MacOS/Helper"], @"login item");

	SMAppService *agent = [SMAppService agentServiceWithPlistName: @"test-agent.plist"];
	expect([agent registerAndReturnError: NULL], @"agent registers again for the asynchronous unregister");
	dispatch_semaphore_t done = dispatch_semaphore_create(0);
	__block NSError *asyncError = [NSError errorWithDomain: @"unset" code: 0 userInfo: nil];
	[agent unregisterWithCompletionHandler: ^(NSError *e) {
		asyncError = e;
		dispatch_semaphore_signal(done);
	}];
	expect(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0 && asyncError == nil,
		[NSString stringWithFormat: @"unregisterWithCompletionHandler: succeeds (%@)", asyncError]);
	expect([agent status] == SMAppServiceStatusNotRegistered, @"agent is not registered after the asynchronous unregister");

	SMAppService *missing = [SMAppService agentServiceWithPlistName: @"missing.plist"];
	expect([missing status] == SMAppServiceStatusNotFound, @"missing agent plist is not found");
	expect(![missing registerAndReturnError: &error], @"missing agent plist does not register");
	checkError(error, kSMErrorJobPlistNotFound, @"missing agent plist reports kSMErrorJobPlistNotFound");

	SMAppService *missingItem = [SMAppService loginItemServiceWithIdentifier: @"org.darlinghq.smappservice-test.nothing"];
	error = nil;
	expect([missingItem status] == SMAppServiceStatusNotFound, @"missing login item is not found");
	expect(![missingItem registerAndReturnError: &error], @"missing login item does not register");
	checkError(error, kSMErrorToolNotValid, @"missing login item reports kSMErrorToolNotValid");

	SMAppService *main = [SMAppService mainAppService];
	error = nil;
	expect([main status] == SMAppServiceStatusNotRegistered, @"main app is not registered");
	expect(![main registerAndReturnError: &error], @"main app cannot be registered for login");
	checkError(error, kSMErrorServiceUnavailable, @"main app register reports kSMErrorServiceUnavailable");
	error = nil;
	expect(![main unregisterAndReturnError: &error], @"main app cannot be unregistered");
	checkError(error, kSMErrorJobNotFound, @"main app unregister reports kSMErrorJobNotFound");

	expect([SMAppService statusForLegacyURL: [NSURL fileURLWithPath: @"/System/Library/LaunchDaemons/org.darlinghq.shellspawn.plist"]] == SMAppServiceStatusEnabled,
		@"loaded legacy daemon is enabled");
	expect([SMAppService statusForLegacyURL: [NSURL fileURLWithPath: @"/Library/LaunchDaemons/org.darlinghq.nothing.plist"]] == SMAppServiceStatusNotFound,
		@"missing legacy plist is not found");
}

int main(void)
{
	@autoreleasepool {
		if (![[[NSBundle mainBundle] bundlePath] hasSuffix: @".app"])
			return stageAndRun();
		runTests();
		[[NSFileManager defaultManager] removeItemAtPath: [[NSBundle mainBundle] bundlePath] error: NULL];
		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
