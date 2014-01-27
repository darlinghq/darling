#import <AppKit/NSControl.h>
#import <AppKit/NSImageCell.h>

@interface NSImageView : NSControl {

    struct __IVFlags {
        unsigned int _hasImageView:1;
        unsigned int _unused:25;
        unsigned int _rejectsMultiFileDrops:1;
        unsigned int _compatibleScalingAndAlignment:1;
        unsigned int _reserved:1;
        unsigned int _overridesDrawing:1;
        unsigned int _allowsCutCopyPaste:1;
        unsigned int _editable:1;
    } _ivFlags;
    id _target;
    SEL _action;
}

- (NSImage *)image;
- (void)setImage:(NSImage *)newImage;

- (NSImageAlignment)imageAlignment;
- (void)setImageAlignment:(NSImageAlignment)newAlign;
- (NSImageScaling)imageScaling;
- (void)setImageScaling:(NSImageScaling)newScaling;
- (NSImageFrameStyle)imageFrameStyle;
- (void)setImageFrameStyle:(NSImageFrameStyle)newStyle;
- (void)setEditable:(BOOL)yn;
- (BOOL)isEditable;

- (void)setAnimates:(BOOL)flag;
- (BOOL)animates;

- (BOOL)allowsCutCopyPaste;
- (void)setAllowsCutCopyPaste:(BOOL)allow;

@end
