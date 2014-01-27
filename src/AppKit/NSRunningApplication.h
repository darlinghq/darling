#import <Foundation/NSObject.h>
#import <AppKit/NSWorkspace.h>


enum {

    NSApplicationActivateAllWindows = 1 << 0,




    NSApplicationActivateIgnoringOtherApps = 1 << 1
};
typedef NSUInteger NSApplicationActivationOptions;



enum {

    NSApplicationActivationPolicyRegular,


    NSApplicationActivationPolicyAccessory,


    NSApplicationActivationPolicyProhibited
};
typedef NSInteger NSApplicationActivationPolicy;


@class NSLock, NSDate, NSImage, NSURL;


@interface NSRunningApplication : NSObject {
    @private
    id _superReserved;
    __strong void *_asn;
    __strong void **_helpers;
    id _obsInfo;
    NSLock *_lock;
    NSString *_bundleID;
    NSString *_localizedName;
    NSURL *_bundleURL;
    NSURL *_executableURL;
    NSDate *_launchDate;
    NSImage *_icon;
    int _pid;
    unsigned int _lastSeed;
    unsigned int _activeSeed;
    unsigned int _staleSeed;
    unsigned long long _obsMask;
    struct {
 unsigned fetchedDynamic:1;
 unsigned deadToUs:1;
        unsigned terminated:1;
        unsigned finishedLaunching:1;
        unsigned hidden:1;
        unsigned active:1;
        unsigned ownsMenuBar:1;
 unsigned arch:3;
 unsigned activationPolicy:3;
        unsigned reserved1:19;
    } _aflags;
    id _appReserved;
}


@property (readonly, getter=isTerminated) BOOL terminated;


@property (readonly, getter=isFinishedLaunching) BOOL finishedLaunching;


@property (readonly, getter=isHidden) BOOL hidden;


@property (readonly, getter=isActive) BOOL active;


@property (readonly) BOOL ownsMenuBar;


@property (readonly) NSApplicationActivationPolicy activationPolicy;


@property (readonly) NSString *localizedName;


@property (readonly) NSString *bundleIdentifier;


@property (readonly) NSURL *bundleURL;


@property (readonly) NSURL *executableURL;


@property (readonly) pid_t processIdentifier;


@property (readonly) NSDate *launchDate;


@property (readonly) NSImage *icon;


@property (readonly) NSInteger executableArchitecture;


- (BOOL)hide;
- (BOOL)unhide;


- (BOOL)activateWithOptions:(NSApplicationActivationOptions)options;


- (BOOL)terminate;


- (BOOL)forceTerminate;


+ (NSArray *)runningApplicationsWithBundleIdentifier:(NSString *)bundleIdentifier;


+ (NSRunningApplication *)runningApplicationWithProcessIdentifier:(pid_t)pid;


+ (NSRunningApplication *)currentApplication;





+ (void) terminateAutomaticallyTerminableApplications;

@end

@interface NSWorkspace (NSWorkspaceRunningApplications)







- (NSArray *)runningApplications;

@end
