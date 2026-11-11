// ConsoleKit log model and ASL store, driven the way Console drives them when it opens a file.
// Build against Foundation, AppKit and the private ConsoleKit framework; run inside a Darling guest.
#import <AppKit/AppKit.h>
#include <asl.h>
#include <stdio.h>
#include <sys/stat.h>

@interface CSKDevice : NSObject
+ (instancetype)hostDevice;
- (NSString *)name;
@end

@interface CSKFileSystem : NSObject
+ (BOOL)doesURLPointToValidFile:(NSURL *)url;
+ (BOOL)isFileAtPathValidASLFile:(NSString *)path;
+ (BOOL)isFileAtPathValidLogArchive:(NSString *)path;
+ (BOOL)isFileAtPathValidKtraceLogFile:(NSString *)path;
@end

@interface CSKHelpers : NSObject
+ (NSString *)formattedNumberFromUnsignedInteger:(NSUInteger)number;
@end

@interface CSKMessage : NSObject
@property (readonly, copy) NSDate *date;
@property (readonly, copy) NSString *sender;
@property (readonly) pid_t processID;
@property (readonly, copy) NSString *subsystem;
@property (readonly, copy) NSString *category;
@property (readonly) int level;
@property (readonly, copy) NSString *composedMessage;
@end

@interface CSKStreamASLSource : NSObject
- (instancetype)initWithFileURL:(NSURL *)url;
- (NSArray<CSKMessage *> *)loadMessagesWithLimit:(NSUInteger)limit error:(NSError **)error;
@end

@interface CSKStreamArchiveSource : NSObject
- (instancetype)initWithArchiveURL:(NSURL *)url error:(NSError **)error;
@end

@interface NSView (CSKLayout)
- (void)csk_activateTiedConstraintsToSuperview;
@end

static int failures;
#define CHECK(cond, ...) do { int ok_ = (cond); printf("%s: ", ok_ ? "PASS" : "FAIL"); printf(__VA_ARGS__); printf("\n"); failures += !ok_; } while (0)

static void writeASL(NSString *path, int count, time_t when)
{
	asl_object_t store = asl_open_path(path.fileSystemRepresentation, ASL_OPT_OPEN_WRITE | ASL_OPT_CREATE_STORE);
	for (int i = 0; i < count; i++) {
		asl_object_t msg = asl_new(ASL_TYPE_MSG);
		asl_set(msg, ASL_KEY_TIME, [NSString stringWithFormat:@"%ld", (long)when + i].UTF8String);
		asl_set(msg, ASL_KEY_TIME_NSEC, "500000000");
		asl_set(msg, ASL_KEY_SENDER, "consolekit-test");
		asl_set(msg, ASL_KEY_PID, "4242");
		asl_set(msg, ASL_KEY_LEVEL, i == 0 ? "3" : "5");
		asl_set(msg, "Subsystem", "org.darlinghq.test");
		asl_set(msg, "Category", "store");
		asl_set(msg, ASL_KEY_MSG, [NSString stringWithFormat:@"message %d", i].UTF8String);
		asl_append(store, msg);
		asl_release(msg);
	}
	asl_release(store);
}

