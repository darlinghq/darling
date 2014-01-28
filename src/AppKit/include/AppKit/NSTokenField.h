#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextField.h>
#import <AppKit/NSTokenFieldCell.h>
#import <Foundation/Foundation.h>

@protocol NSTokenFieldDelegate;


@interface NSTokenField : NSTextField {
    BOOL _reserved1;
    BOOL _reserved2;
    BOOL _reserved3;
    BOOL _reserved4;

    NSTrackingRectTag _trackingRectTag;
    id _reserved5;
    id _reserved6;
    id _reserved7;
}

- (void)setDelegate:(id <NSTokenFieldDelegate>)anObject;
- (id <NSTokenFieldDelegate>)delegate;



- (void)setTokenStyle: (NSTokenStyle) style;
- (NSTokenStyle)tokenStyle;

- (void)setCompletionDelay: (NSTimeInterval) delay;
- (NSTimeInterval)completionDelay;
+ (NSTimeInterval)defaultCompletionDelay;

- (void)setTokenizingCharacterSet:(NSCharacterSet *)characterSet;
- (NSCharacterSet *)tokenizingCharacterSet;
+ (NSCharacterSet *)defaultTokenizingCharacterSet;

@end

@protocol NSTokenFieldDelegate <NSTextFieldDelegate>

@optional





- (NSArray *)tokenField:(NSTokenField *)tokenField completionsForSubstring:(NSString *)substring indexOfToken:(NSInteger)tokenIndex indexOfSelectedItem:(NSInteger *)selectedIndex;




- (NSArray *)tokenField:(NSTokenField *)tokenField shouldAddObjects:(NSArray *)tokens atIndex:(NSUInteger)index;



- (NSString *)tokenField:(NSTokenField *)tokenField displayStringForRepresentedObject:(id)representedObject;
- (NSString *)tokenField:(NSTokenField *)tokenField editingStringForRepresentedObject:(id)representedObject;
- (id)tokenField:(NSTokenField *)tokenField representedObjectForEditingString: (NSString *)editingString;



- (BOOL)tokenField:(NSTokenField *)tokenField writeRepresentedObjects:(NSArray *)objects toPasteboard:(NSPasteboard *)pboard;


- (NSArray *)tokenField:(NSTokenField *)tokenField readFromPasteboard:(NSPasteboard *)pboard;


- (NSMenu *)tokenField:(NSTokenField *)tokenField menuForRepresentedObject:(id)representedObject;
- (BOOL)tokenField:(NSTokenField *)tokenField hasMenuForRepresentedObject:(id)representedObject;


- (NSTokenStyle)tokenField:(NSTokenField *)tokenField styleForRepresentedObject:(id)representedObject;

@end
