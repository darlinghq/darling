#import <AppKit/NSApplication.h>

@class NSArray, NSMutableArray, NSPrintInfo, NSView, NSViewController, NSWindow, NSWindowController;

@interface NSPageLayout : NSObject {
    @private
    NSMutableArray *_accessoryControllers;
    NSPrintInfo *_originalPrintInfo;
    id _delegate;
    SEL _didEndSelector;
    void *_contextInfo;
    NSPrintInfo *_presentedPrintInfo;
    NSWindowController *_windowController;
#if __LP64__
    id _reserved[4];
#else
    unsigned char _compatibilityPadding[156];
#endif
}



+ (NSPageLayout *)pageLayout;




- (void)addAccessoryController:(NSViewController *)accessoryController;
- (void)removeAccessoryController:(NSViewController *)accessoryController;
- (NSArray *)accessoryControllers;






- (void)beginSheetWithPrintInfo:(NSPrintInfo *)printInfo modalForWindow:(NSWindow *)docWindow delegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;



- (NSInteger)runModalWithPrintInfo:(NSPrintInfo *)printInfo;
- (NSInteger)runModal;



- (NSPrintInfo *)printInfo;

@end

@interface NSPageLayout(NSDeprecated)



- (void)setAccessoryView:(NSView *)accessoryView;
- (NSView *)accessoryView;



- (void)readPrintInfo;
- (void)writePrintInfo;

@end

@interface NSApplication(NSPageLayoutPanel)



- (void)runPageLayout:(id)sender;

@end
