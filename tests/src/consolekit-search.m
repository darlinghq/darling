// ConsoleKit search field controller, driven with the calls Console makes when it sets up its
// search bar and applies a stream's filters. Build against Foundation, AppKit and ConsoleKit.
#import <AppKit/AppKit.h>
#include <stdio.h>

@interface CSKTokenField : NSTokenField
@end

@interface CSKTokenFieldCell : NSTokenFieldCell
@end

@interface CSKTokenFieldController : NSObject
@property (strong) NSTokenField *tokenField;
@property (weak) id delegate;
@property BOOL isBasicSearchEnabled;
@property (readonly, copy) NSArray *filters;
- (void)updateSearchWithFilters:(NSArray *)filters;
@end

static int failures;
#define CHECK(cond, ...) do { int ok_ = (cond); printf("%s: ", ok_ ? "PASS" : "FAIL"); printf(__VA_ARGS__); printf("\n"); failures += !ok_; } while (0)

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	@autoreleasepool {
		[NSApplication sharedApplication];
		CHECK([CSKTokenField isSubclassOfClass:[NSTokenField class]] && [CSKTokenFieldCell isSubclassOfClass:[NSTokenFieldCell class]],
			"nib token field classes subclass AppKit's");

		CSKTokenField *field = [[CSKTokenField alloc] initWithFrame:NSMakeRect(0, 0, 200, 22)];
		NSObject *owner = [NSObject new];
		CSKTokenFieldController *controller = [CSKTokenFieldController new];
		controller.tokenField = field;
		controller.isBasicSearchEnabled = NO;
		controller.delegate = owner;
		controller.tokenField = field; // Console sets the field again after the delegate
		CHECK(controller.tokenField == field && controller.delegate == owner && !controller.isBasicSearchEnabled, "setup values are kept");

		[controller updateSearchWithFilters:@[]];
		CHECK(controller.filters.count == 0 && [field.objectValue count] == 0, "no filters leave the field empty");
		[controller updateSearchWithFilters:@[@"process:kernel", @"error"]];
		CHECK([controller.filters isEqual:(@[@"process:kernel", @"error"])] && [field.objectValue isEqual:controller.filters], "filters become the field's tokens");

		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
