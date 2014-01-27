#import <AppKit/AppKitDefines.h>
#import <Foundation/NSObject.h>

@class NSArray, NSMutableArray, NSPrintInfo, NSSet, NSView, NSViewController, NSWindow, NSWindowController;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

enum {



    NSPrintPanelShowsCopies = 1 << 0,
    NSPrintPanelShowsPageRange = 1 << 1,
    NSPrintPanelShowsPaperSize = 1 << 2,
    NSPrintPanelShowsOrientation = 1 << 3,
    NSPrintPanelShowsScaling = 1 << 4,

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6



    NSPrintPanelShowsPrintSelection = 1 << 5,

#endif



    NSPrintPanelShowsPageSetupAccessory = 1 << 8,





    NSPrintPanelShowsPreview = 1 << 17

};
#endif
typedef NSInteger NSPrintPanelOptions;



EXTERN_C NSString *const NSPrintPhotoJobStyleHint;
EXTERN_C NSString *const NSPrintAllPresetsJobStyleHint;
EXTERN_C NSString *const NSPrintNoPresetsJobStyleHint;



EXTERN_C NSString *const NSPrintPanelAccessorySummaryItemNameKey;
EXTERN_C NSString *const NSPrintPanelAccessorySummaryItemDescriptionKey;


@protocol NSPrintPanelAccessorizing
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5



- (NSArray *)localizedSummaryItems;

@optional



- (NSSet *)keyPathsForValuesAffectingPreview;

#endif
@end


@interface NSPrintPanel : NSObject {
    @private
    NSMutableArray *_accessoryControllers;
    id _previewController;
    NSView *_thumbnailView;
    NSInteger _options;
    NSString *_defaultButtonTitle;
    NSString *_helpAnchor;
    NSString *_jobStyleHint;
    NSPrintInfo *_originalPrintInfo;
    id _delegate;
    SEL _didEndSelector;
    void *_contextInfo;
    NSPrintInfo *_presentedPrintInfo;
    NSWindowController *_windowController;
#if __LP64__
    id _reserved[2];
#else
    unsigned char _compatibilityPadding[192];
#endif
}



+ (NSPrintPanel *)printPanel;




- (void)addAccessoryController:(NSViewController<NSPrintPanelAccessorizing> *)accessoryController;
- (void)removeAccessoryController:(NSViewController<NSPrintPanelAccessorizing> *)accessoryController;
- (NSArray *)accessoryControllers;



- (void)setOptions:(NSPrintPanelOptions)options;
- (NSPrintPanelOptions)options;



- (void)setDefaultButtonTitle:(NSString *)defaultButtonTitle;
- (NSString *)defaultButtonTitle;



- (void)setHelpAnchor:(NSString *)helpAnchor;
- (NSString *)helpAnchor;




- (void)setJobStyleHint:(NSString *)hint;
- (NSString *)jobStyleHint;






- (void)beginSheetWithPrintInfo:(NSPrintInfo *)printInfo modalForWindow:(NSWindow *)docWindow delegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;



- (NSInteger)runModalWithPrintInfo:(NSPrintInfo *)printInfo;
- (NSInteger)runModal;




- (NSPrintInfo *)printInfo;


@end

@interface NSPrintPanel(NSDeprecated)



- (void)setAccessoryView:(NSView *)accessoryView;
- (NSView *)accessoryView;



- (void)updateFromPrintInfo;
- (void)finalWritePrintInfo;

@end
