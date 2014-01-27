#import <AppKit/AppKitDefines.h>
#import <Foundation/Foundation.h>

@class NSArray, NSDictionary, NSMutableArray, NSNotification, NSString, NSToolbarItem, NSWindow, NSView;
@protocol NSToolbarDelegate;

enum { NSToolbarDisplayModeDefault, NSToolbarDisplayModeIconAndLabel, NSToolbarDisplayModeIconOnly, NSToolbarDisplayModeLabelOnly };
typedef NSUInteger NSToolbarDisplayMode;

enum { NSToolbarSizeModeDefault, NSToolbarSizeModeRegular, NSToolbarSizeModeSmall };
typedef NSUInteger NSToolbarSizeMode;

@interface NSToolbar : NSObject {
@private
    NSString * _toolbarIdentifier;

    NSMutableArray * _currentItems;
    NSMutableArray * _currentItemIdentifiers;

    id _res1;
    id _res2;

    NSString * _selectedItemIdentifier;
    __strong void * _metrics;

    id _delegate;
    NSWindow * _logicalWindow;
    id _configPalette;
    id _toolbarView;
    NSInteger _syncPostEnabledCount;

    struct __tbFlags {
 unsigned int allowsUserCustomization:1;
 unsigned int autosavesUsingIdentifier:1;
 unsigned int initialConfigurationDone:1;
 unsigned int doesNotAttachToMenuBar:1;
        unsigned int delegateDefaultItemIdentifiers:1;
        unsigned int delegateAllowedItemIdentifiers:1;
        unsigned int delegateItemWithItemIdentifier:1;
 unsigned int delegateNotificationsEnabled:1;
        unsigned int prefersToBeShown:1;
        unsigned int loadItemsImmediately:1;
        unsigned int currentItemsContainsPlaceholder:1;
        unsigned int customizationPanelIsRunning:1;
        unsigned int usesCustomSheetWidth:1;
        unsigned int clickAndDragPerformsCustomization:1;
        unsigned int showsNoContextMenu:1;
        unsigned int currentlyLoadingPlaceholders:1;
        unsigned int delegateItemWithItemIdentifier2:1;
        unsigned int inGlobalWindow:1;
        unsigned int reserved:2;
        unsigned int usingFSMetrics:1;
        unsigned int keyboardLoopNeedsUpdating:1;
        unsigned int showHideDuringConfigurationChangeDisabled:1;
 unsigned int displayMode:2;
 unsigned int sizeMode:2;
 unsigned int doNotShowBaselineSeparator:1;
        unsigned int hideWithoutResizingWindowHint:1;
        unsigned int autovalidatesItemsDisabled:1;
        unsigned int inAutovalidation:1;
 unsigned int loadedMetrics:1;
    } _tbFlags;

    NSInteger _customizationSheetWidth;
    id _tbReserved;
}


- (id)initWithIdentifier:(NSString *)identifier;


- (void)insertItemWithItemIdentifier:(NSString *)itemIdentifier atIndex:(NSInteger)index;
- (void)removeItemAtIndex:(NSInteger)index;


- (void)setDelegate:(id <NSToolbarDelegate>)delegate;
- (id <NSToolbarDelegate>)delegate;


- (void)setVisible:(BOOL)shown;
- (BOOL)isVisible;


- (void)runCustomizationPalette:(id)sender;
- (BOOL)customizationPaletteIsRunning;




- (void)setDisplayMode:(NSToolbarDisplayMode)displayMode;
- (NSToolbarDisplayMode)displayMode;


- (void)setSelectedItemIdentifier:(NSString *)itemIdentifier;
- (NSString *)selectedItemIdentifier;

- (void)setSizeMode:(NSToolbarSizeMode)sizeMode;
- (NSToolbarSizeMode)sizeMode;



- (void)setShowsBaselineSeparator:(BOOL)flag;
- (BOOL)showsBaselineSeparator;


- (void)setAllowsUserCustomization:(BOOL)allowCustomization;
- (BOOL)allowsUserCustomization;




- (void)setFullScreenAccessoryView:(NSView *)view;
- (NSView *)fullScreenAccessoryView;







- (void)setFullScreenAccessoryViewMinHeight:(CGFloat)minHeight;
- (void)setFullScreenAccessoryViewMaxHeight:(CGFloat)maxHeight;
- (CGFloat)fullScreenAccessoryViewMinHeight;
- (CGFloat)fullScreenAccessoryViewMaxHeight;






- (NSString *)identifier;


- (NSArray *)items;


- (NSArray *)visibleItems;





- (void)setAutosavesConfiguration:(BOOL)flag;
- (BOOL)autosavesConfiguration;


- (void)setConfigurationFromDictionary:(NSDictionary *)configDict;
- (NSDictionary *)configurationDictionary;






- (void)validateVisibleItems;

@end

@protocol NSToolbarDelegate <NSObject>


@optional


- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag;


- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar;


- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar;

@optional


- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;




- (void)toolbarWillAddItem: (NSNotification *)notification;


- (void)toolbarDidRemoveItem: (NSNotification *)notification;

@end


EXTERN_C NSString *NSToolbarWillAddItemNotification;
EXTERN_C NSString *NSToolbarDidRemoveItemNotification;
