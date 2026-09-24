// ConsoleKit calls Console makes while it builds its sources list and filter bar: message type
// names, filters, the shared file system, the device manager and directory observers.
// Build against Foundation, AppKit and ConsoleKit.
#import <AppKit/AppKit.h>
#include <stdio.h>

@interface CSKMessage : NSObject
+ (NSString *)localizedMessageTypeNameForType:(NSInteger)type;
@end

@interface CSKFilter : NSObject
@property (readonly) NSInteger type;
@property (readonly, copy) id value;
- (instancetype)initWithType:(NSInteger)type value:(id)value;
@end

@interface CSKFileSystem : NSObject
+ (instancetype)sharedInstance;
@end

@interface CSKDevice : NSObject
+ (instancetype)hostDevice;
- (BOOL)isEqualToDevice:(CSKDevice *)device;
@end

@interface CSKDeviceManager : NSObject
@property (assign) id delegate;
@property (readonly, copy) NSArray *allDevices;
@end

@interface CSKDirectoryObserver : NSObject
@property (strong) dispatch_queue_t eventQueue;
@property (strong) id representedObject;
- (instancetype)initWithURLs:(NSArray *)urls eventHandler:(void (^)(CSKDirectoryObserver *, NSArray *, BOOL))handler;
- (void)start;
@end

static int failures;
#define CHECK(cond, ...) do { int ok_ = (cond); printf("%s: ", ok_ ? "PASS" : "FAIL"); printf(__VA_ARGS__); printf("\n"); failures += !ok_; } while (0)

static BOOL raises(void (^block)(void))
{
	@try {
		block();
	} @catch (NSException *e) {
		return YES;
	}
	return NO;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		// Console's filter bar asks for types 4 and 5, then makes type-7 filters from the names.
		NSString *error = [CSKMessage localizedMessageTypeNameForType:4];
		NSString *fault = [CSKMessage localizedMessageTypeNameForType:5];
		CHECK([error isEqual:@"Error"] && [fault isEqual:@"Fault"], "message type names");
		CHECK(raises(^{ [CSKMessage localizedMessageTypeNameForType:3]; }), "unknown message type raises");
		CSKFilter *filter = [[CSKFilter alloc] initWithType:7 value:error];
		CHECK(filter.type == 7 && [filter.value isEqual:@"Error"], "filter keeps its type and value");

		CSKFileSystem *fs = [CSKFileSystem sharedInstance];
		CHECK(fs != nil && fs == [CSKFileSystem sharedInstance], "shared file system");

		NSObject *delegate = [NSObject new];
		CSKDeviceManager *devices = [CSKDeviceManager new];
		devices.delegate = delegate;
		CHECK(devices.delegate == delegate, "device manager delegate");
		CHECK([devices.allDevices isEqual:@[[CSKDevice hostDevice]]], "the host is the only device");
		CHECK([[devices.allDevices firstObject] isEqualToDevice:[CSKDevice hostDevice]] && ![[CSKDevice hostDevice] isEqualToDevice:nil],
			"device equality");

		NSString *dir = [NSHomeDirectory() stringByAppendingPathComponent:
			[NSString stringWithFormat:@"Library/Logs/consolekit-sources-%d", getpid()]];
		[[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:NULL];
		NSArray *urls = @[[NSURL fileURLWithPath:dir], [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"missing"]]];

		dispatch_semaphore_t fired = dispatch_semaphore_create(0);
		__block CSKDirectoryObserver *seenObserver;
		__block NSArray *seenPaths;
		__block BOOL seenFlag = YES;
		CSKDirectoryObserver *observer = [[CSKDirectoryObserver alloc] initWithURLs:urls
			eventHandler:^(CSKDirectoryObserver *o, NSArray *paths, BOOL flag) {
				seenObserver = o;
				seenPaths = [paths copy];
				seenFlag = flag;
				dispatch_semaphore_signal(fired);
			}];
		CHECK(raises(^{ [observer start]; }), "start without an event queue raises");
		observer.eventQueue = dispatch_queue_create("consolekit-sources", DISPATCH_QUEUE_SERIAL);
		observer.representedObject = delegate;
		CHECK(observer.representedObject == delegate, "represented object kept");
		[observer start];
		BOOL wrote = [@"x" writeToFile:[dir stringByAppendingPathComponent:@"new.log"] atomically:NO encoding:NSUTF8StringEncoding error:NULL];
		CHECK(wrote, "wrote a file into %s", dir.fileSystemRepresentation);
		long timedOut = dispatch_semaphore_wait(fired, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
		CHECK(!timedOut && seenObserver == observer && [seenPaths isEqual:@[dir]] && !seenFlag,
			"adding a file reports the directory (%s)", seenPaths ? [[seenPaths description] UTF8String] : "no event");

		[[NSFileManager defaultManager] removeItemAtPath:dir error:NULL];
		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
