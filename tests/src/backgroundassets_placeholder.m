// CFLAGS: -framework Foundation -framework BackgroundAssets
// Copyright (C) 2026 Darling developers
//
// BackgroundAssets is a placeholder. No asset pack exists, nothing is downloaded and the network and the file
// system are never touched. BAAssetPackManager reports an empty manifest and no asset packs, the documented
// not-found errors for an identifier or a file, and an unavailable set of packs; every completion handler is
// called exactly once, asynchronously. The numeric and string values of the constants are not documented and
// are not pinned. Assertions marked "placeholder choice" pin what the placeholder does where the documentation is
// silent (the GUESS comments in src/BAAssetPackManager.m), not the documented contract. Prints OK, or the
// failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <BackgroundAssets/BackgroundAssets.h>
#include <pthread.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wnonnull"

static int failures;

@interface Delegate : NSObject <BAManagedAssetPackDownloadDelegate>
@end

@implementation Delegate
@end

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

static void checkManaged(const char *what, NSError *error, BAManagedErrorCode code)
{
	CHECK(error != nil, what);
	CHECK([[error domain] isEqualToString:BAManagedErrorDomain], what);
	CHECK([error code] == code, what);
}

static void checkNotFound(const char *what, NSError *error, NSString *identifier)
{
	checkManaged(what, error, BAManagedErrorCodeAssetPackNotFound);
	CHECK([[[error userInfo] objectForKey:BAAssetPackIdentifierErrorKey] isEqual:identifier], what);
}

static void checkFileNotFound(const char *what, NSError *error, NSString *path)
{
	checkManaged(what, error, BAManagedErrorCodeFileNotFound);
	CHECK([[[error userInfo] objectForKey:NSFilePathErrorKey] isEqual:path], what);
}

