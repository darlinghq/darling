#import <Cocoa/Cocoa.h>

extern NSString *WebElementImageKey;
extern NSString *WebElementImageURLKey;
extern NSString *WebElementLinkLabelKey;
extern NSString *WebElementLinkTargetFrameKey;
extern NSString *WebElementLinkTitleKey;
extern NSString *WebElementLinkURLKey;
extern NSString *WebElementDOMNodeKey;
extern NSString *WebElementFrameKey;
extern NSString *WebElementIsSelectedKey;

extern NSString *WebActionElementKey;
extern NSString *WebActionModifierFlagsKey;
extern NSString *WebActionOriginalURLKey;

@interface WebView : NSView
- (instancetype)initWithFrame:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName;
@end

@interface WebHTMLView : NSObject
@end

extern NSString *const WebViewDidChangeSelectionNotification;

extern NSString *WebViewProgressStartedNotification;
extern NSString *WebViewProgressEstimateChangedNotification;
extern NSString *WebViewProgressFinishedNotification;
