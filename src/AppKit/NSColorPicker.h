#import <AppKit/NSColorPicking.h>

@interface NSColorPicker : NSObject <NSColorPickingDefault>
{

    id _imageObject;
    NSColorPanel *_colorPanel;
    NSString *_buttonToolTip;
}
- (id)initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)owningColorPanel;
- (NSColorPanel *)colorPanel;
- (NSImage *)provideNewButtonImage;
- (void)insertNewButtonImage:(NSImage *)newButtonImage in:(NSButtonCell *)buttonCell;
- (void)viewSizeChanged:(id)sender;
- (void)attachColorList:(NSColorList *)colorList;
- (void)detachColorList:(NSColorList *)colorList;
- (void)setMode:(NSColorPanelMode)mode;

- (NSString *)buttonToolTip;

- (NSSize)minContentSize;
@end
