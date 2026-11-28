// CFLAGS: -framework Foundation -framework BackgroundAssets
// Copyright (C) 2026 Darling developers
//
// The unmanaged side of the BackgroundAssets placeholder. BADownload and BAURLDownload record what they are
// given and archive and copy it; BADownloadManager has an empty queue, grants exclusive control, and cannot
// schedule, start or cancel a download, which it reports in BAErrorDomain; every completion handler is called
// exactly once, asynchronously. The numeric and string values of the constants are not documented and are not
// pinned. Assertions marked "placeholder choice" pin what the placeholder does where the documentation is silent,
// not the documented contract. Prints OK, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <BackgroundAssets/BackgroundAssets.h>
#include <pthread.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wnonnull"

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

static BOOL waitFor(dispatch_semaphore_t sem, int64_t nanoseconds)
{
	return dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, nanoseconds)) == 0;
}

// Runs start, which must pass done to a handler, and checks that the handler ran exactly once and not on the
// calling thread. Returns NO if it did not run.
static BOOL awaitHandler(const char *what, void (^start)(void (^done)(void)))
{
	dispatch_semaphore_t sem = dispatch_semaphore_create(0);
	pthread_t caller = pthread_self();
	__block BOOL onCaller = NO;
	void (^done)(void) = ^{
		if (pthread_equal(pthread_self(), caller)) {
			onCaller = YES;
		}
		dispatch_semaphore_signal(sem);
	};
	start(done);
	if (!waitFor(sem, 5 * NSEC_PER_SEC)) {
		printf("FAIL: %s: handler was not called\n", what);
		failures++;
		return NO;
	}
	if (waitFor(sem, NSEC_PER_SEC / 10)) {
		printf("FAIL: %s: handler was called more than once\n", what);
		failures++;
	}
	if (onCaller) {
		printf("FAIL: %s: handler ran on the calling thread\n", what);
		failures++;
	}
	return YES;
}

