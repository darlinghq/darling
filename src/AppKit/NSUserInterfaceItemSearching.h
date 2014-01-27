#import <Foundation/NSString.h>
#import <AppKit/NSApplication.h>


@protocol NSUserInterfaceItemSearching <NSObject>
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6

@required





#if NS_BLOCKS_AVAILABLE
- (void)searchForItemsWithSearchString:(NSString *)searchString resultLimit:(NSInteger)resultLimit matchedItemHandler:(void (^)(NSArray *items))handleMatchedItems;
#endif



- (NSArray *)localizedTitlesForItem:(id)item;

@optional



- (void)performActionForItem:(id)item;



- (void)showAllHelpTopicsForSearchString:(NSString *)searchString;

#endif
@end


@interface NSApplication (NSUserInterfaceItemSearching)



- (void)registerUserInterfaceItemSearchHandler:(id<NSUserInterfaceItemSearching>)handler;



- (void)unregisterUserInterfaceItemSearchHandler:(id<NSUserInterfaceItemSearching>)handler;




- (BOOL)searchString:(NSString *)searchString inUserInterfaceItemString:(NSString *)stringToSearch searchRange:(NSRange)searchRange foundRange:(NSRange *)foundRange;

@end
