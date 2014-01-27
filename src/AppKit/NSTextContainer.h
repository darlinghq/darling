#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSLayoutManager.h>

@class NSTextView;


enum {
    NSLineSweepLeft = 0,
    NSLineSweepRight = 1,
    NSLineSweepDown = 2,
    NSLineSweepUp = 3
};
typedef NSUInteger NSLineSweepDirection;


enum {
    NSLineDoesntMove = 0,
    NSLineMovesLeft = 1,
    NSLineMovesRight = 2,
    NSLineMovesDown = 3,
    NSLineMovesUp = 4
};
typedef NSUInteger NSLineMovementDirection;


@interface NSTextContainer : NSObject <NSCoding, NSTextLayoutOrientationProvider> {



  @private
    NSLayoutManager *_layoutManager;
    NSTextView *_textView;
    NSSize _size;
    CGFloat _lineFragmentPadding;
    struct __tcFlags {
        unsigned short widthTracksTextView:1;
        unsigned short heightTracksTextView:1;
        unsigned short observingFrameChanges:1;
        unsigned short _reserved:13;
    } _tcFlags;
}



- (id)initWithContainerSize:(NSSize)size;



- (NSLayoutManager *)layoutManager;
- (void)setLayoutManager:(NSLayoutManager *)layoutManager;


- (void)replaceLayoutManager:(NSLayoutManager *)newLayoutManager;


- (NSTextView *)textView;
- (void)setTextView:(NSTextView *)textView;


- (void)setWidthTracksTextView:(BOOL)flag;
- (BOOL)widthTracksTextView;
- (void)setHeightTracksTextView:(BOOL)flag;
- (BOOL)heightTracksTextView;




- (void)setContainerSize:(NSSize)size;
- (NSSize)containerSize;


- (void)setLineFragmentPadding:(CGFloat)pad;
- (CGFloat)lineFragmentPadding;




- (NSRect)lineFragmentRectForProposedRect:(NSRect)proposedRect sweepDirection:(NSLineSweepDirection)sweepDirection movementDirection:(NSLineMovementDirection)movementDirection remainingRect:(NSRectPointer)remainingRect;


- (BOOL)isSimpleRectangularTextContainer;




- (BOOL)containsPoint:(NSPoint)point;


@end
