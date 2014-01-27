#import <AppKit/NSTextFieldCell.h>

@class NSButtonCell, NSTableView, NSMutableArray;
@protocol NSComboBoxCellDataSource;

@interface NSComboBoxCell : NSTextFieldCell {

    id _delegate;
    id _dataSource;
    struct __cbcFlags {
        unsigned int usesDataSource:1;
        unsigned int completes:1;
 unsigned int buttonBordered:1;
        unsigned int popUpIsUp:1;
 unsigned int filteringEvents:1;
        unsigned int drawing:1;
        unsigned int synchronizingSelection:1;
        unsigned int reserved:9;
 unsigned int visibleItems:16;
    } _cbcFlags;
    NSButtonCell *_buttonCell;
    NSTableView *_tableView;
    NSScrollView *_scrollView;
    NSWindow *_popUp;
    NSMutableArray *_popUpList;
    __strong NSRect *_cellFrame;
    void *_reserved;
 }

- (BOOL)hasVerticalScroller;
- (void)setHasVerticalScroller:(BOOL)flag;
- (NSSize)intercellSpacing;
- (void)setIntercellSpacing:(NSSize)aSize;
- (CGFloat)itemHeight;
- (void)setItemHeight:(CGFloat)itemHeight;
- (NSInteger)numberOfVisibleItems;
- (void)setNumberOfVisibleItems:(NSInteger)visibleItems;

- (void)setButtonBordered:(BOOL)flag;
- (BOOL)isButtonBordered;

- (void)reloadData;
- (void)noteNumberOfItemsChanged;

- (void)setUsesDataSource:(BOOL)flag;
- (BOOL)usesDataSource;

- (void)scrollItemAtIndexToTop:(NSInteger)index;
- (void)scrollItemAtIndexToVisible:(NSInteger)index;

- (void)selectItemAtIndex:(NSInteger)index;
- (void)deselectItemAtIndex:(NSInteger)index;
- (NSInteger)indexOfSelectedItem;
- (NSInteger)numberOfItems;

- (BOOL)completes;
- (void)setCompletes:(BOOL)completes;
- (NSString *)completedString:(NSString *)string;


- (id <NSComboBoxCellDataSource>)dataSource;
- (void)setDataSource:(id <NSComboBoxCellDataSource>)aSource;


- (void)addItemWithObjectValue:(id)object;
- (void)addItemsWithObjectValues:(NSArray *)objects;
- (void)insertItemWithObjectValue:(id)object atIndex:(NSInteger)index;
- (void)removeItemWithObjectValue:(id)object;
- (void)removeItemAtIndex:(NSInteger)index;
- (void)removeAllItems;
- (void)selectItemWithObjectValue:(id)object;
- (id)itemObjectValueAtIndex:(NSInteger)index;
- (id)objectValueOfSelectedItem;
- (NSInteger)indexOfItemWithObjectValue:(id)object;
- (NSArray *)objectValues;

@end

@protocol NSComboBoxCellDataSource <NSObject>
@optional

- (NSInteger)numberOfItemsInComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (id)comboBoxCell:(NSComboBoxCell *)aComboBoxCell objectValueForItemAtIndex:(NSInteger)index;

- (NSUInteger)comboBoxCell:(NSComboBoxCell *)aComboBoxCell indexOfItemWithStringValue:(NSString *)string;
- (NSString *)comboBoxCell:(NSComboBoxCell *)aComboBoxCell completedString:(NSString *)uncompletedString;
@end
