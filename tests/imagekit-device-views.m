#import <ImageKit/ImageKit.h>
#import <Foundation/NSKeyedArchiver.h>
#include <stdio.h>
#include <objc/runtime.h>

static BOOL delegateReleased;

@interface TestDelegate : NSObject
@end
@implementation TestDelegate
- (void)dealloc
{
    delegateReleased = YES;
    [super dealloc];
}
@end

static int checkView(Class cls)
{
    if (![cls isSubclassOfClass:[NSView class]]) {
        fprintf(stderr, "FAIL %s must inherit NSView\n", class_getName(cls));
        return 1;
    }
    id view = [[cls alloc] initWithFrame:NSMakeRect(1, 2, 30, 40)];
    delegateReleased = NO;
    TestDelegate *delegate = [[TestDelegate alloc] init];
    [view setDelegate:delegate];
    BOOL ok = [view delegate] == delegate && NSEqualRects([view frame], NSMakeRect(1, 2, 30, 40));
    [delegate release];
    ok = ok && delegateReleased;
    [view setDelegate:nil];
    ok = ok && [view delegate] == nil;
    NSData *data = [NSKeyedArchiver archivedDataWithRootObject:view];
    id decoded = [NSKeyedUnarchiver unarchiveObjectWithData:data];
    ok = ok && [decoded isKindOfClass:cls] && NSEqualRects([decoded frame], [view frame]);
    [view release];
    printf("%s %s frame, delegate ownership and archive\n", ok ? "PASS" : "FAIL", class_getName(cls));
    return !ok;
}

int main(void)
{
    @autoreleasepool {
        return checkView([IKCameraDeviceView class]) | checkView([IKScannerDeviceView class]) | checkView([IKDeviceBrowserView class]);
    }
}
