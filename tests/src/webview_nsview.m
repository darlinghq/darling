#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

@interface TestSubWebView : WebView {
@public
    BOOL didCallInitWithFrameName;
}
@end

@implementation TestSubWebView
- (instancetype)initWithFrame:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName {
    self = [super initWithFrame:frameRect frameName:frameName groupName:groupName];
    if (self) {
        didCallInitWithFrameName = YES;
    }
    return self;
}
@end

int main(void) {
    @autoreleasepool {
        CHECK([WebView isSubclassOfClass:[NSView class]]);

        NSRect frame = NSMakeRect(10, 20, 320, 240);
        WebView *wv = [[WebView alloc] initWithFrame:frame frameName:@"main" groupName:@"default"];
        CHECK(wv != nil);
        CHECK([wv isKindOfClass:[NSView class]]);
        CHECK([wv isFlipped] == YES);
        CHECK(NSEqualRects([wv frame], frame));

        NSView *parent = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 400, 400)];
        [parent addSubview:wv];
        CHECK([[parent subviews] containsObject:wv]);
        [wv removeFromSuperview];
        [wv release];
        [parent release];

        WebView *wv2 = [[WebView alloc] initWithFrame:frame];
        CHECK(wv2 != nil);
        CHECK(NSEqualRects([wv2 frame], frame));
        [wv2 release];

        TestSubWebView *sub = [[TestSubWebView alloc] initWithFrame:frame];
        CHECK(sub != nil);
        CHECK(sub->didCallInitWithFrameName == YES);
        CHECK(NSEqualRects([sub frame], frame));
        [sub release];
    }
    puts("PASS");
    return 0;
}
