#import <Foundation/NSObject.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSBundle.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSApplication.h>

@class NSAttributedString, NSWindow;

@interface NSHelpManager : NSObject
{

    NSMapTable *_helpMapTable;
    NSMapTable *_keyMapTable;
    NSMapTable *_bundleMapTable;
    NSWindow *_helpWindow;
    NSWindow *_shadowWindow;
    NSWindow *_evtWindow;
    NSBundle *_helpBundle;
}

+ (NSHelpManager *)sharedHelpManager;
+ (void)setContextHelpModeActive:(BOOL)active;
+ (BOOL)isContextHelpModeActive;

- (void)setContextHelp:(NSAttributedString *)attrString forObject:(id)object;
- (void)removeContextHelpForObject:(id)object;
- (NSAttributedString *)contextHelpForObject:(id)object;

- (BOOL)showContextHelpForObject:(id)object locationHint:(NSPoint)pt;

- (void)openHelpAnchor:(NSString *)anchor inBook:(NSString *)book;
- (void)findString:(NSString *)query inBook:(NSString *)book;


- (BOOL)registerBooksInBundle:(NSBundle *)bundle;

@end





EXTERN_C NSString *NSContextHelpModeDidActivateNotification;
EXTERN_C NSString *NSContextHelpModeDidDeactivateNotification;





@interface NSBundle(NSBundleHelpExtension)
- (NSAttributedString *)contextHelpForKey:(NSString *)key;
@end





@interface NSApplication(NSApplicationHelpExtension)
- (void)activateContextHelpMode:(id)sender;

- (void)showHelp:(id)sender;
@end
