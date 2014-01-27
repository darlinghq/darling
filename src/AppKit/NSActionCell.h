#import <AppKit/NSCell.h>

@interface NSActionCell : NSCell
{

    NSInteger _tag;
    id _target;
    SEL _action;
    id _controlView;
}

- (id)target;
- (void)setTarget:(id)anObject;
- (SEL)action;
- (void)setAction:(SEL)aSelector;
- (NSInteger)tag;
- (void)setTag:(NSInteger)anInt;

@end
