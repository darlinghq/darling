#import <AppKit/NSTextFieldCell.h>
#import <Foundation/Foundation.h>


@class NSMutableArray, NSMutableCharacterSet, NSMutableDictionary, NSTextContainer;
@protocol NSTokenFieldCellDelegate;

enum {
    NSDefaultTokenStyle,
    NSPlainTextTokenStyle,
    NSRoundedTokenStyle
};
typedef NSUInteger NSTokenStyle;


@interface NSTokenFieldCell : NSTextFieldCell {
@private
    NSCharacterSet *_tokenizingCharacterSet;
    id _delegate;
    NSTimeInterval _completionDelay;
    id _cache;
    id _defaultTerminator;
    id _trackingArea;
    id _lastCell;
    NSRect _lastCellFrame;
    BOOL *_autoCompleteCancel;
    id _reserved[6];
    struct {
        unsigned int _style:4;

        unsigned int _invalidCache:1;
        unsigned int _inDidChange:1;
        unsigned int _validationDisabled:1;
        unsigned int _reserved1:1;
        unsigned int _autoCompleteMode:2;
        unsigned int _inValidateEditing:1;
        unsigned int _performingDrop:1;

        unsigned int _reserved:20;
    } _tfcFlags;
}



- (void)setTokenStyle:(NSTokenStyle)style;
- (NSTokenStyle)tokenStyle;



- (void)setCompletionDelay:(NSTimeInterval)delay;
- (NSTimeInterval)completionDelay;
+ (NSTimeInterval)defaultCompletionDelay;

- (void)setTokenizingCharacterSet:(NSCharacterSet *)characterSet;
- (NSCharacterSet *)tokenizingCharacterSet;
+ (NSCharacterSet *)defaultTokenizingCharacterSet;

- (void)setDelegate:(id <NSTokenFieldCellDelegate>)anObject;
- (id <NSTokenFieldCellDelegate>)delegate;

@end

@protocol NSTokenFieldCellDelegate <NSObject>

@optional





- (NSArray *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell completionsForSubstring:(NSString *)substring indexOfToken:(NSInteger)tokenIndex indexOfSelectedItem:(NSInteger *)selectedIndex;




- (NSArray *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell shouldAddObjects:(NSArray *)tokens atIndex:(NSUInteger)index;



- (NSString *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell displayStringForRepresentedObject:(id)representedObject;
- (NSString *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell editingStringForRepresentedObject:(id)representedObject;
- (id)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell representedObjectForEditingString: (NSString *)editingString;



- (BOOL)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell writeRepresentedObjects:(NSArray *)objects toPasteboard:(NSPasteboard *)pboard;


- (NSArray *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell readFromPasteboard:(NSPasteboard *)pboard;


- (NSMenu *)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell menuForRepresentedObject:(id)representedObject;
- (BOOL)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell hasMenuForRepresentedObject:(id)representedObject;


- (NSTokenStyle)tokenFieldCell:(NSTokenFieldCell *)tokenFieldCell styleForRepresentedObject:(id)representedObject;

@end
