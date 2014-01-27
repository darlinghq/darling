#import <AppKit/NSCell.h>

@class NSImage;

enum {
    NSImageAlignCenter = 0,
    NSImageAlignTop,
    NSImageAlignTopLeft,
    NSImageAlignTopRight,
    NSImageAlignLeft,
    NSImageAlignBottom,
    NSImageAlignBottomLeft,
    NSImageAlignBottomRight,
    NSImageAlignRight
};
typedef NSUInteger NSImageAlignment;

enum {
    NSImageFrameNone = 0,
    NSImageFramePhoto,
    NSImageFrameGrayBezel,
    NSImageFrameGroove,
    NSImageFrameButton
};
typedef NSUInteger NSImageFrameStyle;


@interface NSImageCell : NSCell <NSCopying, NSCoding>
{

    id _controlView;
    struct __ICFlags {
        unsigned int _unused:22;
        unsigned int _animates:1;
        unsigned int _align:4;
        unsigned int _scale:2;
        unsigned int _style:3;
    } _icFlags;
    struct _NSImageCellAnimationState *_animationState;
    NSImage *_scaledImage;
}

- (NSImageAlignment)imageAlignment;
- (void)setImageAlignment:(NSImageAlignment)newAlign;
- (NSImageScaling)imageScaling;
- (void)setImageScaling:(NSImageScaling)newScaling;
- (NSImageFrameStyle)imageFrameStyle;
- (void)setImageFrameStyle:(NSImageFrameStyle)newStyle;

@end
