// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices/AppKit: launching an application and opening documents. Every launch gives the
// application exactly one initial Apple Event, handled inside -[NSApplication run] after
// applicationWillFinishLaunching: and before applicationDidFinishLaunching::
//   - open-application ('oapp'): applicationShouldOpenUntitledFile:, applicationOpenUntitledFile:, then
//     applicationDidFinishLaunching: with NSApplicationLaunchIsDefaultLaunchKey = 1, handled while the
//     event is current. Also when the executable is started directly, not by LaunchServices.
//   - open-documents ('odoc'): application:openFiles: with all documents in one call (no untitled
//     document), then applicationDidFinishLaunching: with the key = 0 and no current event.
//   - documents named in argv are opened first, and the later 'oapp' then opens no untitled document.
// Opening an application that is running sends it 'rapp' (applicationShouldHandleReopen:...), and a
// document sent to it arrives as 'odoc' in the same process.
// The sequences are the ones observed on macOS (10.14.4 and 26.7.3).
//
// The program builds two throw-away app bundles from its own executable, registers them with
// LSRegisterURL, drives "open" and direct launches, and the bundles record their callbacks in a
// log file. Run with no arguments.
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

#define ID_FILES @"org.darlinghq.test.doclaunch.openfiles"
#define ID_FILE  @"org.darlinghq.test.doclaunch.openfile"

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

static NSString *CurrentEvent(void)
{
	NSAppleEventDescriptor *e = [[NSAppleEventManager sharedAppleEventManager] currentAppleEvent];
	if (e == nil)
		return @"nil";
	OSType c = CFSwapInt32HostToBig([e eventClass]), i = CFSwapInt32HostToBig([e eventID]);
	return [NSString stringWithFormat:@"%.4s/%.4s", (char *) &c, (char *) &i];
}

@interface DocApp : NSObject <NSApplicationDelegate> {
	NSString *_bundleID;
}
@property (copy) NSString *bundleID;
- (void)log:(NSString *)what;
@end

@implementation DocApp
@synthesize bundleID = _bundleID;
- (void)log:(NSString *)what
{
	AppendLog(self.bundleID, [NSString stringWithFormat:@"%@ [current event %@]", what, CurrentEvent()]);
}
- (void)applicationWillFinishLaunching:(NSNotification *)n
{
	[self log:@"willFinishLaunching"];
}
- (void)applicationDidFinishLaunching:(NSNotification *)n
{
	[self log:[NSString stringWithFormat:@"didFinishLaunching default=%@",
		[[n userInfo] objectForKey:NSApplicationLaunchIsDefaultLaunchKey]]];
}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)app
{
	[self log:@"shouldOpenUntitledFile"];
	return YES;
}
- (BOOL)applicationOpenUntitledFile:(NSApplication *)app
{
	[self log:@"openUntitledFile"];
	return YES;
}
- (BOOL)applicationShouldHandleReopen:(NSApplication *)app hasVisibleWindows:(BOOL)flag
{
	[self log:@"shouldHandleReopen"];
	return NO;
}
- (void)application:(NSApplication *)app openFiles:(NSArray<NSString *> *)files
{
	[self log:[NSString stringWithFormat:@"openFiles %lu", (unsigned long) [files count]]];
	for (NSString *f in files)
		AppendLog(self.bundleID, [NSString stringWithFormat:@"file %@", [f lastPathComponent]]);
	[app replyToOpenOrPrint:NSApplicationDelegateReplySuccess];
}
@end

@interface SingleDocApp : DocApp
@end

@implementation SingleDocApp
// Only openFile: is implemented; openFiles: is not forwarded to this class.
- (BOOL)respondsToSelector:(SEL)sel
{
	if (sel == @selector(application:openFiles:))
		return NO;
	return [super respondsToSelector:sel];
}
- (BOOL)application:(NSApplication *)app openFile:(NSString *)file
{
	[self log:@"openFile"];
	AppendLog(self.bundleID, [NSString stringWithFormat:@"file %@", [file lastPathComponent]]);
	return YES;
}
@end

