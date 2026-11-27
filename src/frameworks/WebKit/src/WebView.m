#import <WebKit/WebView.h>
#import "WKStubForwarding.h"

NSString *WebElementImageKey = @"WebElementImage";
NSString *WebElementImageURLKey = @"WebElementImageURL";
NSString *WebElementLinkLabelKey = @"WebElementLinkLabel";
NSString *WebElementLinkTargetFrameKey = @"WebElementTargetFrame";
NSString *WebElementLinkTitleKey = @"WebElementLinkTitle";
NSString *WebElementLinkURLKey = @"WebElementLinkURL";

NSString *const WebViewDidChangeSelectionNotification = @"WebViewDidChangeSelectionNotification";

NSString *WebViewProgressStartedNotification = @"WebProgressStartedNotification";
NSString *WebViewProgressEstimateChangedNotification = @"WebProgressEstimateChangedNotification";
NSString *WebViewProgressFinishedNotification = @"WebProgressFinishedNotification";

NSString *WebElementDOMNodeKey = @"WebElementDOMNode";
NSString *WebElementFrameKey = @"WebElementFrame";
NSString *WebElementIsSelectedKey = @"WebElementIsSelected";

NSString *WebActionElementKey = @"WebActionElementKey";
NSString *WebActionModifierFlagsKey = @"WebActionModifierFlagsKey";
NSString *WebActionOriginalURLKey = @"WebActionOriginalURLKey";

// Class stub: Darling's WebKit has no legacy HTML view, so -isKindOfClass: checks against it fail.
@implementation WebHTMLView
@end

@implementation WebView

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-designated-initializers"
- (instancetype)initWithFrame:(NSRect)frameRect {
    return [self initWithFrame:frameRect frameName:nil groupName:nil];
}
#pragma clang diagnostic pop

- (instancetype)initWithFrame:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName {
    (void)frameName;
    (void)groupName;
    if ([NSView instancesRespondToSelector:@selector(initWithFrame:)]) {
        return [super initWithFrame:frameRect];
    }
    return [super init];
}

- (BOOL)isFlipped {
    return YES;
}

WK_FORWARD_UNIMPLEMENTED_SELECTORS

@end
