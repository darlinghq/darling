#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>

@class CSKDirectoryObserver;
typedef void (^CSKDirectoryEventHandler)(CSKDirectoryObserver *observer, NSArray<NSString *> *paths, BOOL flag);

// Watches directories through FSEvents and reports the paths of the ones whose contents change.
@interface CSKDirectoryObserver : NSObject
@property (strong) dispatch_queue_t eventQueue;
@property (strong) id representedObject;
- (instancetype)initWithURLs:(NSArray<NSURL *> *)urls eventHandler:(CSKDirectoryEventHandler)handler;
- (void)start;
@end

@implementation CSKDirectoryObserver {
	NSArray<NSURL *> *_urls;
	CSKDirectoryEventHandler _handler;
	FSEventStreamRef _stream;
}

- (instancetype)initWithURLs:(NSArray<NSURL *> *)urls eventHandler:(CSKDirectoryEventHandler)handler
{
	self = [super init];
	if (self) {
		_urls = [urls copy];
		_handler = [handler copy];
	}
	return self;
}

// Console's handler also takes a flag whose meaning isn't known; it is always NO here.
static void streamCallback(ConstFSEventStreamRef stream, void *info, size_t count, void *paths,
	const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
	CSKDirectoryObserver *observer = (__bridge CSKDirectoryObserver *)info;
	NSMutableArray<NSString *> *changed = [NSMutableArray array];

	for (size_t i = 0; i < count; i++) {
		NSString *path = [NSString stringWithUTF8String:((char **)paths)[i]];
		if (![changed containsObject:path])
			[changed addObject:path];
	}
	observer->_handler(observer, changed, NO);
}

- (void)start
{
	if (self.eventQueue == nil)
		[NSException raise:NSInternalInconsistencyException format:@"%@ started without an event queue", self];
	if (_stream != NULL)
		return;

	NSMutableArray<NSString *> *paths = [NSMutableArray array];
	for (NSURL *url in _urls)
		[paths addObject:url.path];
	FSEventStreamContext context = { 0, (__bridge void *)self, NULL, NULL, NULL };
	_stream = FSEventStreamCreate(NULL, streamCallback, &context, (__bridge CFArrayRef)paths,
		kFSEventStreamEventIdSinceNow, 0, 0);
	if (_stream == NULL)
		[NSException raise:NSInternalInconsistencyException format:@"%@ could not create an FSEvents stream", self];
	FSEventStreamSetDispatchQueue(_stream, self.eventQueue);
	FSEventStreamStart(_stream);
}

- (void)dealloc
{
	if (_stream != NULL) {
		FSEventStreamStop(_stream);
		FSEventStreamInvalidate(_stream);
		FSEventStreamRelease(_stream);
	}
}

@end
