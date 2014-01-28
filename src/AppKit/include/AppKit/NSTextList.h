#import <Foundation/NSObject.h>


enum {
    NSTextListPrependEnclosingMarker = (1 << 0)
};

@interface NSTextList : NSObject <NSCoding, NSCopying> {

    NSString *_markerFormat;
    NSUInteger _listFlags;
    NSInteger _startIndex;
    void *_listSecondary;
}

- (id)initWithMarkerFormat:(NSString *)format options:(NSUInteger)mask;
- (NSString *)markerFormat;
- (NSUInteger)listOptions;
- (NSString *)markerForItemNumber:(NSInteger)itemNum;
- (void)setStartingItemNumber:(NSInteger)itemNum;
- (NSInteger)startingItemNumber;

@end
