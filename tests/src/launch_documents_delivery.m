// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices/AppKit: opening a document launches the application registered for its
// type (CFBundleDocumentTypes) and hands it the document as a kAEOpenDocuments Apple Event, which
// NSApplication delivers to the delegate (application:openFiles: when implemented, otherwise
// application:openFile:) after applicationWillFinishLaunching:. A document opened while the
// application is running goes to the running instance; several documents arrive in one request;
// "open -a" sends the documents to the named application.
//
// The program builds two throw-away app bundles from its own executable, one per delegate
// method, registers them with LSRegisterURL, drives "open", and the bundles record what they
// received in a log file. Run with no arguments.
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

@interface DocApp : NSObject <NSApplicationDelegate>
@property (copy) NSString *bundleID;
@end

@implementation DocApp
- (void)applicationWillFinishLaunching:(NSNotification *)n
{
	AppendLog(self.bundleID, @"willFinishLaunching");
}
- (void)applicationDidFinishLaunching:(NSNotification *)n
{
	AppendLog(self.bundleID, @"didFinishLaunching");
}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)app
{
	return NO;
}
- (void)application:(NSApplication *)app openFiles:(NSArray<NSString *> *)files
{
	AppendLog(self.bundleID, [NSString stringWithFormat:@"openFiles %lu", (unsigned long) [files count]]);
	for (NSString *f in files)
		AppendLog(self.bundleID, [NSString stringWithFormat:@"file %@", f]);
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
	AppendLog(self.bundleID, @"openFile");
	AppendLog(self.bundleID, [NSString stringWithFormat:@"file %@", file]);
	return YES;
}
@end

static int RunApp(NSString *bundleID)
{
	@autoreleasepool {
		AppendLog(bundleID, [NSString stringWithFormat:@"pid %d", (int) getpid()]);
		NSApplication *app = [NSApplication sharedApplication];
		[app setActivationPolicy:NSApplicationActivationPolicyAccessory];
		DocApp *delegate = [bundleID isEqualToString:ID_FILE] ? [[SingleDocApp alloc] init] : [[DocApp alloc] init];
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

	// The type resolves to the bundle.
	CFURLRef handler = LSCopyDefaultApplicationURLForURL((__bridge CFURLRef) [NSURL fileURLWithPath:doc1], kLSRolesAll, NULL);
	NSString *handlerPath = handler ? [(__bridge NSURL *) handler path] : nil;
	printf("%s: handler: %s\n", [how UTF8String], handlerPath ? [handlerPath UTF8String] : "(none)");
	CHECK(handlerPath != nil && [handlerPath hasSuffix:[bundleID stringByAppendingString:@".app"]]);
	if (handler)
		CFRelease(handler);

	// App not running: launched by open, the document delivered after willFinishLaunching.
	int rc = Open(@[ doc1 ]);
	printf("%s: open #1 rc=%d\n", [how UTF8String], rc);
	CHECK(rc == 0);
	BOOL got = WaitForLog(bundleID, [@"file " stringByAppendingString:doc1], 20);
	NSString *log = ReadLog(bundleID);
	printf("%s: log after first open:\n%s", [how UTF8String], [log UTF8String]);
	CHECK(got);
	NSRange will = [log rangeOfString:@"willFinishLaunching"];
	NSRange file = [log rangeOfString:@"file "];
	CHECK(will.location != NSNotFound && file.location != NSNotFound && will.location < file.location);
	CHECK([log rangeOfString:batch ? @"openFiles 1" : @"openFile\n"].location != NSNotFound);

	// App already running: delivered to the same instance.
	int pid = pid_of(bundleID);
	rc = Open(@[ doc2 ]);
	printf("%s: open #2 rc=%d\n", [how UTF8String], rc);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, [@"file " stringByAppendingString:doc2], 20));

	// Several documents in one request, named application given explicitly.
	rc = Open(@[ @"-a", app, doc2, doc3 ]);
	printf("%s: open -a rc=%d\n", [how UTF8String], rc);
	CHECK(rc == 0);
	CHECK(WaitForLog(bundleID, [@"file " stringByAppendingString:doc3], 20));
	log = ReadLog(bundleID);
	printf("%s: log after third open:\n%s", [how UTF8String], [log UTF8String]);
	if (batch)
		CHECK([log rangeOfString:@"openFiles 2"].location != NSNotFound);

	int pids = 0;
	for (NSString *l in [log componentsSeparatedByString:@"\n"])
		if ([l hasPrefix:@"pid "])
			pids++;
	CHECK(pids == 1);

	if (pid > 0)
		kill(pid, SIGTERM);
	usleep(300000);
	unlink([LogPath(bundleID) fileSystemRepresentation]);
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
			return RunApp(bid);

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
