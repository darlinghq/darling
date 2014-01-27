#import <AppKit/NSTextField.h>


@interface NSSearchField : NSTextField {
  @private
    unsigned int _reserved1;
    unsigned int _reserved2;
    unsigned int _reserved3;
    unsigned int _reserved4;
}

- (void) setRecentSearches:(NSArray*)searches;
- (NSArray*) recentSearches;


- (void) setRecentsAutosaveName:(NSString*)string;
- (NSString*) recentsAutosaveName;


@end
