#import <CoreFoundation/CFDate.h>
#import <CoreFoundation/CFRunLoop.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSWindow.h>

@class NSArray;
@class NSLock;
@class NSView;
@class NSWindow;
@class NSNotification;
@protocol NSDrawerDelegate;

enum {
    NSDrawerClosedState = 0,
    NSDrawerOpeningState = 1,
    NSDrawerOpenState = 2,
    NSDrawerClosingState = 3
};
typedef NSUInteger NSDrawerState;

@interface NSDrawer : NSResponder
{

    NSDrawerState _drawerState;
    NSDrawerState _drawerNextState;
    NSRectEdge _drawerEdge;
    NSRectEdge _drawerNextEdge;
    NSRectEdge _drawerPreferredEdge;
    float _drawerPercent;
    float _drawerPercentSaved;
    CGFloat _drawerLeadingOffset;
    CGFloat _drawerTrailingOffset;
    NSLock *_drawerLock;
    NSWindow *_drawerWindow;
    NSWindow *_drawerParentWindow;
    NSWindow *_drawerNextParentWindow;
    NSString *_drawerSaveName;
    CFAbsoluteTime _drawerStartTime;
    CFTimeInterval _drawerTotalTime;
    CFRunLoopRef _drawerLoop;
    __strong CFRunLoopTimerRef _drawerTimer;
    id _drawerDelegate;
    unsigned int _drawerFlags;
    __strong CFRunLoopObserverRef _drawerObserver;
}

- (id)initWithContentSize:(NSSize)contentSize preferredEdge:(NSRectEdge)edge;

- (void)setParentWindow:(NSWindow *)parent;
- (NSWindow *)parentWindow;
- (void)setContentView:(NSView *)aView;
- (NSView *)contentView;
- (void)setPreferredEdge:(NSRectEdge)edge;
- (NSRectEdge)preferredEdge;
- (void)setDelegate:(id <NSDrawerDelegate>)anObject;
- (id <NSDrawerDelegate>)delegate;

- (void)open;
- (void)openOnEdge:(NSRectEdge)edge;
- (void)close;

- (void)open:(id)sender;
- (void)close:(id)sender;
- (void)toggle:(id)sender;

- (NSInteger)state;
- (NSRectEdge)edge;

- (void)setContentSize:(NSSize)size;
- (NSSize)contentSize;
- (void)setMinContentSize:(NSSize)size;
- (NSSize)minContentSize;
- (void)setMaxContentSize:(NSSize)size;
- (NSSize)maxContentSize;

- (void)setLeadingOffset:(CGFloat)offset;
- (CGFloat)leadingOffset;
- (void)setTrailingOffset:(CGFloat)offset;
- (CGFloat)trailingOffset;

@end

@interface NSWindow(Drawers)

- (NSArray *)drawers;

@end

@protocol NSDrawerDelegate <NSObject>
@optional
- (BOOL)drawerShouldOpen:(NSDrawer *)sender;
- (BOOL)drawerShouldClose:(NSDrawer *)sender;
- (NSSize)drawerWillResizeContents:(NSDrawer *)sender toSize:(NSSize)contentSize;


- (void)drawerWillOpen:(NSNotification *)notification;
- (void)drawerDidOpen:(NSNotification *)notification;
- (void)drawerWillClose:(NSNotification *)notification;
- (void)drawerDidClose:(NSNotification *)notification;

@end


EXTERN_C NSString *NSDrawerWillOpenNotification;
EXTERN_C NSString *NSDrawerDidOpenNotification;
EXTERN_C NSString *NSDrawerWillCloseNotification;
EXTERN_C NSString *NSDrawerDidCloseNotification;
