#ifndef NSAPPLICATIONDELEGATE_H_
#define NSAPPLICATIONDELEGATE_H_

@class NSApplication;
@class NSCoder;
@class NSError;
@class NSData;
@class NSDictionary;
@class NSString;
@class NSArray;
@class NSNotification;
@class NSMenu;

typedef NSUInteger NSApplicationPrintReply;
typedef NSUInteger NSApplicationTerminateReply;

enum
{
	NSPrintingCancelled  = 0,
	NSPrintingSuccess    = 1,
	NSPrintingFailure    = 3,
	NSPrintingReplyLater = 2
};

enum
{
	NSTerminateCancel = 0,
	NSTerminateNow    = 1,
	NSTerminateLater  = 2
};

@protocol NSApplicationDelegate

- (void)application:(NSApplication *)app didDecodeRestorableState:(NSCoder *)coder;
- (void)application:(NSApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error;
- (void)application:(NSApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo;
- (void)application:(NSApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken;
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames;
- (BOOL)application:(id)sender openFileWithoutUI:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename;
- (NSApplicationPrintReply)application:(NSApplication *)application printFiles:(NSArray *)fileNames withSettings:(NSDictionary *)printSettings showPrintPanels:(BOOL)showPrintPanels;
- (void)application:(NSApplication *)app willEncodeRestorableState:(NSCoder *)coder;
- (NSError *)application:(NSApplication *)application willPresentError:(NSError *)error;
- (void)applicationDidBecomeActive:(NSNotification *)aNotification;
- (void)applicationDidChangeOcclusionState:(NSNotification *)notification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
- (void)applicationDidHide:(NSNotification *)aNotification;
- (void)applicationDidResignActive:(NSNotification *)aNotification;
- (void)applicationDidUnhide:(NSNotification *)aNotification;
- (void)applicationDidUpdate:(NSNotification *)aNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)sender;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag;
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
- (void)applicationWillBecomeActive:(NSNotification *)aNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)applicationWillHide:(NSNotification *)aNotification;
- (void)applicationWillResignActive:(NSNotification *)aNotification;
- (void)applicationWillTerminate:(NSNotification *)aNotification;
- (void)applicationWillUnhide:(NSNotification *)aNotification;
- (void)applicationWillUpdate:(NSNotification *)aNotification;

@end

#endif
