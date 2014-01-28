#import <AppKit/NSTextField.h>

EXTERN_C NSString *NSComboBoxWillPopUpNotification;
EXTERN_C NSString *NSComboBoxWillDismissNotification;
EXTERN_C NSString *NSComboBoxSelectionDidChangeNotification;
EXTERN_C NSString *NSComboBoxSelectionIsChangingNotification;

@protocol NSComboBoxDelegate, NSComboBoxDataSource;

@interface NSComboBox : NSTextField {

    id _dataSource;
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

- (id <NSComboBoxDelegate>)delegate;
- (void)setDelegate:(id <NSComboBoxDelegate>)anObject;


- (id <NSComboBoxDataSource>)dataSource;
- (void)setDataSource:(id <NSComboBoxDataSource>)aSource;


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

@protocol NSComboBoxDataSource <NSObject>
@optional

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox;
- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index;

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string;
- (NSString *)comboBox:(NSComboBox *)aComboBox completedString:(NSString *)string;
@end

@protocol NSComboBoxDelegate <NSTextFieldDelegate>
@optional


- (void)comboBoxWillPopUp:(NSNotification *)notification;
- (void)comboBoxWillDismiss:(NSNotification *)notification;
- (void)comboBoxSelectionDidChange:(NSNotification *)notification;
- (void)comboBoxSelectionIsChanging:(NSNotification *)notification;

@end
