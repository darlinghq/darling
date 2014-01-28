#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSRange.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/AppKitDefines.h>

@class NSArray, NSView, NSOperationQueue;
@protocol NSTextFinderClient, NSTextFinderBarContainer;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSTextFinderActionShowFindInterface = 1,
    NSTextFinderActionNextMatch = 2,
    NSTextFinderActionPreviousMatch = 3,
    NSTextFinderActionReplaceAll = 4,
    NSTextFinderActionReplace = 5,
    NSTextFinderActionReplaceAndFind = 6,
    NSTextFinderActionSetSearchString = 7,
    NSTextFinderActionReplaceAllInSelection = 8,
    NSTextFinderActionSelectAll = 9,
    NSTextFinderActionSelectAllInSelection = 10,
    NSTextFinderActionHideFindInterface = 11,
    NSTextFinderActionShowReplaceInterface = 12,
    NSTextFinderActionHideReplaceInterface = 13
};
#endif
typedef NSInteger NSTextFinderAction;




EXTERN_C NSString *const NSTextFinderCaseInsensitiveKey;
EXTERN_C NSString *const NSTextFinderMatchingTypeKey;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {
    NSTextFinderMatchingTypeContains = 0,
    NSTextFinderMatchingTypeStartsWith = 1,
    NSTextFinderMatchingTypeFullWord = 2,
    NSTextFinderMatchingTypeEndsWith = 3
};
#endif
typedef NSInteger NSTextFinderMatchingType;



@interface NSTextFinder : NSObject <NSCoding> {
@private
    id _client;
    id _content;
    id <NSTextFinderBarContainer> _container;
    NSUInteger _style;
    BOOL _incrementalEnabled;
    BOOL _shouldDim;

    id _private;
}

- (id)init;


@property (assign) IBOutlet id <NSTextFinderClient> client;


- (void)performAction:(NSTextFinderAction)op;
- (BOOL)validateAction:(NSTextFinderAction)op;


@property (assign) IBOutlet id <NSTextFinderBarContainer> findBarContainer;





- (void)cancelFindIndicator;


@property BOOL findIndicatorNeedsUpdate;





@property (getter=isIncrementalSearchingEnabled) BOOL incrementalSearchingEnabled;


@property BOOL incrementalSearchingShouldDimContentView;


@property (readonly) NSArray *incrementalMatchRanges;


+ (void)drawIncrementalMatchHighlightInRect:(NSRect)rect;


- (void)noteClientStringWillChange;

@end


@protocol NSTextFinderClient <NSObject>







@optional


@property (getter=isSelectable, readonly) BOOL selectable;
@property (readonly) BOOL allowsMultipleSelection;
@property (getter=isEditable, readonly) BOOL editable;





@property (readonly) NSString *string;
- (NSString *)stringAtIndex:(NSUInteger)characterIndex effectiveRange:(NSRangePointer)outRange endsWithSearchBoundary:(BOOL *)outFlag;
- (NSUInteger)stringLength;





@property (readonly) NSRange firstSelectedRange;


@property (copy) NSArray *selectedRanges;


- (void)scrollRangeToVisible:(NSRange)range;


- (BOOL)shouldReplaceCharactersInRanges:(NSArray *)ranges withStrings:(NSArray *)strings;
- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string;
- (void)didReplaceCharacters;



- (NSView *)contentViewAtIndex:(NSUInteger)index effectiveCharacterRange:(NSRangePointer)outRange;


- (NSArray *)rectsForCharacterRange:(NSRange)range;


@property (readonly) NSArray *visibleCharacterRanges;


- (void)drawCharactersInRange:(NSRange)range forContentView:(NSView *)view;

@end


@protocol NSTextFinderBarContainer <NSObject>

@required


@property (retain) NSView *findBarView;


@property (getter=isFindBarVisible) BOOL findBarVisible;


- (void)findBarViewDidChangeHeight;

@optional


- (NSView *)contentView;

@end