static void checkDownloads(void)
{
	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"https://example.com/a.zip"]];
	BAURLDownload *download = [[BAURLDownload alloc] initWithIdentifier:@"a"
	                                                            request:request
	                                                          essential:YES
	                                                           fileSize:10
	                                         applicationGroupIdentifier:@"group.x"
	                                                           priority:BADownloaderPriorityMax];
	CHECK([download isKindOfClass:[BADownload class]], "BAURLDownload is not a BADownload");
	CHECK([[download identifier] isEqualToString:@"a"], "identifier was not stored");
	CHECK([[download uniqueIdentifier] length] > 0, "uniqueIdentifier is empty");
	CHECK([download isEssential] == YES, "essential was not stored");
	CHECK([download priority] == BADownloaderPriorityMax, "priority was not stored");
	CHECK([download state] == BADownloadStateCreated, "a new download is not in the created state");

	BAURLDownload *other = [[BAURLDownload alloc] initWithIdentifier:@"a" request:request applicationGroupIdentifier:@"group.x"];
	CHECK(![[other uniqueIdentifier] isEqualToString:[download uniqueIdentifier]], "two downloads share a uniqueIdentifier");
	CHECK([other isEssential] == NO && [other priority] == BADownloaderPriorityDefault, "the short initializer is not non-essential at the default priority");
	CHECK(BADownloaderPriorityMin < BADownloaderPriorityDefault && BADownloaderPriorityDefault < BADownloaderPriorityMax, "the priorities are not ordered");

	BADownload *nonEssential = [download copyAsNonEssential];
	CHECK([nonEssential isKindOfClass:[BAURLDownload class]], "copyAsNonEssential did not keep the class");
	CHECK([nonEssential isEssential] == NO && [[nonEssential identifier] isEqualToString:@"a"], "copyAsNonEssential is wrong");
	CHECK([download isEssential] == YES, "copyAsNonEssential changed the original");
	BADownload *copy = [download copy];
	CHECK([copy isEssential] == YES && [[copy identifier] isEqualToString:@"a"] && [copy priority] == BADownloaderPriorityMax, "copy is wrong");
	// Placeholder choice: a copy is a new download object with its own uniqueIdentifier.
	CHECK([[copy uniqueIdentifier] length] > 0 && ![[copy uniqueIdentifier] isEqualToString:[download uniqueIdentifier]], "a copy shares a uniqueIdentifier");

	NSError *error = nil;
	NSData *archive = [NSKeyedArchiver archivedDataWithRootObject:download requiringSecureCoding:YES error:&error];
	CHECK(archive != nil, "a download did not archive");
	BADownload *decoded = archive ? [NSKeyedUnarchiver unarchivedObjectOfClass:[BAURLDownload class] fromData:archive error:&error] : nil;
	CHECK([decoded isKindOfClass:[BAURLDownload class]] && [[decoded identifier] isEqualToString:@"a"] && [decoded isEssential], "a download did not round trip");
	// Placeholder choice: archiving keeps the uniqueIdentifier.
	CHECK([decoded priority] == BADownloaderPriorityMax && [[decoded uniqueIdentifier] isEqualToString:[download uniqueIdentifier]], "a decoded download is wrong");

	BAAppExtensionInfo *info = [[BAAppExtensionInfo alloc] init];
	CHECK([info restrictedDownloadSizeRemaining] == nil && [info restrictedEssentialDownloadSizeRemaining] == nil, "extension info has values");
	CHECK([BAAppExtensionInfo supportsSecureCoding], "BAAppExtensionInfo does not support secure coding");
	NSData *infoArchive = [NSKeyedArchiver archivedDataWithRootObject:info requiringSecureCoding:YES error:&error];
	BAAppExtensionInfo *decodedInfo = infoArchive ? [NSKeyedUnarchiver unarchivedObjectOfClass:[BAAppExtensionInfo class] fromData:infoArchive error:&error] : nil;
	CHECK(decodedInfo != nil && [decodedInfo restrictedDownloadSizeRemaining] == nil, "extension info did not round trip");

	BADownloadManager *manager = [BADownloadManager sharedManager];
	CHECK(manager != nil && manager == [BADownloadManager sharedManager], "sharedManager is not a singleton");
	error = nil;
	CHECK([manager scheduleDownload:download error:&error] == NO, "a download was scheduled");
	CHECK([[error domain] isEqualToString:BAErrorDomain], "schedule error is not in BAErrorDomain");
	error = nil;
	CHECK([manager startForegroundDownload:download error:&error] == NO, "a foreground download was started");
	CHECK([[error domain] isEqualToString:BAErrorDomain], "foreground error is not in BAErrorDomain");
	error = nil;
	CHECK([manager cancelDownload:download error:&error] == NO, "a download was cancelled");
	CHECK([[error domain] isEqualToString:BAErrorDomain], "cancel error is not in BAErrorDomain");
	CHECK([manager scheduleDownload:download error:NULL] == NO, "a NULL error pointer was not accepted");

	error = nil;
	NSArray *current = [manager fetchCurrentDownloads:&error];
	CHECK(current != nil && [current count] == 0 && error == nil, "fetchCurrentDownloads is not empty");

	__block NSArray *fetched = nil;
	__block NSError *fetchError = nil;
	awaitHandler("fetchCurrentDownloads", ^(void (^done)(void)) {
		[manager fetchCurrentDownloadsWithCompletionHandler:^(NSArray<BADownload *> *downloads, NSError *e) {
			fetched = [downloads retain];
			fetchError = [e retain];
			done();
		}];
	});
	CHECK(fetched != nil && [fetched count] == 0 && fetchError == nil, "fetchCurrentDownloads did not deliver an empty queue");

	__block BOOL acquired = NO;
	__block NSError *lockError = (NSError *)@"unset";
	awaitHandler("performWithExclusiveControl", ^(void (^done)(void)) {
		[manager performWithExclusiveControl:^(BOOL acquiredLock, NSError *e) {
			acquired = acquiredLock;
			lockError = e;
			done();
		}];
	});
	CHECK(acquired && lockError == nil, "exclusive control was not granted");
	acquired = NO;
	lockError = (NSError *)@"unset";
	awaitHandler("performWithExclusiveControlBeforeDate", ^(void (^done)(void)) {
		[manager performWithExclusiveControlBeforeDate:[NSDate dateWithTimeIntervalSinceNow:10] performHandler:^(BOOL acquiredLock, NSError *e) {
			acquired = acquiredLock;
			lockError = e;
			done();
		}];
	});
	CHECK(acquired && lockError == nil, "exclusive control before a date was not granted");

	[manager fetchCurrentDownloadsWithCompletionHandler:nil];
	[manager performWithExclusiveControl:nil];

	[info release];
	[download release];
	[other release];
	[nonEssential release];
	[copy release];
}

int main(void)
{
	@autoreleasepool {
		CHECK([BADownloadManager class] != Nil, "BADownloadManager is missing");
		CHECK([BADownload class] != Nil, "BADownload is missing");
		CHECK([BAURLDownload class] != Nil, "BAURLDownload is missing");
		CHECK([BAAppExtensionInfo class] != Nil, "BAAppExtensionInfo is missing");
		CHECK([BAURLDownload superclass] == [BADownload class], "BAURLDownload is not a subclass of BADownload");
		CHECK([BAErrorDomain length] > 0, "BAErrorDomain is empty");
		CHECK(![BAErrorDomain isEqualToString:BAManagedErrorDomain], "the error domains are equal");

		checkDownloads();
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
