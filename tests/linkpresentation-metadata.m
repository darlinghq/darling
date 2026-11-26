// LPLinkMetadata and LPMetadataProvider (LinkPresentation, documented in
// Apple's reference): metadata keeps what is set on it and survives copying
// and secure coding; Darling does not fetch metadata, so a fetch completes
// with an error of LPErrorDomain and no metadata.
#import <LinkPresentation/LinkPresentation.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s\n", #cond); failures++; } else printf("PASS: %s\n", #cond); } while (0)

int main(void) {
    @autoreleasepool {
        NSURL *url = [NSURL URLWithString: @"https://www.darlinghq.org/"];
        LPLinkMetadata *metadata = [[[LPLinkMetadata alloc] init] autorelease];
        metadata.URL = url;
        metadata.originalURL = url;
        metadata.title = @"Darling";
        CHECK([metadata.title isEqualToString: @"Darling"]);

        LPLinkMetadata *copy = [[metadata copy] autorelease];
        CHECK(copy != metadata && [copy.URL isEqual: url] && [copy.title isEqualToString: @"Darling"]);

        NSData *archive = [NSKeyedArchiver archivedDataWithRootObject: metadata requiringSecureCoding: YES error: NULL];
        CHECK(archive != nil);
        LPLinkMetadata *decoded = [NSKeyedUnarchiver unarchivedObjectOfClass: [LPLinkMetadata class] fromData: archive error: NULL];
        CHECK([decoded.originalURL isEqual: url] && [decoded.title isEqualToString: @"Darling"]);

        LPMetadataProvider *provider = [[[LPMetadataProvider alloc] init] autorelease];
        dispatch_semaphore_t done = dispatch_semaphore_create(0);
        __block NSError *fetchError = nil;
        __block LPLinkMetadata *fetched = (LPLinkMetadata *) 1;
        [provider startFetchingMetadataForURL: url completionHandler: ^(LPLinkMetadata *result, NSError *error) {
            fetched = result;
            fetchError = [error retain];
            dispatch_semaphore_signal(done);
        }];
        CHECK(dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) == 0);
        CHECK(fetched == nil);
        CHECK([fetchError.domain isEqualToString: LPErrorDomain] && fetchError.code == LPErrorMetadataFetchFailed);
        [fetchError release];
    }
    printf("%s\n", failures ? "FAILED" : "ALL PASSED");
    return failures != 0;
}
