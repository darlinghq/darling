#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSAppleEventDescriptor.h>
#import <AppKit/AppKitDefines.h>

@class NSArray, NSError, NSImage, NSView, NSNotificationCenter, NSURL, NSScreen, NSRunningApplication;

typedef NSUInteger NSWorkspaceLaunchOptions;
enum {
     NSWorkspaceLaunchAndPrint = 0x00000002,
     NSWorkspaceLaunchInhibitingBackgroundOnly = 0x00000080,
     NSWorkspaceLaunchWithoutAddingToRecents = 0x00000100,
     NSWorkspaceLaunchWithoutActivation = 0x00000200,
     NSWorkspaceLaunchAsync = 0x00010000,
     NSWorkspaceLaunchAllowingClassicStartup = 0x00020000,
     NSWorkspaceLaunchPreferringClassic = 0x00040000,
     NSWorkspaceLaunchNewInstance = 0x00080000,
     NSWorkspaceLaunchAndHide = 0x00100000,
     NSWorkspaceLaunchAndHideOthers = 0x00200000,

     NSWorkspaceLaunchDefault = NSWorkspaceLaunchAsync |
NSWorkspaceLaunchAllowingClassicStartup
};

typedef NSUInteger NSWorkspaceIconCreationOptions;
enum {
    NSExcludeQuickDrawElementsIconCreationOption = 1 << 1,
    NSExclude10_4ElementsIconCreationOption = 1 << 2
};

@interface NSWorkspace : NSObject {

  @private
    NSNotificationCenter *notificationCenter;
#if ! __LP64__
    int deviceStatusCount;
    int applicationStatusCount;
#endif
    id _reservedWorkspace1;
}


+ (NSWorkspace *)sharedWorkspace;


- (NSNotificationCenter *)notificationCenter;



- (BOOL)openFile:(NSString *)fullPath;
- (BOOL)openFile:(NSString *)fullPath withApplication:(NSString *)appName;
- (BOOL)openFile:(NSString *)fullPath withApplication:(NSString *)appName andDeactivate:(BOOL)flag;


- (BOOL)openFile:(NSString *)fullPath fromImage:(NSImage *)anImage at:(NSPoint)point inView:(NSView *)aView;


- (BOOL)openURL:(NSURL *)url;


- (BOOL)launchApplication:(NSString *)appName;





- (NSRunningApplication *)launchApplicationAtURL:(NSURL *)url options:(NSWorkspaceLaunchOptions)options configuration:(NSDictionary *)configuration error:(NSError **)error;


- (BOOL)launchApplication:(NSString *)appName showIcon:(BOOL)showIcon autolaunch:(BOOL)autolaunch;


- (NSString *)fullPathForApplication:(NSString *)appName;


- (BOOL)selectFile:(NSString *)fullPath inFileViewerRootedAtPath:(NSString *)rootFullPath;


- (void)activateFileViewerSelectingURLs:(NSArray *)fileURLs;



- (BOOL)showSearchResultsForQueryString:(NSString *)queryString;


- (void)noteFileSystemChanged:(NSString *)path;


- (BOOL)getInfoForFile:(NSString *)fullPath application:(NSString **)appName type:(NSString **)type;


- (BOOL)isFilePackageAtPath:(NSString *)fullPath;


- (NSImage *)iconForFile:(NSString *)fullPath;


- (NSImage *)iconForFiles:(NSArray *)fullPaths;


- (NSImage *)iconForFileType:(NSString *)fileType;



- (BOOL)setIcon:(NSImage *)image forFile:(NSString *)fullPath options:(NSWorkspaceIconCreationOptions)options;


- (NSArray *)fileLabels;


- (NSArray *)fileLabelColors;

#if NS_BLOCKS_AVAILABLE






- (void)recycleURLs:(NSArray *)URLs completionHandler:(void (^)(NSDictionary *newURLs, NSError *error))handler;
- (void)duplicateURLs:(NSArray *)URLs completionHandler:(void (^)(NSDictionary *newURLs, NSError *error))handler;
#endif
- (BOOL)getFileSystemInfoForPath:(NSString *)fullPath isRemovable:(BOOL *)removableFlag isWritable:(BOOL *)writableFlag isUnmountable:(BOOL *)unmountableFlag description:(NSString **)description type:(NSString **)fileSystemType;




- (BOOL)performFileOperation:(NSString *)operation source:(NSString *)source destination:(NSString *)destination files:(NSArray *)files tag:(NSInteger *)tag;


- (BOOL)unmountAndEjectDeviceAtPath:(NSString *)path;


- (BOOL)unmountAndEjectDeviceAtURL:(NSURL *)url error:(NSError **)error;


- (NSInteger)extendPowerOffBy:(NSInteger)requested;


- (void)hideOtherApplications;


- (NSURL *)URLForApplicationWithBundleIdentifier:(NSString *)bundleIdentifier;


- (NSURL *)URLForApplicationToOpenURL:(NSURL *)url;


- (NSString *)absolutePathForAppBundleWithIdentifier:(NSString *)bundleIdentifier;




- (BOOL)launchAppWithBundleIdentifier:(NSString *)bundleIdentifier options:(NSWorkspaceLaunchOptions)options additionalEventParamDescriptor:(NSAppleEventDescriptor *)descriptor launchIdentifier:(NSNumber **)identifier;
- (BOOL)openURLs:(NSArray *)urls withAppBundleIdentifier:(NSString *)bundleIdentifier options:(NSWorkspaceLaunchOptions)options additionalEventParamDescriptor:(NSAppleEventDescriptor *)descriptor launchIdentifiers:(NSArray **)identifiers;




