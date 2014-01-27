#import <AppKit/NSTextFieldCell.h>




#define NSSearchFieldRecentsTitleMenuItemTag 1000


#define NSSearchFieldRecentsMenuItemTag 1001


#define NSSearchFieldClearRecentsMenuItemTag 1002


#define NSSearchFieldNoRecentsMenuItemTag 1003


@class NSButtonCell, NSMenu, NSMutableArray, NSTimer;

@interface NSSearchFieldCell : NSTextFieldCell {
  @private
    struct __sfFlags {
 unsigned int sendsWholeSearchString:1;
 unsigned int maximumRecents:8;
 unsigned int cancelVisible:1;
 unsigned int reserved2:2;
 unsigned int disableText:1;
 unsigned int menuTracking:1;
 unsigned int deferredUpdate:1;
 unsigned int sendsImmediately:1;

 unsigned int reserved:16;
    } _sfFlags;
    NSButtonCell* _searchButtonCell;
    NSButtonCell* _cancelButtonCell;
    NSMenu* _searchMenuTemplate;
    NSString* _recentsAutosaveName;
    NSMutableArray* _recentSearches;
    NSMenu* _searchMenu;
    NSTimer* _partialStringTimer;
    unsigned int _reserved1;
    unsigned int _reserved2;
    unsigned int _reserved3;
    unsigned int _reserved4;
}

- (NSButtonCell*) searchButtonCell;
- (void) setSearchButtonCell:(NSButtonCell*)cell;


- (NSButtonCell*) cancelButtonCell;
- (void) setCancelButtonCell:(NSButtonCell*)cell;


- (void) resetSearchButtonCell;
- (void) resetCancelButtonCell;


- (NSRect) searchTextRectForBounds:(NSRect)rect;
- (NSRect) searchButtonRectForBounds:(NSRect)rect;
- (NSRect) cancelButtonRectForBounds:(NSRect)rect;


- (void) setSearchMenuTemplate:(NSMenu*)menu;
- (NSMenu*)searchMenuTemplate;


- (void) setSendsWholeSearchString:(BOOL)flag;
- (BOOL) sendsWholeSearchString;


- (void) setMaximumRecents:(NSInteger)maxRecents;
- (NSInteger) maximumRecents;


- (void) setRecentSearches:(NSArray*)searches;
- (NSArray*) recentSearches;


- (void) setRecentsAutosaveName:(NSString*)string;
- (NSString*) recentsAutosaveName;


- (BOOL) sendsSearchStringImmediately;
- (void) setSendsSearchStringImmediately:(BOOL)flag;

@end
