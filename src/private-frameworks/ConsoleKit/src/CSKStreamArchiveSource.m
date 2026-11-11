#import <Foundation/Foundation.h>

@interface CSKStreamArchiveSource : NSObject
- (instancetype)initWithArchiveURL:(NSURL *)archiveURL error:(NSError **)error;
@end

@implementation CSKStreamArchiveSource

// .logarchive bundles hold unified-log trace files, whose format is not public; Darling records
// its logs through ASL and has no reader for them, so opening one fails with an error Console shows.
- (instancetype)initWithArchiveURL:(NSURL *)archiveURL error:(NSError **)error
{
	if (error) {
		NSMutableDictionary *info = [@{NSLocalizedDescriptionKey: @"Log archives can't be read on this system."} mutableCopy];
		if (archiveURL)
			info[NSURLErrorKey] = archiveURL;
		*error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadUnsupportedSchemeError userInfo:info];
	}
	return nil;
}

@end
