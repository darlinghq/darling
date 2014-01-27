#ifndef NSAPPLICATION_H
#define NSAPPLICATION_H
#ifdef DARLING_BUILD
#	include <QEventLoop>
#	include <QApplication>
#	include "QNSEventDispatcher.h"

class NSApplicationEventFilter;
#endif

#include "NSResponder.h"
#include "NSApplicationDelegate.h"
#include "AppKitDefines.h"

@class NSEvent;

@interface NSApplication : NSResponder
#ifdef DARLING_BUILD
{
	bool m_running;
	QApplication* m_application;
	id<NSApplicationDelegate> m_delegate;
}
#endif

+(NSApplication*) sharedApplication;
-(id) init;
-(void) dealloc;

-(BOOL) isRunning;
-(void) stop: (id) sender;
-(void) run;

- (void) setDelegate:(id<NSApplicationDelegate>)anObject;
- (id<NSApplicationDelegate>) delegate;

- (NSEvent *)nextEventMatchingMask:(NSUInteger)mask untilDate:(NSDate *)expiration inMode:(NSString *)mode dequeue:(BOOL)flag;

- (void)sendEvent:(NSEvent *)anEvent;

@end

BEGIN_EXTERN_C

extern id NSApp;

BOOL NSApplicationLoad(void);
int NSApplicationMain(int argc, const char *argv[]);

extern const double NSAppKitVersionNumber;

#define NSAppKitVersionNumber10_0 577
#define NSAppKitVersionNumber10_1 620
#define NSAppKitVersionNumber10_2 663
#define NSAppKitVersionNumber10_2_3 663.6
#define NSAppKitVersionNumber10_3 743
#define NSAppKitVersionNumber10_3_2 743.14
#define NSAppKitVersionNumber10_3_3 743.2
#define NSAppKitVersionNumber10_3_5 743.24
#define NSAppKitVersionNumber10_3_7 743.33
#define NSAppKitVersionNumber10_3_9 743.36
#define NSAppKitVersionNumber10_4 824
#define NSAppKitVersionNumber10_4_1 824.1
#define NSAppKitVersionNumber10_4_3 824.23
#define NSAppKitVersionNumber10_4_4 824.33
#define NSAppKitVersionNumber10_4_7 824.41
#define NSAppKitVersionNumber10_5 949
#define NSAppKitVersionNumber10_5_2 949.27
#define NSAppKitVersionNumber10_5_3 949.33
#define NSAppKitVersionNumber10_6 1038
#define NSAppKitVersionNumber10_7 1138
#define NSAppKitVersionNumber10_7_2 1138.23

extern NSString *NSApplicationDidBecomeActiveNotification;
extern NSString *NSApplicationDidHideNotification;
extern NSString *NSApplicationDidFinishLaunchingNotification;
extern NSString *NSApplicationDidResignActiveNotification;
extern NSString *NSApplicationDidUnhideNotification;
extern NSString *NSApplicationDidUpdateNotification;
extern NSString *NSApplicationWillBecomeActiveNotification;
extern NSString *NSApplicationWillHideNotification;
extern NSString *NSApplicationWillFinishLaunchingNotification;
extern NSString *NSApplicationWillResignActiveNotification;
extern NSString *NSApplicationWillUnhideNotification;
extern NSString *NSApplicationWillUpdateNotification;
extern NSString *NSApplicationWillTerminateNotification;
extern NSString *NSApplicationDidChangeScreenParametersNotification;



END_EXTERN_C

enum {
    NSUserInterfaceLayoutDirectionLeftToRight = 0,
    NSUserInterfaceLayoutDirectionRightToLeft = 1
};
typedef NSInteger NSUserInterfaceLayoutDirection;

@interface NSApplication (NSApplicationLayoutDirection)
- (NSUserInterfaceLayoutDirection)userInterfaceLayoutDirection;
@end

enum {
    NSApplicationPresentationDefault = 0,
    NSApplicationPresentationAutoHideDock = (1 << 0),
    NSApplicationPresentationHideDock = (1 << 1),

    NSApplicationPresentationAutoHideMenuBar = (1 << 2),
    NSApplicationPresentationHideMenuBar = (1 << 3),

    NSApplicationPresentationDisableAppleMenu = (1 << 4),
    NSApplicationPresentationDisableProcessSwitching = (1 << 5),
    NSApplicationPresentationDisableForceQuit = (1 << 6),
    NSApplicationPresentationDisableSessionTermination = (1 << 7),
    NSApplicationPresentationDisableHideApplication = (1 << 8),
    NSApplicationPresentationDisableMenuBarTransparency = (1 << 9)
};

enum {
    NSApplicationPresentationFullScreen = (1 << 10),
    NSApplicationPresentationAutoHideToolbar = (1 << 11)
};

typedef NSUInteger NSApplicationPresentationOptions;


#endif