static void checkManager(void)
{
	BAAssetPackManager *manager = [BAAssetPackManager sharedManager];
	CHECK(manager != nil, "sharedManager is nil");
	CHECK(manager == [BAAssetPackManager sharedManager], "sharedManager is not a singleton");

	__block BAAssetPackManifest *manifest = nil;
	__block NSError *error = nil;
	awaitHandler("getManifest", ^(void (^done)(void)) {
		[manager getManifestWithCompletionHandler:^(BAAssetPackManifest *m, NSError *e) {
			manifest = [m retain];
			error = [e retain];
			done();
		}];
	});
	CHECK(manifest != nil && error == nil, "getManifest did not deliver an empty manifest");
	CHECK([[manifest assetPacks] count] == 0, "the manifest has asset packs");
	CHECK([[manifest allDownloads] count] == 0, "the manifest has downloads");
	CHECK([[manifest allDownloadsForContentRequest:BAContentRequestInstall] count] == 0, "the manifest has downloads for a request");
	CHECK([[manifest localizedAssetPacks] count] == 0, "the manifest has localized asset packs");
	CHECK([[manifest localizedAssetPacksForLanguage:@"en"] count] == 0, "the manifest has asset packs for a language");
	CHECK([[manifest availableLanguages] count] == 0, "the manifest has languages");
	CHECK([manifest assetPackWithIdentifier:@"x"] == nil, "the manifest found an asset pack");
	CHECK([manifest primaryLanguage] == nil && [manifest resolvedLanguage] == nil, "the manifest has a language");

	__block NSSet *packs = nil;
	error = nil;
	awaitHandler("getAllAssetPacks", ^(void (^done)(void)) {
		[manager getAllAssetPacksWithCompletionHandler:^(NSSet<BAAssetPack *> *p, NSError *e) {
			packs = [p retain];
			error = [e retain];
			done();
		}];
	});
	CHECK(packs != nil && [packs count] == 0 && error == nil, "getAllAssetPacks did not deliver an empty set");

	__block BAAssetPack *pack = (BAAssetPack *)@"unset";
	error = nil;
	awaitHandler("getAssetPack", ^(void (^done)(void)) {
		[manager getAssetPackWithIdentifier:@"x" completionHandler:^(BAAssetPack *p, NSError *e) {
			pack = p;
			error = [e retain];
			done();
		}];
	});
	CHECK(pack == nil, "getAssetPack found an asset pack");
	checkNotFound("getAssetPack error", error, @"x");

	NSError *failure = nil;
	NSData *data = [manager contentsAtPath:@"a/b.txt" searchingInAssetPackWithIdentifier:nil options:0 error:&failure];
	CHECK(data == nil, "contentsAtPath returned data");
	checkFileNotFound("contentsAtPath error", failure, @"a/b.txt");
	failure = nil;
	data = [manager contentsAtPath:@"a/b.txt" searchingInAssetPackWithIdentifier:@"x" options:0 error:&failure];
	CHECK(data == nil, "contentsAtPath in a pack returned data");
	checkFileNotFound("contentsAtPath in a pack error", failure, @"a/b.txt");
	failure = nil;
	data = [manager contentsAtPath:@"a/b.txt" asLocalizedForLanguage:@"en" options:0 error:&failure];
	CHECK(data == nil, "localized contentsAtPath returned data");
	checkFileNotFound("localized contentsAtPath error", failure, @"a/b.txt");
	failure = nil;
	CHECK([manager fileDescriptorForPath:@"a/b.txt" searchingInAssetPackWithIdentifier:nil error:&failure] == -1, "fileDescriptorForPath did not return -1");
	checkFileNotFound("fileDescriptorForPath error", failure, @"a/b.txt");
	failure = nil;
	CHECK([manager fileDescriptorForPath:@"a/b.txt" asLocalizedForLanguage:@"en" error:&failure] == -1, "localized fileDescriptorForPath did not return -1");
	checkFileNotFound("localized fileDescriptorForPath error", failure, @"a/b.txt");
	CHECK([manager contentsAtPath:@"a" searchingInAssetPackWithIdentifier:nil options:0 error:NULL] == nil, "a NULL error pointer was not accepted");
	CHECK([manager fileDescriptorForPath:@"a" searchingInAssetPackWithIdentifier:nil error:NULL] == -1, "a NULL error pointer was not accepted for a descriptor");

	// The documentation allows a URL for an item that does not exist, and says reading it fails.
	failure = nil;
	NSURL *url = [manager URLForPath:@"a/b.txt" error:&failure];
	CHECK(url != nil && failure == nil, "URLForPath did not return a URL");
	CHECK([url isFileURL] && [[url path] hasSuffix:@"a/b.txt"], "URLForPath is not a file URL for the path");
	CHECK([[NSFileManager defaultManager] fileExistsAtPath:[url path]] == NO, "URLForPath names an existing item");
	url = [manager URLForPath:@"a/b.txt" asLocalizedForLanguage:@"en" error:&failure];
	CHECK(url != nil && failure == nil, "localized URLForPath did not return a URL");
	// Placeholder choice: a path that could leave the empty root is refused.
	failure = nil;
	CHECK([manager URLForPath:@"../../etc/passwd" error:&failure] == nil, "URLForPath returned a URL for a path with ..");
	checkFileNotFound("URLForPath .. error", failure, @"../../etc/passwd");

	__block NSSet *updating = nil;
	__block NSSet *removed = nil;
	error = nil;
	awaitHandler("checkForUpdates", ^(void (^done)(void)) {
		[manager checkForUpdatesWithCompletionHandler:^(NSSet<NSString *> *u, NSSet<NSString *> *r, NSError *e) {
			updating = [u retain];
			removed = [r retain];
			error = [e retain];
			done();
		}];
	});
	CHECK(updating != nil && [updating count] == 0 && removed != nil && [removed count] == 0 && error == nil, "checkForUpdates did not report nothing");

	CHECK([manager assetPackIsAvailableLocallyWithIdentifier:@"x"] == NO, "an asset pack is available locally");
	failure = nil;
	CHECK([manager localVersionOfAssetPackWithIdentifier:@"x" error:&failure] == -1, "localVersion did not return -1");
	checkNotFound("localVersion error", failure, @"x");

	// A status that includes no case is zero.
	__block BAAssetPackStatus status = BAAssetPackStatusDownloaded;
	awaitHandler("getLocalStatus", ^(void (^done)(void)) {
		[manager getLocalStatusOfAssetPackWithIdentifier:@"x" completionHandler:^(BAAssetPackStatus s) {
			status = s;
			done();
		}];
	});
	CHECK(status == 0, "the local status of an unknown asset pack is not empty");

	status = BAAssetPackStatusDownloaded;
	error = nil;
	awaitHandler("getStatusOf", ^(void (^done)(void)) {
		[manager getStatusOfAssetPackWithIdentifier:@"x" completionHandler:^(BAAssetPackStatus s, NSError *e) {
			status = s;
			error = [e retain];
			done();
		}];
	});
	CHECK(status == 0, "the status of an unknown asset pack is not empty");
	checkNotFound("getStatusOf error", error, @"x");

	// Placeholder choice: not found.
	error = nil;
	awaitHandler("removeAssetPack", ^(void (^done)(void)) {
		[manager removeAssetPackWithIdentifier:@"x" completionHandler:^(NSError *e) {
			error = [e retain];
			done();
		}];
	});
	checkNotFound("removeAssetPack error", error, @"x");

	__block long long size = 0;
	error = nil;
	awaitHandler("getLocalSize", ^(void (^done)(void)) {
		[manager getLocalSizeOfAssetPackWithIdentifier:@"x" calculationMethod:BASizeCalculationMethodLogical completionHandler:^(long long s, NSError *e) {
			size = s;
			error = [e retain];
			done();
		}];
	});
	checkNotFound("getLocalSize error", error, @"x");
	// Placeholder choice: the size of a failed result is -1.
	CHECK(size == -1, "the local size of an unknown asset pack is not -1");

	__block NSArray *languages = nil;
	awaitHandler("getLocallyAvailableLanguages", ^(void (^done)(void)) {
		[manager getLocallyAvailableLanguagesWithCompletionHandler:^(NSArray<NSString *> *l) {
			languages = [l retain];
			done();
		}];
	});
	CHECK(languages != nil && [languages count] == 0, "languages are available locally");

	error = (NSError *)@"unset";
	awaitHandler("reconcilePreferredLanguages", ^(void (^done)(void)) {
		[manager reconcilePreferredLanguagesWithCompletionHandler:^(NSError *e) {
			error = e;
			done();
		}];
	});
	CHECK(error == nil, "reconcilePreferredLanguages reported an error");

	// An instance can only come from alloc and init: an empty asset pack with an empty identifier.
	BAAssetPack *empty = [[BAAssetPack alloc] init];
	CHECK([[empty identifier] isEqualToString:@""], "an empty asset pack has a non-empty identifier");
	CHECK([empty version] == 0 && [empty downloadSize] == 0 && [empty userInfo] == nil && [empty language] == nil, "an empty asset pack has values");
	BADownload *packDownload = [empty download];
	CHECK(packDownload != nil && [packDownload state] == BADownloadStateCreated, "an asset pack's download is not a created download");
	CHECK([empty downloadForContentRequest:BAContentRequestUpdate] != nil, "downloadForContentRequest returned nil");

	// Placeholder choice: a single asset pack that is not known is not found.
	error = (NSError *)@"unset";
	awaitHandler("ensureLocalAvailability", ^(void (^done)(void)) {
		[manager ensureLocalAvailabilityOfAssetPack:empty completionHandler:^(NSError *e) {
			error = [e retain];
			done();
		}];
	});
	checkManaged("ensureLocalAvailability error", error, BAManagedErrorCodeAssetPackNotFound);
	error = (NSError *)@"unset";
	awaitHandler("ensureLocalAvailability requireLatestVersion", ^(void (^done)(void)) {
		[manager ensureLocalAvailabilityOfAssetPack:empty requireLatestVersion:YES completionHandler:^(NSError *e) {
			error = [e retain];
			done();
		}];
	});
	checkManaged("ensureLocalAvailability requireLatestVersion error", error, BAManagedErrorCodeAssetPackNotFound);

	error = (NSError *)@"unset";
	awaitHandler("ensureLocalAvailability of no packs", ^(void (^done)(void)) {
		[manager ensureLocalAvailabilityOfAssetPacks:[NSSet set] completionHandler:^(NSError *e) {
			error = e;
			done();
		}];
	});
	CHECK(error == nil, "ensureLocalAvailability of no asset packs reported an error");
	error = nil;
	awaitHandler("ensureLocalAvailability of packs", ^(void (^done)(void)) {
		[manager ensureLocalAvailabilityOfAssetPacks:[NSSet setWithObject:empty] requireLatestVersions:NO completionHandler:^(NSError *e) {
			error = [e retain];
			done();
		}];
	});
	checkManaged("ensureLocalAvailability of packs error", error, BAManagedErrorCodeLocalAvailabilityFailure);
	NSDictionary *failureMap = [[error userInfo] objectForKey:BAFailuresErrorKey];
	CHECK([failureMap isKindOfClass:[NSDictionary class]] && [failureMap count] == 1, "the failures are not a dictionary of one asset pack");
	checkManaged("the failure of the asset pack", [failureMap objectForKey:empty], BAManagedErrorCodeAssetPackNotFound);
	NSSet *successes = [[error userInfo] objectForKey:BASuccessesErrorKey];
	CHECK([successes isKindOfClass:[NSSet class]] && [successes count] == 0, "the successes are not an empty set");

	CHECK([manager resolvedLanguage] == nil, "resolvedLanguage is not nil at first");
	manager.resolvedLanguage = @"en";
	CHECK([[manager resolvedLanguage] isEqualToString:@"en"], "resolvedLanguage was not stored");
	manager.resolvedLanguage = nil;
	CHECK([manager resolvedLanguage] == nil, "resolvedLanguage was not reset");

	CHECK([manager delegate] == nil, "the delegate is not nil at first");
	Delegate *delegate = [[Delegate alloc] init];
	manager.delegate = delegate;
	CHECK([manager delegate] == delegate, "the delegate was not stored");
	manager.delegate = nil;
	[delegate release];

	// A manifest cannot be read from a file or from data.
	NSError *manifestError = nil;
	BAAssetPackManifest *none = [[BAAssetPackManifest alloc] initFromData:[NSData data] applicationGroupIdentifier:@"group.x" error:&manifestError];
	CHECK(none == nil && manifestError != nil, "a manifest was made from data");
	manifestError = nil;
	none = [[BAAssetPackManifest alloc] initWithContentsOfURL:[NSURL fileURLWithPath:@"/nonexistent/manifest.json"] applicationGroupIdentifier:@"group.x" error:&manifestError];
	CHECK(none == nil && manifestError != nil, "a manifest was made from a file");

	// nil handlers are accepted.
	[manager getManifestWithCompletionHandler:nil];
	[manager getAssetPackWithIdentifier:@"x" completionHandler:nil];
	[manager removeAssetPackWithIdentifier:@"x" completionHandler:nil];
	[empty release];
}

