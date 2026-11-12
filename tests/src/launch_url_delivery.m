// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices/AppKit: "open <url>" for a scheme registered through
// CFBundleURLTypes launches the app and delivers the URL after
// applicationWillFinishLaunching:, through the kAEGetURL Apple Event handler
// when the app installed one and through application:openURLs: otherwise. A URL
// opened while the app is already running is delivered to the running instance.
//
// The program builds two throw-away app bundles from its own executable, one
// per delivery path, registers them with LSRegisterURL, drives "open", and the
// bundles record what they received in a log file. Run with no arguments.
#import <Cocoa/Cocoa.h>
#import <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

// kInternetEventClass and kAEGetURL, both the four-character code 'GURL'.
#define URL_EVENT_CLASS ((AEEventClass) 'GURL')
#define URL_EVENT_ID ((AEEventID) 'GURL')

#define ID_OPENURLS @"org.darlinghq.test.urllaunch.openurls"
#define ID_AEGET    @"org.darlinghq.test.urllaunch.aeget"

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static NSString *LogPath(NSString *bundleID)
{
	// Not per user: LaunchServices starts applications through launchd, whose jobs may run as another user than the test.
	return [NSString stringWithFormat:@"/tmp/%@.log", bundleID];
}

static void AppendLog(NSString *bundleID, NSString *line)
{
	FILE *f = fopen([LogPath(bundleID) fileSystemRepresentation], "a");
	if (f) {
		fprintf(f, "%s\n", [line UTF8String]);
		fclose(f);
	}
}

// ---- app side ----

@interface URLApp : NSObject <NSApplicationDelegate>
@property (copy) NSString *bundleID;
@end

@implementation URLApp
- (void)applicationWillFinishLaunching:(NSNotification *)n
{
	AppendLog(self.bundleID, @"willFinishLaunching");
	if ([self.bundleID isEqualToString:ID_AEGET]) {
		[[NSAppleEventManager sharedAppleEventManager] setEventHandler:self
			andSelector:@selector(getURL:withReplyEvent:)
			forEventClass:URL_EVENT_CLASS andEventID:URL_EVENT_ID];
	}
}
- (void)applicationDidFinishLaunching:(NSNotification *)n
{
	AppendLog(self.bundleID, @"didFinishLaunching");
}
- (void)getURL:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)reply
{
	NSString *url = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
	AppendLog(self.bundleID, [NSString stringWithFormat:@"URL aeget %@", url]);
}
- (void)application:(NSApplication *)app openURLs:(NSArray<NSURL *> *)urls
{
	for (NSURL *u in urls)
		AppendLog(self.bundleID, [NSString stringWithFormat:@"URL openurls %@", [u absoluteString]]);
}
@end

static int RunApp(NSString *bundleID)
{
	@autoreleasepool {
		AppendLog(bundleID, [NSString stringWithFormat:@"pid %d", (int) getpid()]);
		NSApplication *app = [NSApplication sharedApplication];
		[app setActivationPolicy:NSApplicationActivationPolicyAccessory];
		URLApp *delegate = [[URLApp alloc] init];
		delegate.bundleID = bundleID;
		[app setDelegate:delegate];
		[app run];
	}
	return 0;
}

// ---- driver side ----

static NSString *ReadLog(NSString *bundleID)
{
	NSString *s = [NSString stringWithContentsOfFile:LogPath(bundleID) encoding:NSUTF8StringEncoding error:NULL];
	return s ? s : @"";
}

static BOOL WaitForLog(NSString *bundleID, NSString *needle, int seconds)
{
	for (int i = 0; i < seconds * 10; i++) {
		if ([ReadLog(bundleID) rangeOfString:needle].location != NSNotFound)
			return YES;
		usleep(100000);
	}
	return NO;
}

