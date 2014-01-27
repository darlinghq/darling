#import <Foundation/NSObject.h>


#import <AppKit/NSTextFieldCell.h>

@class NSImage, NSString, NSURL;





@interface NSPathComponentCell : NSTextFieldCell {
@private
    NSImage *_image;

    CGFloat _fullWidth;

    CGFloat _resizedWidth;

    CGFloat _currentWidth;
    NSURL *_url;
    struct {
        unsigned int shouldDrawArrow:1;
        unsigned int drawsAsNavigationBar:1;
        unsigned int isFirstItem:1;
        unsigned int isLastItem:1;
        unsigned int isDropTarget:1;
        unsigned int reserved:27;
    } _flags;
    id _aux;
}



- (NSImage *)image;
- (void)setImage:(NSImage *)value;



- (NSURL *)URL;
- (void)setURL:(NSURL *)url;

@end
