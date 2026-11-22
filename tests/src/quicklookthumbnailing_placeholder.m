// CFLAGS: -framework Foundation -framework QuickLookThumbnailing -framework UniformTypeIdentifiers
// Copyright (C) 2026 Darling developers
//
// QuickLookThumbnailing is a placeholder. QLThumbnailGenerationRequest stores what it is given, and every
// QLThumbnailGenerator generate or save method calls its handler asynchronously with no representation and an
// error in QLThumbnailErrorDomain whose underlying error is NSFeatureUnsupportedError in NSCocoaErrorDomain, so
// a caller is never left waiting. The type passed with an updateHandler error is the placeholder's own choice
// (the highest quality requested). Prints OK, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <QuickLookThumbnailing/QuickLookThumbnailing.h>
#import <UniformTypeIdentifiers/UTCoreTypes.h>
#include <pthread.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

typedef QLThumbnailGenerationRequestRepresentationTypes Types;

// Retained by the handlers, released by checkError.
static void checkError(const char *what, NSError *error, QLThumbnailError code)
{
	CHECK(error != nil, what);
	CHECK([[error domain] isEqualToString:QLThumbnailErrorDomain], what);
	CHECK([error code] == code, what);
	NSError *reason = [[error userInfo] objectForKey:NSUnderlyingErrorKey];
	CHECK([[reason domain] isEqualToString:NSCocoaErrorDomain], what);
	CHECK([reason code] == NSFeatureUnsupportedError, what);
	[error release];
}

static BOOL waitFor(dispatch_semaphore_t sem)
{
	return dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0;
}

static QLThumbnailGenerationRequest *makeRequest(NSString *path, Types types)
{
	return [[QLThumbnailGenerationRequest alloc] initWithFileAtURL:[NSURL fileURLWithPath:path]
	                                                          size:CGSizeMake(128, 64)
	                                                         scale:2.0
	                                           representationTypes:types];
}

// Runs generateRepresentations and returns the type its handler was given; -1 if it was not called.
static NSInteger updateType(QLThumbnailGenerator *generator, QLThumbnailGenerationRequest *request)
{
	__block NSInteger type = -1;
	__block NSError *error = nil;
	__block QLThumbnailRepresentation *thumbnail = nil;
	dispatch_semaphore_t sem = dispatch_semaphore_create(0);
	pthread_t caller = pthread_self();
	__block BOOL synchronous = NO;
	[generator generateRepresentationsForRequest:request updateHandler:^(QLThumbnailRepresentation *t, QLThumbnailRepresentationType ty, NSError *e) {
		synchronous = pthread_equal(pthread_self(), caller);
		thumbnail = t;
		type = ty;
		error = [e retain];
		dispatch_semaphore_signal(sem);
	}];
	if (!waitFor(sem)) {
		CHECK(NO, "generateRepresentations handler was not called");
		return -1;
	}
	CHECK(!synchronous, "generateRepresentations handler ran on the calling thread");
	CHECK(thumbnail == nil, "generateRepresentations returned a representation");
	checkError("generateRepresentations error", error, QLThumbnailErrorGenerationFailed);
	return type;
}