static NSString *MakeBundle(NSString *root, NSString *bundleID, NSString *scheme, NSString *exe)
{
	NSString *app = [root stringByAppendingPathComponent:[bundleID stringByAppendingString:@".app"]];
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
	[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
	[fm copyItemAtPath:exe toPath:[macos stringByAppendingPathComponent:@"urlapp"] error:NULL];
	NSDictionary *info = @{
		@"CFBundleIdentifier": bundleID,
		@"CFBundleName": @"urlapp",
		@"CFBundleExecutable": @"urlapp",
		@"CFBundlePackageType": @"APPL",
		@"CFBundleVersion": @"1",
		@"CFBundleShortVersionString": @"1.0",
		@"LSUIElement": @YES,
		@"CFBundleURLTypes": @[ @{ @"CFBundleURLName": bundleID, @"CFBundleURLSchemes": @[ scheme ] } ],
	};
	[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
	return app;
}

static int Open(NSString *url)
{
	char *argv[] = { "/usr/bin/open", (char *) [url UTF8String], NULL };
	pid_t pid;
	if (posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) != 0)
		return -1;
	int status = 0;
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int pid_of(NSString *bundleID)
{
	for (NSString *line in [ReadLog(bundleID) componentsSeparatedByString:@"\n"])
		if ([line hasPrefix:@"pid "])
			return [[line substringFromIndex:4] intValue];
	return 0;
}

static void Scenario(NSString *root, NSString *exe, NSString *bundleID, NSString *scheme, NSString *how)
{
	unlink([LogPath(bundleID) fileSystemRepresentation]);
	NSString *app = MakeBundle(root, bundleID, scheme, exe);
	OSStatus st = LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true);
	printf("%s: LSRegisterURL=%d\n", [how UTF8String], (int) st);
	CHECK(st == noErr);

	// The registered scheme resolves to the bundle.
	CFStringRef handler = LSCopyDefaultHandlerForURLScheme((__bridge CFStringRef) scheme);
	printf("%s: default handler for %s: %s\n", [how UTF8String], [scheme UTF8String],
		handler ? [(__bridge NSString *) handler UTF8String] : "(none)");
	CHECK(handler && [(__bridge NSString *) handler caseInsensitiveCompare:bundleID] == NSOrderedSame);
	if (handler)
		CFRelease(handler);

	// App not running: launched by open, URL delivered after willFinishLaunching.
	NSString *url1 = [NSString stringWithFormat:@"%@://first/path?x=1", scheme];
	int rc = Open(url1);
	printf("%s: open #1 rc=%d\n", [how UTF8String], rc);
	CHECK(rc == 0);
	BOOL got = WaitForLog(bundleID, [@"URL " stringByAppendingString:how], 20);
	NSString *log = ReadLog(bundleID);
	printf("%s: log after first open:\n%s", [how UTF8String], [log UTF8String]);
	CHECK(got);
	CHECK([log rangeOfString:url1].location != NSNotFound);
	NSRange will = [log rangeOfString:@"willFinishLaunching"];
	NSRange url = [log rangeOfString:@"URL "];
	CHECK(will.location != NSNotFound && url.location != NSNotFound && will.location < url.location);
	CHECK([log rangeOfString:[@"URL " stringByAppendingString:how]].location != NSNotFound);

	// App already running: delivered to the same instance.
	int pid = pid_of(bundleID);
	NSString *url2 = [NSString stringWithFormat:@"%@://second", scheme];
	rc = Open(url2);
	printf("%s: open #2 rc=%d\n", [how UTF8String], rc);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, url2, 20));
	log = ReadLog(bundleID);
	int pids = 0;
	for (NSString *l in [log componentsSeparatedByString:@"\n"])
		if ([l hasPrefix:@"pid "])
			pids++;
	printf("%s: log after second open:\n%s", [how UTF8String], [log UTF8String]);
	CHECK(pids == 1);

	if (pid > 0)
		kill(pid, SIGTERM);
	usleep(300000);
	unlink([LogPath(bundleID) fileSystemRepresentation]);
	[[NSFileManager defaultManager] removeItemAtPath:app error:NULL];
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		NSString *bid = [[NSBundle mainBundle] bundleIdentifier];
		if ([bid isEqualToString:ID_OPENURLS] || [bid isEqualToString:ID_AEGET])
			return RunApp(bid);

		char path[4096];
		uint32_t len = sizeof(path);
		_NSGetExecutablePath(path, &len);
		NSString *exe = [[NSString stringWithUTF8String:path] stringByResolvingSymlinksInPath];
		// LaunchServices does not index applications under /tmp.
		NSString *apps = [NSHomeDirectory() stringByAppendingPathComponent:@"Applications"];
		[[NSFileManager defaultManager] createDirectoryAtPath:apps withIntermediateDirectories:YES attributes:nil error:NULL];
		char tmpl[4096];
		snprintf(tmpl, sizeof(tmpl), "%s/urllaunch.XXXXXX", [apps fileSystemRepresentation]);
		NSString *root = [NSString stringWithUTF8String:mkdtemp(tmpl)];
		Scenario(root, exe, ID_OPENURLS, @"darlingurltesta", @"openurls");
		Scenario(root, exe, ID_AEGET, @"darlingurltestb", @"aeget");
		rmdir(tmpl);
		printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	}
	return g_failures != 0;
}