static int openDescriptors(void)
{
	return (int)[[NSFileManager defaultManager] contentsOfDirectoryAtPath:@"/dev/fd" error:NULL].count;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		NSFileManager *fm = [NSFileManager defaultManager];
		NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID UUID].UUIDString];
		mkdir(dir.fileSystemRepresentation, 0755);

		CSKDevice *host = [CSKDevice hostDevice];
		CHECK(host == [CSKDevice hostDevice], "hostDevice is a singleton");
		CHECK([host.name isEqualToString:[NSProcessInfo processInfo].hostName], "host device name is the host name (%s)", host.name.UTF8String);

		NSString *asl = [dir stringByAppendingPathComponent:@"test.asl"];
		time_t when = 1700000000;
		writeASL(asl, 5, when);
		// A store keeps one ASL data file per day; use it as a single-file input.
		NSString *aslFile = nil;
		for (NSString *name in [fm contentsOfDirectoryAtPath:asl error:NULL])
			if ([name.pathExtension isEqualToString:@"asl"])
				aslFile = [asl stringByAppendingPathComponent:name];
		BOOL isDirectory = YES;
		[fm fileExistsAtPath:aslFile isDirectory:&isDirectory];
		NSString *text = [dir stringByAppendingPathComponent:@"plain.log"];
		[@"plain text\n" writeToFile:text atomically:YES encoding:NSUTF8StringEncoding error:NULL];
		NSString *archive = [dir stringByAppendingPathComponent:@"test.logarchive"];
		mkdir(archive.fileSystemRepresentation, 0755);

		CHECK([CSKFileSystem isFileAtPathValidASLFile:asl], "ASL store is recognised");
		CHECK(!isDirectory && [CSKFileSystem isFileAtPathValidASLFile:aslFile], "single ASL data file is recognised");
		NSString *missing = [dir stringByAppendingPathComponent:@"missing.asl"];
		CHECK(![CSKFileSystem isFileAtPathValidASLFile:missing] && ![fm fileExistsAtPath:missing], "missing path is not an ASL file and is not created");
		CHECK(![CSKFileSystem isFileAtPathValidASLFile:text], "plain text is not an ASL file");
		CHECK([CSKFileSystem isFileAtPathValidLogArchive:archive], ".logarchive directory is an archive");
		CHECK(![CSKFileSystem isFileAtPathValidLogArchive:text], "plain file is not an archive");
		CHECK(![CSKFileSystem isFileAtPathValidKtraceLogFile:text], "plain file is not a ktrace file");
		CHECK([CSKFileSystem doesURLPointToValidFile:[NSURL fileURLWithPath:text]], "readable file URL is valid");
		CHECK(![CSKFileSystem doesURLPointToValidFile:[NSURL fileURLWithPath:dir]], "directory URL is not a valid file");
		CHECK(![CSKFileSystem doesURLPointToValidFile:[NSURL URLWithString:@"https://example.com/x.log"]], "remote URL is not a valid file");

		CSKStreamASLSource *source = [[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:asl]];
		NSError *error = nil;
		NSArray<CSKMessage *> *all = [source loadMessagesWithLimit:100 error:&error];
		CHECK(all.count == 5 && error == nil, "ASL source loads every message (%lu)", (unsigned long)all.count);
		CSKMessage *first = all.firstObject;
		CHECK([first.composedMessage isEqualToString:@"message 0"] && [first.sender isEqualToString:@"consolekit-test"]
			&& first.processID == 4242 && first.level == 3 && [first.subsystem isEqualToString:@"org.darlinghq.test"]
			&& [first.category isEqualToString:@"store"], "message fields come from the ASL record");
		CHECK(first.date.timeIntervalSince1970 == when + 0.5, "message date is the record time with nanoseconds");
		NSArray<CSKMessage *> *newest = [source loadMessagesWithLimit:2 error:&error];
		CHECK(newest.count == 2 && [newest.lastObject.composedMessage isEqualToString:@"message 4"]
			&& [newest.firstObject.composedMessage isEqualToString:@"message 3"], "limit keeps the newest messages");

		NSArray<CSKMessage *> *single = [[[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:aslFile]] loadMessagesWithLimit:10 error:&error];
		CHECK(single.count == 5 && [single.firstObject.composedMessage isEqualToString:@"message 0"]
			&& [single.lastObject.composedMessage isEqualToString:@"message 4"], "single ASL data file loads oldest first");

		int fdsBefore = openDescriptors();
		CSKStreamASLSource *fileSource = [[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:aslFile]];
		for (int i = 0; i < 50; i++)
			[fileSource loadMessagesWithLimit:2 error:NULL];
		CHECK(openDescriptors() == fdsBefore, "repeated loads release the file (%d -> %d descriptors)", fdsBefore, openDescriptors());

		CSKStreamASLSource *bad = [[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:text]];
		error = nil;
		CHECK([bad loadMessagesWithLimit:10 error:&error] == nil && error != nil, "unreadable ASL file reports an error");
		error = nil;
		CHECK([[[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:missing]] loadMessagesWithLimit:10 error:&error] == nil
			&& error != nil && ![fm fileExistsAtPath:missing], "missing ASL file reports an error and is not created");

		error = nil;
		CHECK([[CSKStreamArchiveSource alloc] initWithArchiveURL:[NSURL fileURLWithPath:archive] error:&error] == nil && error != nil,
			"log archive reports that it can't be read");

		CHECK([[CSKHelpers formattedNumberFromUnsignedInteger:1234567] isEqualToString:
			[NSNumberFormatter localizedStringFromNumber:@1234567 numberStyle:NSNumberFormatterDecimalStyle]], "count is formatted with grouping");

		NSView *parent = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 200)];
		NSView *child = [[NSView alloc] initWithFrame:NSMakeRect(10, 10, 20, 20)];
		[parent addSubview:child];
		[child csk_activateTiedConstraintsToSuperview];
		CHECK(NSEqualRects(child.frame, parent.bounds), "tied view fills its superview");
		parent.frame = NSMakeRect(0, 0, 500, 400);
		CHECK(NSEqualRects(child.frame, parent.bounds), "tied view follows superview resizes");

		[fm removeItemAtPath:dir error:NULL];
		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
