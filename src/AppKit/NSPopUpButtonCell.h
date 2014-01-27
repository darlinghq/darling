#import <AppKit/NSMenuItemCell.h>
#import <AppKit/NSMenuItem.h>

@class NSMenu;

enum {
    NSPopUpNoArrow = 0,
    NSPopUpArrowAtCenter = 1,
    NSPopUpArrowAtBottom = 2
};
typedef NSUInteger NSPopUpArrowPosition;

@interface NSPopUpButtonCell : NSMenuItemCell {

    @private
    NSMenu *_menu;
    int _selectedIndex;
    struct __pbcFlags {
        unsigned int pullsDown:1;
        unsigned int preferredEdge:3;
        unsigned int menuIsAttached:1;
        unsigned int usesItemFromMenu:1;
        unsigned int altersStateOfSelectedItem:1;
        unsigned int decoding:1;
        unsigned int arrowPosition:2;
        unsigned int ignoreMenuLayout:1;
        unsigned int drawing:1;
        unsigned int menuShouldBeUniquedAgainstMain:1;
        unsigned int RESERVED:19;
    } _pbcFlags;
#if __LP64__
    id _popupReserved;
#endif
}

- (id)initTextCell:(NSString *)stringValue pullsDown:(BOOL)pullDown;


- (void)setMenu:(NSMenu *)menu;
- (NSMenu *)menu;



- (void)setPullsDown:(BOOL)flag;
- (BOOL)pullsDown;

- (void)setAutoenablesItems:(BOOL)flag;
- (BOOL)autoenablesItems;

- (void)setPreferredEdge:(NSRectEdge)edge;
- (NSRectEdge)preferredEdge;


- (void)setUsesItemFromMenu:(BOOL)flag;
- (BOOL)usesItemFromMenu;


- (void)setAltersStateOfSelectedItem:(BOOL)flag;
- (BOOL)altersStateOfSelectedItem;



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

- (void)attachPopUpWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (void)dismissPopUp;
- (void)performClickWithFrame:(NSRect)frame inView:(NSView *)controlView;


- (NSPopUpArrowPosition)arrowPosition;
- (void)setArrowPosition:(NSPopUpArrowPosition)position;


- (id)objectValue;
- (void)setObjectValue:(id)obj;

@end



EXTERN_C NSString *NSPopUpButtonCellWillPopUpNotification;
