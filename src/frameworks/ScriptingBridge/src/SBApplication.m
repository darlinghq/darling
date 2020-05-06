#import <ScriptingBridge/SBApplication.h>

@interface SBApplication () {

}

@end

@implementation SBApplication

//@synthesize delegate, launchFlags, sendMode, timeout;

- (nullable __kindof SBApplication *) initWithBundleIdentifier:(NSString *)ident {
    return nil;
}

- (nullable __kindof SBApplication *) initWithURL:(NSURL *)url {
    return nil;
}

- (nullable __kindof SBApplication *) initWithProcessIdentifier:(pid_t)pid {
    return nil;
}

+ (nullable __kindof SBApplication *) applicationWithBundleIdentifier:(NSString *) ident {
    return nil;
}

+ (nullable __kindof SBApplication *) applicationWithURL:(NSURL *) url {
    return nil;
}
+ (nullable __kindof SBApplication *) applicationWithProcessIdentifier:(pid_t) pid {
    return nil;
}

- (nullable Class) classForScriptingClass:(NSString *) className {
    return nil;
}

- (void) activate {

}

- (nullable id) eventDidFail:(const AppleEvent *)event withError:(NSError *)error {
    return nil;
}

@end
