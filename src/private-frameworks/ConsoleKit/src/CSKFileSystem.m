#import <Foundation/Foundation.h>
#include <asl.h>

@interface CSKFileSystem : NSObject
+ (instancetype)sharedInstance;
+ (BOOL)doesURLPointToValidFile:(NSURL *)url;
+ (BOOL)isFileAtPathValidASLFile:(NSString *)path;
+ (BOOL)isFileAtPathValidLogArchive:(NSString *)path;
+ (BOOL)isFileAtPathValidKtraceLogFile:(NSString *)path;
@end

@implementation CSKFileSystem

+ (instancetype)sharedInstance
{
	static CSKFileSystem *shared;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		shared = [[CSKFileSystem alloc] init];
	});
	return shared;
}

+ (BOOL)doesURLPointToValidFile:(NSURL *)url
{
	if (!url.isFileURL)
		return NO;
	BOOL isDirectory = NO;
	if (![[NSFileManager defaultManager] fileExistsAtPath:url.path isDirectory:&isDirectory] || isDirectory)
		return NO;
	return [[NSFileManager defaultManager] isReadableFileAtPath:url.path];
}

+ (BOOL)isFileAtPathValidASLFile:(NSString *)path
{
	if (path == nil)
		return NO;
	asl_object_t store = asl_open_path(path.fileSystemRepresentation, 0);
	if (store == NULL)
		return NO;
	asl_release(store);
	return YES;
}

// A log archive is a directory bundle with the .logarchive extension (see log(1)).
+ (BOOL)isFileAtPathValidLogArchive:(NSString *)path
{
	BOOL isDirectory = NO;
	return [path.pathExtension caseInsensitiveCompare:@"logarchive"] == NSOrderedSame
		&& [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDirectory] && isDirectory;
}

// Darling cannot decode kernel trace files, so none is reported as one Console could open.
+ (BOOL)isFileAtPathValidKtraceLogFile:(NSString *)path
{
	return NO;
}

@end
