#import <AppKit/NSApplication.h>

@interface NSApplication(NSScripting)


- (NSArray *)orderedDocuments;


- (NSArray *)orderedWindows;

@end

@interface NSObject(NSApplicationScriptingDelegation)


- (BOOL)application:(NSApplication *)sender delegateHandlesKey:(NSString *)key;

@end
