// ConsoleKit stream view, driven with the calls Console makes when it opens an ASL file.
// Build against Foundation, AppKit and the private ConsoleKit framework; run inside a Darling guest.
#import <AppKit/AppKit.h>
#include <asl.h>
#include <stdio.h>
#include <sys/stat.h>

@interface CSKStreamASLSource : NSObject
- (instancetype)initWithFileURL:(NSURL *)url;
@end

@interface CSKTableColumnLayout : NSObject
@end

@interface CSKASLTableColumnLayout : CSKTableColumnLayout
@end

@interface CSKStreamViewController : NSViewController
@property (copy) NSString *messagesColumnLayoutAutosaveName;
@property (weak) id delegate;
@property BOOL wantsNowMode;
@property BOOL showsActivities;
@property BOOL showsDetailsPane;
@property (readonly, copy) NSArray *currentFilters;
@property (readonly, copy) NSError *loadError;
- (instancetype)initWithStreamSource:(id)source capacity:(NSUInteger)capacity;
- (void)updateMessagesColumnLayoutWithoutInvalidateAndSave:(CSKTableColumnLayout *)layout;
- (void)reload;
@end

static int failures;
#define CHECK(cond, ...) do { int ok_ = (cond); printf("%s: ", ok_ ? "PASS" : "FAIL"); printf(__VA_ARGS__); printf("\n"); failures += !ok_; } while (0)

static NSTableView *findTable(NSView *view)
{
	if ([view isKindOfClass:[NSTableView class]])
		return (NSTableView *)view;
	for (NSView *subview in view.subviews) {
		NSTableView *table = findTable(subview);
		if (table)
			return table;
	}
	return nil;
}

static CSKStreamViewController *openStream(NSString *path, NSUInteger capacity)
{
	// Console's sequence for an ASL document window.
	CSKStreamViewController *controller = [[CSKStreamViewController alloc]
		initWithStreamSource:[[CSKStreamASLSource alloc] initWithFileURL:[NSURL fileURLWithPath:path]] capacity:capacity];
	controller.messagesColumnLayoutAutosaveName = @"Console-ASLView-Messages";
	[controller updateMessagesColumnLayoutWithoutInvalidateAndSave:[CSKASLTableColumnLayout new]];
	controller.wantsNowMode = YES;
	controller.showsActivities = NO;
	controller.showsDetailsPane = NO;
	[controller reload];
	return controller;
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		[NSApplication sharedApplication];
		NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:[NSUUID UUID].UUIDString];
		mkdir(dir.fileSystemRepresentation, 0755);
		NSString *path = [dir stringByAppendingPathComponent:@"stream.asl"];
		asl_object_t store = asl_open_path(path.fileSystemRepresentation, ASL_OPT_OPEN_WRITE | ASL_OPT_CREATE_STORE);
		for (int i = 0; i < 4; i++) {
			asl_object_t msg = asl_new(ASL_TYPE_MSG);
			asl_set(msg, ASL_KEY_TIME, [NSString stringWithFormat:@"%d", 1700000000 + i].UTF8String);
			asl_set(msg, ASL_KEY_SENDER, "streamer");
			asl_set(msg, ASL_KEY_PID, "77");
			asl_set(msg, "Subsystem", "org.darlinghq.stream");
			asl_set(msg, ASL_KEY_MSG, [NSString stringWithFormat:@"line %d", i].UTF8String);
			asl_append(store, msg);
			asl_release(msg);
		}
		asl_release(store);

		CSKStreamViewController *controller = openStream(path, 100000);
		CHECK(controller.currentFilters != nil && controller.currentFilters.count == 0, "a new stream has no filters");
		CHECK(!controller.showsDetailsPane && controller.wantsNowMode && !controller.showsActivities, "display flags keep their values");
		CHECK([controller.messagesColumnLayoutAutosaveName isEqualToString:@"Console-ASLView-Messages"], "autosave name is kept");

		NSTableView *table = findTable(controller.view);
		CHECK(table != nil, "stream view contains a table");
		CHECK(table.numberOfColumns == 6, "ASL layout shows time, process, PID, subsystem, category and message (%ld)", (long)table.numberOfColumns);
		CHECK(table.numberOfRows == 4 && controller.loadError == nil, "every message is a row (%ld)", (long)table.numberOfRows);
		NSTableColumn *messageColumn = [table tableColumnWithIdentifier:@"composedMessage"];
		NSTableColumn *senderColumn = [table tableColumnWithIdentifier:@"sender"];
		CHECK([[table.dataSource tableView:table objectValueForTableColumn:messageColumn row:3] isEqual:@"line 3"]
			&& [[table.dataSource tableView:table objectValueForTableColumn:senderColumn row:0] isEqual:@"streamer"]
			&& [[table.dataSource tableView:table objectValueForTableColumn:[table tableColumnWithIdentifier:@"subsystem"] row:1] isEqual:@"org.darlinghq.stream"],
			"cells show the message fields");

		CSKStreamViewController *small = openStream(path, 2);
		NSTableView *smallTable = findTable(small.view);
		CHECK(smallTable.numberOfRows == 2
			&& [[smallTable.dataSource tableView:smallTable objectValueForTableColumn:[smallTable tableColumnWithIdentifier:@"composedMessage"] row:0] isEqual:@"line 2"],
			"capacity keeps the newest messages");

		CSKStreamViewController *missing = openStream([dir stringByAppendingPathComponent:@"missing.asl"], 10);
		CHECK(findTable(missing.view).numberOfRows == 0 && missing.loadError != nil, "an unreadable file shows no rows and keeps the error");

		[[NSFileManager defaultManager] removeItemAtPath:dir error:NULL];
		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
