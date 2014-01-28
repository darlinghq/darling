#import <AppKit/NSCell.h>

@class NSImage;

@interface NSBrowserCell : NSCell

+ (NSImage *)branchImage;
+ (NSImage *)highlightedBranchImage;

- (NSColor *)highlightColorInView:(NSView *)controlView;

- (BOOL)isLeaf;
- (void)setLeaf:(BOOL)flag;
- (BOOL)isLoaded;
- (void)setLoaded:(BOOL)flag;
- (void)reset;
- (void)set;
- (void)setImage:(NSImage *)image;
- (NSImage *)image;
- (void)setAlternateImage:(NSImage *)newAltImage;
- (NSImage *)alternateImage;

@end
