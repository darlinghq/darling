#import <WebKit/WebView.h>

NSString *WebElementImageKey = @"WebElementImage";
NSString *WebElementLinkLabelKey = @"WebElementLinkLabel";
NSString *WebElementLinkTargetFrameKey = @"WebElementTargetFrame";
NSString *WebElementLinkTitleKey = @"WebElementLinkTitle";
NSString *WebElementLinkURLKey = @"WebElementLinkURL";

NSString *const WebViewDidChangeSelectionNotification = @"WebViewDidChangeSelectionNotification";

NSString *WebElementDOMNodeKey = @"WebElementDOMNode";
NSString *WebElementFrameKey = @"WebElementFrame";
NSString *WebElementIsSelectedKey = @"WebElementIsSelected";

NSString *WebActionElementKey = @"WebActionElement";
NSString *WebActionModifierFlagsKey = @"WebActionModifierFlags";
NSString *WebActionOriginalURLKey = @"WebActionOriginalURL";

// Class stub: Darling's WebKit has no legacy HTML view, so -isKindOfClass: checks against it fail.
@implementation WebHTMLView
@end