- (NSRunningApplication *)frontmostApplication;


- (NSRunningApplication *)menuBarOwningApplication;






- (NSString *)typeOfFile:(NSString *)absoluteFilePath error:(NSError **)outError;





- (NSString *)localizedDescriptionForType:(NSString *)typeName;





- (NSString *)preferredFilenameExtensionForType:(NSString *)typeName;





- (BOOL)filenameExtension:(NSString *)filenameExtension isValidForType:(NSString *)typeName;







- (BOOL)type:(NSString *)firstTypeName conformsToType:(NSString *)secondTypeName;


@end



@interface NSWorkspace (NSDesktopImages)





- (BOOL)setDesktopImageURL:(NSURL *)url forScreen:(NSScreen *)screen options:(NSDictionary *)options error:(NSError **)error;



- (NSURL *)desktopImageURLForScreen:(NSScreen *)screen;



- (NSDictionary *)desktopImageOptionsForScreen:(NSScreen *)screen;

@end





EXTERN_C NSString * const NSWorkspaceDesktopImageScalingKey;



EXTERN_C NSString * const NSWorkspaceDesktopImageAllowClippingKey;



EXTERN_C NSString * const NSWorkspaceDesktopImageFillColorKey;






EXTERN_C NSString * const NSWorkspaceApplicationKey;

EXTERN_C NSString * NSWorkspaceWillLaunchApplicationNotification;
EXTERN_C NSString * NSWorkspaceDidLaunchApplicationNotification;
EXTERN_C NSString * NSWorkspaceDidTerminateApplicationNotification;
EXTERN_C NSString * const NSWorkspaceDidHideApplicationNotification;
EXTERN_C NSString * const NSWorkspaceDidUnhideApplicationNotification;
EXTERN_C NSString * const NSWorkspaceDidActivateApplicationNotification;
EXTERN_C NSString * const NSWorkspaceDidDeactivateApplicationNotification;






EXTERN_C NSString * const NSWorkspaceVolumeLocalizedNameKey;
EXTERN_C NSString * const NSWorkspaceVolumeURLKey;



EXTERN_C NSString * const NSWorkspaceVolumeOldLocalizedNameKey;
EXTERN_C NSString * const NSWorkspaceVolumeOldURLKey;

EXTERN_C NSString * NSWorkspaceDidMountNotification;
EXTERN_C NSString * NSWorkspaceDidUnmountNotification;
EXTERN_C NSString * NSWorkspaceWillUnmountNotification;



EXTERN_C NSString * const NSWorkspaceDidRenameVolumeNotification;



EXTERN_C NSString * const NSWorkspaceWillPowerOffNotification;

EXTERN_C NSString * NSWorkspaceWillSleepNotification;
EXTERN_C NSString * NSWorkspaceDidWakeNotification;

EXTERN_C NSString * const NSWorkspaceScreensDidSleepNotification;
EXTERN_C NSString * const NSWorkspaceScreensDidWakeNotification;


EXTERN_C NSString * NSWorkspaceSessionDidBecomeActiveNotification;
EXTERN_C NSString * NSWorkspaceSessionDidResignActiveNotification;




EXTERN_C NSString * NSWorkspaceDidPerformFileOperationNotification;



EXTERN_C NSString * const NSWorkspaceDidChangeFileLabelsNotification;

EXTERN_C NSString * const NSWorkspaceActiveSpaceDidChangeNotification;




EXTERN_C NSString * const NSWorkspaceLaunchConfigurationAppleEvent;
EXTERN_C NSString * const NSWorkspaceLaunchConfigurationArguments;
EXTERN_C NSString * const NSWorkspaceLaunchConfigurationEnvironment;
EXTERN_C NSString * const NSWorkspaceLaunchConfigurationArchitecture;



EXTERN_C NSString * NSWorkspaceMoveOperation;
EXTERN_C NSString * NSWorkspaceCopyOperation;
EXTERN_C NSString * NSWorkspaceLinkOperation;
EXTERN_C NSString * NSWorkspaceCompressOperation;
EXTERN_C NSString * NSWorkspaceDecompressOperation;
EXTERN_C NSString * NSWorkspaceEncryptOperation;
EXTERN_C NSString * NSWorkspaceDecryptOperation;
EXTERN_C NSString * NSWorkspaceDestroyOperation;
EXTERN_C NSString * NSWorkspaceRecycleOperation;
EXTERN_C NSString * NSWorkspaceDuplicateOperation;




@interface NSWorkspace (NSDeprecated)
- (BOOL)openTempFile:(NSString *)fullPath;
- (void)findApplications;
- (void)noteUserDefaultsChanged;
- (void)slideImage:(NSImage *)image from:(NSPoint)fromPoint to:(NSPoint)toPoint;
- (void)checkForRemovableMedia;
- (void)noteFileSystemChanged;
- (BOOL)fileSystemChanged;
- (BOOL)userDefaultsChanged;
- (NSArray *)mountNewRemovableMedia;
- (NSDictionary *)activeApplication;


- (NSArray *)mountedLocalVolumePaths;


- (NSArray *)mountedRemovableMedia;




- (NSArray *)launchedApplications;

@end

EXTERN_C NSString *NSPlainFileType;
EXTERN_C NSString *NSDirectoryFileType;
EXTERN_C NSString *NSApplicationFileType;
EXTERN_C NSString *NSFilesystemFileType;
EXTERN_C NSString *NSShellCommandFileType;
