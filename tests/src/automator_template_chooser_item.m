// Copyright (C) 2026 Lubos Dolezel
//
// +[AMTemplateChooserItem templateChooserItems] offers a blank Workflow item
// first, with a title and a description.
// Link with -framework Automator. The program passes unchanged on macOS 27.0.1
// (arm64 and x86_64), where the list also holds further document types.
#import <Foundation/Foundation.h>
#include <stdio.h>

@interface AMTemplateChooserItem : NSObject
+ (NSArray *)templateChooserItems;
- (NSString *)imageTitle;
- (NSString *)templateDescription;
@end

int main(void)
{
	@autoreleasepool {
		NSArray *items = [AMTemplateChooserItem templateChooserItems];
		if ([items count] < 1) {
			printf("FAIL: no template chooser items\n");
			return 1;
		}
		AMTemplateChooserItem *first = [items objectAtIndex:0];
		BOOL ok = [[first imageTitle] isEqualToString:@"Workflow"] && [[first templateDescription] length] > 0;
		printf("%s\n", ok ? "PASS" : "FAIL");
		return ok ? 0 : 1;
	}
}
