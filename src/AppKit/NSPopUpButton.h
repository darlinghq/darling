#import <AppKit/NSButton.h>
#import <AppKit/NSMenuItemCell.h>
#import <AppKit/NSMenuItem.h>

@class NSMenu;

@interface NSPopUpButton : NSButton {

    @private
    struct __pbFlags {
        unsigned int needsPullsDownFromTemplate:1;
        unsigned int RESERVED:31;
    } _pbFlags;
#if __LP64__
    id _popupReserved;
#endif
}

- (id)initWithFrame:(NSRect)buttonFrame pullsDown:(BOOL)flag;


- (void)setMenu:(NSMenu *)menu;
- (NSMenu *)menu;


- (void)setPullsDown:(BOOL)flag;
- (BOOL)pullsDown;

- (void)setAutoenablesItems:(BOOL)flag;
- (BOOL)autoenablesItems;

- (void)setPreferredEdge:(NSRectEdge)edge;
- (NSRectEdge)preferredEdge;



- (void)addItemWithTitle:(NSString *)title;
- (void)addItemsWithTitles:(NSArray *)itemTitles;
- (void)insertItemWithTitle:(NSString *)title atIndex:(NSInteger)index;

- (void)removeItemWithTitle:(NSString *)title;
- (void)removeItemAtIndex:(NSInteger)index;
- (void)removeAllItems;



- (NSArray *)itemArray;
- (NSInteger)numberOfItems;

- (NSInteger)indexOfItem:(NSMenuItem *)item;
- (NSInteger)indexOfItemWithTitle:(NSString *)title;
- (NSInteger)indexOfItemWithTag:(NSInteger)tag;
- (NSInteger)indexOfItemWithRepresentedObject:(id)obj;
- (NSInteger)indexOfItemWithTarget:(id)target andAction:(SEL)actionSelector;

- (NSMenuItem *)itemAtIndex:(NSInteger)index;
- (NSMenuItem *)itemWithTitle:(NSString *)title;
- (NSMenuItem *)lastItem;



- (void)selectItem:(NSMenuItem *)item;
- (void)selectItemAtIndex:(NSInteger)index;
- (void)selectItemWithTitle:(NSString *)title;
- (BOOL)selectItemWithTag:(NSInteger)tag;
- (void)setTitle:(NSString *)aString;

- (NSMenuItem *)selectedItem;
- (NSInteger)indexOfSelectedItem;
- (void)synchronizeTitleAndSelectedItem;


- (NSString *)itemTitleAtIndex:(NSInteger)index;
- (NSArray *)itemTitles;
- (NSString *)titleOfSelectedItem;

@end


EXTERN_C NSString *NSPopUpButtonWillPopUpNotification;
