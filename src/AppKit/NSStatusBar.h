#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSColor;
@class NSFont;
@class NSStatusItem;
@class NSMutableArray;

#define NSVariableStatusItemLength (-1)
#define NSSquareStatusItemLength (-2)

@interface NSStatusBar : NSObject
{
 @private
    id _items;
    void *_fReserved1;
    void *_fReserved2;
    NSInteger _registeredForNote;
}

+ (NSStatusBar*)systemStatusBar;

- (NSStatusItem*)statusItemWithLength:(CGFloat)length;
- (void)removeStatusItem:(NSStatusItem*)item;

- (BOOL)isVertical;
- (CGFloat)thickness;

@end