int main(void)
{
	@autoreleasepool {
		CHECK([BAAssetPackManager class] != Nil, "BAAssetPackManager is missing");
		CHECK([BAAssetPack class] != Nil, "BAAssetPack is missing");
		CHECK([BAAssetPackManifest class] != Nil, "BAAssetPackManifest is missing");
		CHECK([BAAssetPackManager superclass] == [NSObject class], "BAAssetPackManager is not an NSObject subclass");

		CHECK([BAManagedErrorDomain length] > 0, "BAManagedErrorDomain is empty");
		CHECK([BAAssetPackIdentifierErrorKey length] > 0 && [BAFailuresErrorKey length] > 0 && [BASuccessesErrorKey length] > 0, "an error key is empty");
		CHECK(![BAFailuresErrorKey isEqualToString:BASuccessesErrorKey], "the failures and successes keys are equal");

		// The status values are an option set: nonzero and pairwise disjoint.
		BAAssetPackStatus all[] = {BAAssetPackStatusDownloadAvailable, BAAssetPackStatusDownloading, BAAssetPackStatusDownloaded,
			BAAssetPackStatusUpToDate, BAAssetPackStatusOutOfDate, BAAssetPackStatusObsolete, BAAssetPackStatusUpdateAvailable};
		for (int i = 0; i < 7; i++) {
			CHECK(all[i] != 0, "a status value is empty");
			for (int j = i + 1; j < 7; j++) {
				CHECK((all[i] & all[j]) == 0, "two status values share a bit");
			}
		}

		checkManager();
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
