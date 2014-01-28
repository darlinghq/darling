#import <AppKit/NSText.h>
#import <AppKit/NSToolbar.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <Foundation/Foundation.h>

@class NSToolbarItemViewer, NSMenuItem, NSView, NSImage;

@interface NSToolbarItem : NSObject <NSCopying, NSValidatedUserInterfaceItem> {
@private
    NSToolbar * _toolbar;
    NSImage * _image;
    NSString * _itemIdentifier;

    NSString * _label;
    NSTextAlignment _labelAlignment;
    NSString * _paletteLabel;

    NSString * _toolTip;
    NSMenuItem * _menuItemRep;
    NSInteger _tag;

    struct __tbiFlags {
 unsigned int viewRespondsToIsEnabled:1;
 unsigned int viewRespondsToSetEnabled:1;
 unsigned int viewRespondsToTag:1;
 unsigned int viewRespondsToSetTag:1;
 unsigned int viewRespondsToAction:1;
 unsigned int viewRespondsToSetAction:1;
 unsigned int viewRespondsToTarget:1;
 unsigned int viewRespondsToSetTarget:1;
        unsigned int viewRespondsToImage:1;
        unsigned int viewRespondsToSetImage:1;
 unsigned int isEnabled:1;
 unsigned int isUserRemovable:1;
 unsigned int menuHasBeenSet:1;
        unsigned int menuRepIsDefault:1;
        unsigned int viewHasBeenLoaded:1;
        unsigned int drawingForDragImage:1;
        unsigned int isCustomItemType:1;
 unsigned int hasValidatedAutoModeConfiguration:1;
 unsigned int useAutoModeConfiguration:1;
        unsigned int RESERVED2:1;
        unsigned int autovalidationDisabled:1;
        unsigned int tagHasBeenSet:1;
 unsigned int sizeHasBeenSet:1;
        unsigned int stateWasDisabledBeforeSheet:1;
 unsigned int RESERVED:8;
    } _tbiFlags;
    id _tbiReserved;

    id _itemViewer;
    NSView * _view;
    NSSize _minSize;
    NSSize _maxSize;
#if __LP64__
    id _toolbarItemReserved;
#endif
}

- (id)initWithItemIdentifier:(NSString *)itemIdentifier;


- (NSString *)itemIdentifier;
- (NSToolbar *)toolbar;




- (void)setLabel:(NSString *)label;
- (NSString *)label;


- (void)setPaletteLabel:(NSString *)paletteLabel;
- (NSString *)paletteLabel;


- (void)setToolTip:(NSString*)toolTip;
- (NSString *)toolTip;


- (void)setMenuFormRepresentation:(NSMenuItem *)menuItem;
- (NSMenuItem *)menuFormRepresentation;


- (void)setTag:(NSInteger)tag;
- (NSInteger)tag;


- (void)setTarget:(id)target;
- (id)target;


- (void)setAction:(SEL)action;
- (SEL)action;


- (void)setEnabled:(BOOL)enabled;
- (BOOL)isEnabled;


- (void)setImage:(NSImage*)image;
- (NSImage *)image;


- (void)setView:(NSView *)view;
- (NSView *)view;


- (void)setMinSize:(NSSize)size;
- (NSSize)minSize;


- (void)setMaxSize:(NSSize)size;
- (NSSize)maxSize;



enum {
   NSToolbarItemVisibilityPriorityStandard = 0,
   NSToolbarItemVisibilityPriorityLow = -1000,
   NSToolbarItemVisibilityPriorityHigh = 1000,
   NSToolbarItemVisibilityPriorityUser = 2000
};

- (void)setVisibilityPriority:(NSInteger)visibilityPriority;
- (NSInteger)visibilityPriority;





- (void)validate;



- (void)setAutovalidates:(BOOL)resistance;
- (BOOL)autovalidates;






- (BOOL)allowsDuplicatesInToolbar;


@end


@interface NSObject (NSToolbarItemValidation)
- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem;


@end




EXTERN_C NSString *NSToolbarSeparatorItemIdentifier;
EXTERN_C NSString *NSToolbarSpaceItemIdentifier;
EXTERN_C NSString *NSToolbarFlexibleSpaceItemIdentifier;

EXTERN_C NSString *NSToolbarShowColorsItemIdentifier;
EXTERN_C NSString *NSToolbarShowFontsItemIdentifier;
EXTERN_C NSString *NSToolbarCustomizeToolbarItemIdentifier;
EXTERN_C NSString *NSToolbarPrintItemIdentifier;