static int RunApp(NSString *bundleID, int argc, char **argv)
{
	@autoreleasepool {
		AppendLog(bundleID, [NSString stringWithFormat:@"pid %d", (int) getpid()]);
		NSApplication *app = [NSApplication sharedApplication];
		[app setActivationPolicy:NSApplicationActivationPolicyAccessory];
		DocApp *delegate = [bundleID isEqualToString:ID_FILE] ? [[SingleDocApp alloc] init] : [[DocApp alloc] init];
		delegate.bundleID = bundleID;
		[app setDelegate:delegate];
		// As a program built on NSApplicationMain: a file named in argv is opened first.
		NSApplicationMain(argc, (const char **) argv);
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

static NSString *MakeBundle(NSString *root, NSString *bundleID, NSString *extension, NSString *exe)
{
	NSString *app = [root stringByAppendingPathComponent:[bundleID stringByAppendingString:@".app"]];
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
	[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
	[fm copyItemAtPath:exe toPath:[macos stringByAppendingPathComponent:@"docapp"] error:NULL];
	NSDictionary *info = @{
		@"CFBundleIdentifier": bundleID,
		@"CFBundleName": @"docapp",
		@"CFBundleExecutable": @"docapp",
		@"CFBundlePackageType": @"APPL",
		@"CFBundleVersion": @"1",
		@"CFBundleShortVersionString": @"1.0",
		@"LSUIElement": @YES,
		@"CFBundleDocumentTypes": @[ @{
			@"CFBundleTypeName": @"Test Document",
			@"CFBundleTypeRole": @"Editor",
			@"LSHandlerRank": @"Owner",
			@"CFBundleTypeExtensions": @[ extension ],
		} ],
	};
	[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
	return app;
}

static int Open(NSArray<NSString *> *args)
{
	NSMutableArray *all = [NSMutableArray arrayWithObject:@"/usr/bin/open"];
	[all addObjectsFromArray:args];
	char **argv = calloc([all count] + 1, sizeof(char *));
	for (NSUInteger i = 0; i < [all count]; i++)
		argv[i] = (char *) [all[i] UTF8String];
	pid_t pid;
	int rc = posix_spawn(&pid, argv[0], NULL, NULL, argv, environ);
	free(argv);
	if (rc != 0)
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

static NSString *MakeDoc(NSString *root, NSString *name)
{
	NSString *path = [root stringByAppendingPathComponent:name];
	[@"x" writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:NULL];
	return path;
}

// The callback lines of the log, without the process ids and with the callbacks' current event.
static NSArray *Sequence(NSString *bundleID)
{
	NSMutableArray *lines = [NSMutableArray array];
	for (NSString *l in [ReadLog(bundleID) componentsSeparatedByString:@"\n"])
		if ([l length] > 0 && ![l hasPrefix:@"pid "])
			[lines addObject:l];
	return lines;
}

static void Expect(NSString *what, NSString *bundleID, NSArray *expected)
{
	NSArray *got = Sequence(bundleID);
	printf("%s: %s\n", [what UTF8String], [[got componentsJoinedByString:@" | "] UTF8String]);
	CHECK([got isEqualToArray:expected]);
	if (![got isEqualToArray:expected])
		printf("  expected: %s\n", [[expected componentsJoinedByString:@" | "] UTF8String]);
}

static void Reset(NSString *bundleID, int pid)
{
	if (pid > 0)
		kill(pid, SIGTERM);
	usleep(500000);
	unlink([LogPath(bundleID) fileSystemRepresentation]);
}

static int Spawn(NSString *exe, NSString *arg)
{
	char *argv[] = { (char *) [exe fileSystemRepresentation], arg ? (char *) [arg fileSystemRepresentation] : NULL, NULL };
	pid_t pid;
	return posix_spawn(&pid, argv[0], NULL, NULL, argv, environ) == 0 ? (int) pid : -1;
}

static void Scenario(NSString *root, NSString *exe, NSString *bundleID, NSString *extension, NSString *how, BOOL batch)
{
	unlink([LogPath(bundleID) fileSystemRepresentation]);
	NSString *app = MakeBundle(root, bundleID, extension, exe);
	OSStatus st = LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true);
	printf("%s: LSRegisterURL=%d\n", [how UTF8String], (int) st);
	CHECK(st == noErr);

	NSString *doc1 = MakeDoc(root, [@"one." stringByAppendingString:extension]);
	NSString *doc2 = MakeDoc(root, [@"two." stringByAppendingString:extension]);
	NSString *doc3 = MakeDoc(root, [@"three." stringByAppendingString:extension]);
	NSString *open1 = batch ? @"openFiles 1" : @"openFile";
	NSString *openN = batch ? @"openFiles 2" : @"openFile";
	NSString *no = @" [current event nil]";
	NSString *sOpenFiles1 = [open1 stringByAppendingString:@" [current event aevt/odoc]"];
	NSString *willFinish = [@"willFinishLaunching" stringByAppendingString:no];

	// The type resolves to the bundle.
	CFURLRef handler = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) [NSURL fileURLWithPath:doc1], kLSRolesAll, NULL);
	NSString *handlerPath = handler ? [(__bridge NSURL *) handler path] : nil;
	CHECK(handlerPath != nil && [handlerPath hasSuffix:[bundleID stringByAppendingString:@".app"]]);
	if (handler)
		CFRelease(handler);

	// Launch with one document: odoc replaces oapp; no untitled document; did-finish is not the default launch.
	int rc = Open(@[ doc1 ]);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, @"didFinishLaunching", 20));
	int pid = pid_of(bundleID);
	NSMutableArray *expected = [NSMutableArray arrayWithObjects:willFinish, sOpenFiles1, @"file one." , nil];
	expected[2] = [@"file one." stringByAppendingString:extension];
	[expected addObject:[@"didFinishLaunching default=0" stringByAppendingString:no]];
	Expect(how, bundleID, expected);

	// A document for the running instance arrives in the same process.
	rc = Open(@[ doc2 ]);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, [@"file two." stringByAppendingString:extension], 20));
	CHECK(pid_of(bundleID) == pid);
	NSString *log = ReadLog(bundleID);
	CHECK([log rangeOfString:[@"file two." stringByAppendingString:extension]].location != NSNotFound);

	// Opening the running application again is a reopen; no new process.
	rc = Open(@[ @"-a", app ]);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, @"shouldHandleReopen", 20));
	CHECK([[ReadLog(bundleID) componentsSeparatedByString:@"pid "] count] == 2);
	Reset(bundleID, pid);

	// Plain launch: oapp, untitled-document callbacks, did-finish as the default launch inside oapp.
	rc = Open(@[ @"-a", app ]);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, @"didFinishLaunching", 20));
	pid = pid_of(bundleID);
	Expect(@"plain launch", bundleID, @[ willFinish,
		@"shouldOpenUntitledFile [current event aevt/oapp]",
		@"openUntitledFile [current event aevt/oapp]",
		@"didFinishLaunching default=1 [current event aevt/oapp]" ]);
	Reset(bundleID, pid);

	// Two documents at launch arrive together (one call with both when openFiles: is implemented).
	rc = Open(@[ @"-a", app, doc2, doc3 ]);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, @"didFinishLaunching", 20));
	pid = pid_of(bundleID);
	log = ReadLog(bundleID);
	CHECK([log rangeOfString:[openN stringByAppendingString:@" [current event aevt/odoc]"]].location != NSNotFound);
	CHECK([log rangeOfString:@"shouldOpenUntitledFile"].location == NSNotFound);
	CHECK([log rangeOfString:[@"file three." stringByAppendingString:extension]].location != NSNotFound);
	Reset(bundleID, pid);

	// Started directly with a document argument: the document is opened from argv (no current event),
	// then oapp arrives and creates no untitled document.
	NSString *direct = [app stringByAppendingPathComponent:@"Contents/MacOS/docapp"];
	pid = Spawn(direct, doc1);
	CHECK(pid > 0);
	CHECK(WaitForLog(bundleID, @"didFinishLaunching", 20));
	log = ReadLog(bundleID);
	printf("%s direct launch with argv document:\n%s", [how UTF8String], [log UTF8String]);
	NSRange opened = [log rangeOfString:[open1 stringByAppendingString:no]];
	NSRange finished = [log rangeOfString:@"didFinishLaunching default=1 [current event aevt/oapp]"];
	CHECK(opened.location != NSNotFound && finished.location != NSNotFound && opened.location < finished.location);
	CHECK([log rangeOfString:@"shouldOpenUntitledFile"].location == NSNotFound);
	Reset(bundleID, pid);

	// Started directly without arguments: also gets oapp.
	pid = Spawn(direct, nil);
	CHECK(pid > 0);
	CHECK(WaitForLog(bundleID, @"didFinishLaunching", 20));
	CHECK([ReadLog(bundleID) rangeOfString:@"didFinishLaunching default=1 [current event aevt/oapp]"].location != NSNotFound);
	Reset(bundleID, pid);

	[[NSFileManager defaultManager] removeItemAtPath:app error:NULL];
	unlink([doc1 fileSystemRepresentation]);
	unlink([doc2 fileSystemRepresentation]);
	unlink([doc3 fileSystemRepresentation]);
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		NSString *bid = [[NSBundle mainBundle] bundleIdentifier];
		if ([bid isEqualToString:ID_FILES] || [bid isEqualToString:ID_FILE])
			return RunApp(bid, argc, argv);

		char path[4096];
		uint32_t len = sizeof(path);
		_NSGetExecutablePath(path, &len);
		NSString *exe = [[NSString stringWithUTF8String:path] stringByResolvingSymlinksInPath];
		// LaunchServices does not index applications under /tmp.
		NSString *apps = [NSHomeDirectory() stringByAppendingPathComponent:@"Applications"];
		[[NSFileManager defaultManager] createDirectoryAtPath:apps withIntermediateDirectories:YES attributes:nil error:NULL];
		char tmpl[4096];
		snprintf(tmpl, sizeof(tmpl), "%s/doclaunch.XXXXXX", [apps fileSystemRepresentation]);
		NSString *root = [NSString stringWithUTF8String:mkdtemp(tmpl)];
		Scenario(root, exe, ID_FILES, @"dlgdoca", @"openFiles", YES);
		Scenario(root, exe, ID_FILE, @"dlgdocb", @"openFile", NO);
		rmdir(tmpl);
		printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	}
	return g_failures != 0;
}
