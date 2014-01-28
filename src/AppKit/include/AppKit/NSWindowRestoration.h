#import <AppKit/NSWindow.h>
#import <AppKit/NSDocument.h>
#import <AppKit/NSDocumentController.h>
#import <AppKit/NSApplication.h>





@protocol NSWindowRestoration <NSObject>
#if NS_BLOCKS_AVAILABLE







+ (void)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler;
#endif
@end


@interface NSDocumentController (NSWindowRestoration) <NSWindowRestoration>
@end

@interface NSApplication (NSWindowRestoration)

#if NS_BLOCKS_AVAILABLE






- (BOOL)restoreWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler;
#endif

@end



EXTERN_C NSString * const NSApplicationDidFinishRestoringWindowsNotification;


@interface NSWindow (NSUserInterfaceRestoration)



- (void)setRestorable:(BOOL)flag;
- (BOOL)isRestorable;



- (void)setRestorationClass:(Class <NSWindowRestoration>)restorationClass;
- (Class <NSWindowRestoration>)restorationClass;


- (void)disableSnapshotRestoration;
- (void)enableSnapshotRestoration;


@end


@interface NSResponder (NSRestorableState)



- (void)encodeRestorableStateWithCoder:(NSCoder *)coder;



- (void)restoreStateWithCoder:(NSCoder *)coder;




- (void)invalidateRestorableState;




+ (NSArray *)restorableStateKeyPaths;

@end


@interface NSApplication (NSRestorableStateExtension)
- (void)extendStateRestoration;
- (void)completeStateRestoration;

@end

@interface NSDocument (NSRestorableState)







- (void)restoreDocumentWindowWithIdentifier:(NSString *)identifier state:(NSCoder *)state completionHandler:(void (^)(NSWindow *, NSError *))completionHandler;


- (void)encodeRestorableStateWithCoder:(NSCoder *)coder;
- (void)restoreStateWithCoder:(NSCoder *)coder;
- (void)invalidateRestorableState;
+ (NSArray *)restorableStateKeyPaths;

@end
