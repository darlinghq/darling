#import <ConsoleKit/CSKStreamSource.h>
#include <asl.h>
#include <stdlib.h>

@interface CSKStreamASLSource : NSObject <CSKStreamSource>
@property (readonly, copy) NSURL *fileURL;
- (instancetype)initWithFileURL:(NSURL *)fileURL;
@end

static NSString *CSKASLString(asl_object_t msg, const char *key)
{
	const char *value = asl_get(msg, key);
	return value ? @(value) : nil;
}

@implementation CSKStreamASLSource

- (instancetype)initWithFileURL:(NSURL *)fileURL
{
	self = [super init];
	if (self)
		_fileURL = [fileURL copy];
	return self;
}

- (NSArray<CSKMessage *> *)loadMessagesWithLimit:(NSUInteger)limit error:(NSError **)error
{
	// A NULL path would open the system database instead.
	asl_object_t store = _fileURL.isFileURL ? asl_open_path(_fileURL.fileSystemRepresentation, 0) : NULL;
	if (store == NULL) {
		if (error)
			*error = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileReadCorruptFileError
				userInfo:_fileURL ? @{NSURLErrorKey: _fileURL} : nil];
		return nil;
	}

	// Newest `limit` records, newest first; a NULL result means none matched.
	size_t last = 0;
	asl_object_t results = limit ? asl_match(store, NULL, &last, SIZE_MAX, MIN(limit, UINT32_MAX), 0, -1) : NULL;
	NSMutableArray<CSKMessage *> *messages = [NSMutableArray array];
	asl_object_t msg;
	while (results != NULL && (msg = asl_next(results)) != NULL) {
		const char *time = asl_get(msg, ASL_KEY_TIME);
		const char *nano = asl_get(msg, ASL_KEY_TIME_NSEC);
		NSDate *date = nil;
		if (time)
			date = [NSDate dateWithTimeIntervalSince1970:strtoll(time, NULL, 10) + (nano ? strtoll(nano, NULL, 10) / 1e9 : 0)];
		const char *pid = asl_get(msg, ASL_KEY_PID);
		const char *level = asl_get(msg, ASL_KEY_LEVEL);
		// Darling's os_log records its subsystem and category under these keys (libtrace os_log.c);
		// plain ASL clients classify by facility instead.
		[messages addObject:[[CSKMessage alloc] initWithDate:date
			sender:CSKASLString(msg, ASL_KEY_SENDER)
			processID:pid ? atoi(pid) : 0
			subsystem:CSKASLString(msg, "Subsystem") ?: CSKASLString(msg, ASL_KEY_FACILITY)
			category:CSKASLString(msg, "Category")
			level:level ? atoi(level) : ASL_LEVEL_NOTICE
			message:CSKASLString(msg, ASL_KEY_MSG) ?: @""]];
	}
	if (results != NULL)
		asl_release(results);
	asl_release(store);
	return [[messages reverseObjectEnumerator] allObjects];
}

@end
