#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSMenuItem.h>

@class NSEvent, NSView, NSFont;
@class NSMenu;
@class NSMutableArray, NSArray;
@protocol NSMenuDelegate;

@interface NSMenu : NSObject <NSCopying, NSCoding>
{

    @private
    NSMenu *_supermenu;
    NSString *_title;
    id _itemArray;
    id _extra;
    struct __mFlags {
        unsigned int noAutoenable:1;
        unsigned int inMain:1;
        unsigned int internalPerformAction:1;
        unsigned int condenseSeparators:1;
        unsigned int disabled:1;
        unsigned int ownedByPopUp:1;
 unsigned int delegateNeedsUpdate:1;
 unsigned int delegateUpdateItem:1;
 unsigned int delegateHasKeyEquiv:1;
 unsigned int delegateHasAltKeyEquiv:1;
 unsigned int excludeMarkColumn:1;
 unsigned int isContextualMenu:1;
        unsigned int cmPluginMode:2;
        unsigned int invertedCMPluginTypes:2;
        unsigned int allowsDifferentSelection:1;
        unsigned int noTopPadding:1;
        unsigned int noBottomPadding:1;
        unsigned int RESERVED:13;
    } _mFlags;
    NSString *_uiid;
}



- (id)initWithTitle:(NSString *)aTitle;


- (void)setTitle:(NSString *)aString;
- (NSString *)title;






+ (void)popUpContextMenu:(NSMenu*)menu withEvent:(NSEvent*)event forView:(NSView*)view;
+ (void)popUpContextMenu:(NSMenu*)menu withEvent:(NSEvent*)event forView:(NSView*)view withFont:(NSFont*)font;





- (BOOL)popUpMenuPositioningItem:(NSMenuItem *)item atLocation:(NSPoint)location inView:(NSView *)view;


+ (void)setMenuBarVisible:(BOOL)visible;
+ (BOOL)menuBarVisible;


- (NSMenu *)supermenu;


 - (void)setSupermenu:(NSMenu *)supermenu;


- (void)insertItem:(NSMenuItem *)newItem atIndex:(NSInteger)index;


- (void)addItem:(NSMenuItem *)newItem;



- (NSMenuItem *)insertItemWithTitle:(NSString *)aString action:(SEL)aSelector keyEquivalent:(NSString *)charCode atIndex:(NSInteger)index;


- (NSMenuItem *)addItemWithTitle:(NSString *)aString action:(SEL)aSelector keyEquivalent:(NSString *)charCode;


- (void)removeItemAtIndex:(NSInteger)index;


- (void)removeItem:(NSMenuItem *)item;


- (void)setSubmenu:(NSMenu *)aMenu forItem:(NSMenuItem *)anItem;



- (void)removeAllItems;


- (NSArray *)itemArray;


- (NSInteger)numberOfItems;


 - (NSMenuItem *)itemAtIndex:(NSInteger)index;


- (NSInteger)indexOfItem:(NSMenuItem *)index;


- (NSInteger)indexOfItemWithTitle:(NSString *)aTitle;
- (NSInteger)indexOfItemWithTag:(NSInteger)aTag;
- (NSInteger)indexOfItemWithRepresentedObject:(id)object;
- (NSInteger)indexOfItemWithSubmenu:(NSMenu *)submenu;
- (NSInteger)indexOfItemWithTarget:(id)target andAction:(SEL)actionSelector;


- (NSMenuItem *)itemWithTitle:(NSString *)aTitle;
- (NSMenuItem *)itemWithTag:(NSInteger)tag;


- (void)setAutoenablesItems:(BOOL)flag;
- (BOOL)autoenablesItems;


- (void)update;



- (BOOL)performKeyEquivalent:(NSEvent *)theEvent;



- (void)itemChanged:(NSMenuItem *)item;



- (void)performActionForItemAtIndex:(NSInteger)index;


- (void)setDelegate:(id <NSMenuDelegate>)anObject;
- (id <NSMenuDelegate>)delegate;



- (CGFloat)menuBarHeight;


- (void)cancelTracking;


- (void)cancelTrackingWithoutAnimation;


- (NSMenuItem *)highlightedItem;



- (CGFloat)minimumWidth;
- (void)setMinimumWidth:(CGFloat)width;



- (NSSize)size;



- (NSFont *)font;
- (void)setFont:(NSFont *)font;



- (BOOL)allowsContextMenuPlugIns;
- (void)setAllowsContextMenuPlugIns:(BOOL)allows;


- (void)setShowsStateColumn:(BOOL)showsState;
- (BOOL)showsStateColumn;


+ (NSZone *)menuZone;


- (void)setMenuChangedMessagesEnabled:(BOOL)flag;
- (BOOL)menuChangedMessagesEnabled;
- (void)helpRequested:(NSEvent *)eventPtr;
- (BOOL)isTornOff;

@end

@interface NSMenu (NSSubmenuAction)
- (void)submenuAction:(id)sender;
@end

@interface NSObject (NSMenuValidation)
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
@end

@protocol NSMenuDelegate <NSObject>
@optional
- (void)menuNeedsUpdate:(NSMenu*)menu;

- (NSInteger)numberOfItemsInMenu:(NSMenu*)menu;
- (BOOL)menu:(NSMenu*)menu updateItem:(NSMenuItem*)item atIndex:(NSInteger)index shouldCancel:(BOOL)shouldCancel;

- (BOOL)menuHasKeyEquivalent:(NSMenu*)menu forEvent:(NSEvent*)event target:(id*)target action:(SEL*)action;



- (void)menuWillOpen:(NSMenu *)menu;
- (void)menuDidClose:(NSMenu *)menu;


- (void)menu:(NSMenu *)menu willHighlightItem:(NSMenuItem *)item;




- (NSRect)confinementRectForMenu:(NSMenu *)menu onScreen:(NSScreen *)screen;

@end



enum {
    NSMenuPropertyItemTitle = 1 << 0,
    NSMenuPropertyItemAttributedTitle = 1 << 1,
    NSMenuPropertyItemKeyEquivalent = 1 << 2,
    NSMenuPropertyItemImage = 1 << 3,
    NSMenuPropertyItemEnabled = 1 << 4,
    NSMenuPropertyItemAccessibilityDescription = 1 << 5
};
typedef NSUInteger NSMenuProperties;





@interface NSMenu (NSMenuPropertiesToUpdate)
- (NSMenuProperties)propertiesToUpdate;
@end

EXTERN_C NSString *NSMenuWillSendActionNotification;
EXTERN_C NSString *NSMenuDidSendActionNotification;

EXTERN_C NSString *NSMenuDidAddItemNotification;
EXTERN_C NSString *NSMenuDidRemoveItemNotification;
EXTERN_C NSString *NSMenuDidChangeItemNotification;


EXTERN_C NSString *NSMenuDidBeginTrackingNotification;
EXTERN_C NSString *NSMenuDidEndTrackingNotification;


@interface NSMenu (NSDeprecated)

- (void)setMenuRepresentation:(id)menuRep;
- (id)menuRepresentation;

- (void)setContextMenuRepresentation:(id)menuRep;
- (id)contextMenuRepresentation;

- (void)setTearOffMenuRepresentation:(id)menuRep;
- (id)tearOffMenuRepresentation;

+ (void)setMenuZone:(NSZone *)aZone;

- (NSMenu *)attachedMenu;
- (BOOL)isAttached;
- (void)sizeToFit;
- (NSPoint)locationForSubmenu:(NSMenu *)aSubmenu;

@end