int main(void)
{
	@autoreleasepool {
		CHECK([QLThumbnailGenerator class] != Nil, "QLThumbnailGenerator is missing");
		CHECK([QLThumbnailGenerationRequest class] != Nil, "QLThumbnailGenerationRequest is missing");
		CHECK([QLThumbnailRepresentation class] != Nil, "QLThumbnailRepresentation is missing");
		CHECK([QLThumbnailErrorDomain length] > 0, "QLThumbnailErrorDomain is empty");

		QLThumbnailGenerationRequest *request = makeRequest(@"/tmp/quicklookthumbnailing-placeholder.txt", QLThumbnailGenerationRequestRepresentationTypeAll);
		CHECK(request.size.width == 128 && request.size.height == 64, "request size not stored");
		CHECK(request.scale == 2.0, "request scale not stored");
		CHECK(request.representationTypes == QLThumbnailGenerationRequestRepresentationTypeAll, "request representationTypes not stored");
		CHECK(request.minimumDimension == 0, "minimumDimension default is not 0");
		CHECK(request.iconMode == NO, "iconMode default is not NO");
		request.minimumDimension = 16;
		request.iconMode = YES;
		CHECK(request.minimumDimension == 16, "minimumDimension setter");
		CHECK(request.iconMode == YES, "iconMode setter");

		// contentType is null_resettable: never nil, derived from the file extension while unset.
		CHECK(request.contentType != nil, "contentType is nil when unset");
		CHECK([request.contentType conformsToType:UTTypeText], "contentType of a .txt file is not derived from the extension");
		request.contentType = UTTypePNG;
		CHECK([request.contentType isEqual:UTTypePNG], "contentType setter");
		request.contentType = nil;
		CHECK(request.contentType != nil, "contentType is nil after reset");
		CHECK([request.contentType conformsToType:UTTypeText], "contentType is not re-derived after reset");
		QLThumbnailGenerationRequest *bare = makeRequest(@"/tmp/quicklookthumbnailing-placeholder", QLThumbnailGenerationRequestRepresentationTypeIcon);
		CHECK(bare.contentType != nil, "contentType is nil for a file without an extension");

		// The numeric values and the domain string are the ones macOS 26.7.3 reports (black-box
		// observation); the enums are ABI, so a different number would change what callers compare.
		CHECK(QLThumbnailErrorGenerationFailed == 0, "QLThumbnailErrorGenerationFailed is not 0");
		CHECK(QLThumbnailErrorSavingToURLFailed == 1, "QLThumbnailErrorSavingToURLFailed is not 1");
		CHECK(QLThumbnailErrorNoCachedThumbnail == 2, "QLThumbnailErrorNoCachedThumbnail is not 2");
		CHECK(QLThumbnailErrorNoCloudThumbnail == 3, "QLThumbnailErrorNoCloudThumbnail is not 3");
		CHECK(QLThumbnailErrorRequestInvalid == 4, "QLThumbnailErrorRequestInvalid is not 4");
		CHECK(QLThumbnailErrorRequestCancelled == 5, "QLThumbnailErrorRequestCancelled is not 5");
		CHECK(QLThumbnailRepresentationTypeIcon == 0 && QLThumbnailRepresentationTypeLowQualityThumbnail == 1 &&
		      QLThumbnailRepresentationTypeThumbnail == 2, "QLThumbnailRepresentationType values are not 0, 1, 2");
		CHECK(QLThumbnailGenerationRequestRepresentationTypeIcon == 1 &&
		      QLThumbnailGenerationRequestRepresentationTypeLowQualityThumbnail == 2 &&
		      QLThumbnailGenerationRequestRepresentationTypeThumbnail == 4 &&
		      QLThumbnailGenerationRequestRepresentationTypeAll == 7, "request representation types are not 1, 2, 4, 7");
		CHECK([QLThumbnailErrorDomain isEqualToString:@"QLThumbnailErrorDomain"], "QLThumbnailErrorDomain string");

		QLThumbnailGenerator *generator = [QLThumbnailGenerator sharedGenerator];
		CHECK(generator != nil, "sharedGenerator is nil");
		CHECK(generator == [QLThumbnailGenerator sharedGenerator], "sharedGenerator is not a singleton");

		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		pthread_t caller = pthread_self();

		__block QLThumbnailRepresentation *bestThumbnail = nil;
		__block NSError *bestError = nil;
		__block BOOL bestSynchronous = NO;
		[generator generateBestRepresentationForRequest:request completionHandler:^(QLThumbnailRepresentation *thumbnail, NSError *error) {
			bestSynchronous = pthread_equal(pthread_self(), caller);
			bestThumbnail = thumbnail;
			bestError = [error retain];
			dispatch_semaphore_signal(sem);
		}];
		CHECK(waitFor(sem), "generateBest handler was not called");
		CHECK(!bestSynchronous, "generateBest handler ran on the calling thread");
		CHECK(bestThumbnail == nil, "generateBest returned a representation");
		checkError("generateBest error", bestError, QLThumbnailErrorGenerationFailed);

		// Highest quality requested: Thumbnail, then LowQualityThumbnail, then Icon.
		CHECK(updateType(generator, request) == QLThumbnailRepresentationTypeThumbnail, "update type for All");
		QLThumbnailGenerationRequest *icon = makeRequest(@"/tmp/a.txt", QLThumbnailGenerationRequestRepresentationTypeIcon);
		CHECK(updateType(generator, icon) == QLThumbnailRepresentationTypeIcon, "update type for Icon only");
		QLThumbnailGenerationRequest *low = makeRequest(@"/tmp/a.txt", QLThumbnailGenerationRequestRepresentationTypeIcon | QLThumbnailGenerationRequestRepresentationTypeLowQualityThumbnail);
		CHECK(updateType(generator, low) == QLThumbnailRepresentationTypeLowQualityThumbnail, "update type for Icon and LowQualityThumbnail");

		// An empty set is the placeholder's own case: it falls back to Thumbnail.
		QLThumbnailGenerationRequest *none = makeRequest(@"/tmp/a.txt", 0);
		CHECK(updateType(generator, none) == QLThumbnailRepresentationTypeThumbnail, "update type for an empty set");

		__block NSError *saveError = nil;
		__block BOOL saveSynchronous = NO;
		[generator saveBestRepresentationForRequest:request
		                                toFileAtURL:[NSURL fileURLWithPath:@"/tmp/quicklookthumbnailing-placeholder.png"]
		                            withContentType:@"public.png"
		                          completionHandler:^(NSError *error) {
			saveSynchronous = pthread_equal(pthread_self(), caller);
			saveError = [error retain];
			dispatch_semaphore_signal(sem);
		}];
		CHECK(waitFor(sem), "saveBest withContentType handler was not called");
		CHECK(!saveSynchronous, "saveBest withContentType handler ran on the calling thread");
		checkError("saveBest withContentType error", saveError, QLThumbnailErrorSavingToURLFailed);

		__block NSError *saveAsError = nil;
		[generator saveBestRepresentationForRequest:request
		                                toFileAtURL:[NSURL fileURLWithPath:@"/tmp/quicklookthumbnailing-placeholder.png"]
		                              asContentType:UTTypePNG
		                          completionHandler:^(NSError *error) {
			saveAsError = [error retain];
			dispatch_semaphore_signal(sem);
		}];
		CHECK(waitFor(sem), "saveBest asContentType handler was not called");
		checkError("saveBest asContentType error", saveAsError, QLThumbnailErrorSavingToURLFailed);

		// Handlers that are nil must be accepted (updateHandler is documented as optional).
		void (^noBest)(QLThumbnailRepresentation *, NSError *) = NULL;
		void (^noSave)(NSError *) = NULL;
		[generator generateRepresentationsForRequest:request updateHandler:nil];
		[generator generateBestRepresentationForRequest:request completionHandler:noBest];
		[generator saveBestRepresentationForRequest:request toFileAtURL:[NSURL fileURLWithPath:@"/tmp/x.png"] withContentType:@"public.png" completionHandler:noSave];
		[generator saveBestRepresentationForRequest:request toFileAtURL:[NSURL fileURLWithPath:@"/tmp/x.png"] asContentType:UTTypePNG completionHandler:noSave];

		[generator cancelRequest:request];
		[generator cancelRequest:request];

		[request release];
		[bare release];
		[icon release];
		[low release];
		[none release];
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
