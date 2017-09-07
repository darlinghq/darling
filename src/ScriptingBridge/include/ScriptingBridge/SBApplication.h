#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#import <ScriptingBridge/SBObject.h>

NS_ASSUME_NONNULL_BEGIN

@protocol SBApplicationDelegate;

NS_CLASS_AVAILABLE(10_5, NA)
@interface SBApplication : SBObject <NSCoding>

- (nullable __kindof SBApplication *) initWithBundleIdentifier:(NSString *)ident;
- (nullable __kindof SBApplication *) initWithURL:(NSURL *)url;
- (nullable __kindof SBApplication *) initWithProcessIdentifier:(pid_t)pid;

+ (nullable __kindof SBApplication *) applicationWithBundleIdentifier:(NSString *) ident;
+ (nullable __kindof SBApplication *) applicationWithURL:(NSURL *) url;
+ (nullable __kindof SBApplication *) applicationWithProcessIdentifier:(pid_t) pid;

- (nullable Class) classForScriptingClass:(NSString *) className;

@property (readonly, getter=isRunning) BOOL running;

- (void) activate;

@property (nullable, strong) id <SBApplicationDelegate> delegate;

@property LSLaunchFlags launchFlags;

@property AESendMode sendMode;

@property long timeout;

@end

@protocol SBApplicationDelegate

- (nullable id) eventDidFail:(const AppleEvent *)event withError:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
