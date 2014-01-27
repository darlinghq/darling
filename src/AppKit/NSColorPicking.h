#import <Foundation/NSObject.h>
#import <AppKit/NSColorPanel.h>

@class NSButtonCell, NSColor, NSColorPanel, NSColorList, NSImage, NSView;

@protocol NSColorPickingDefault
- (id)initWithPickerMask:(NSUInteger)mask colorPanel:(NSColorPanel *)owningColorPanel;



- (NSImage *)provideNewButtonImage;
- (void)insertNewButtonImage:(NSImage *)newButtonImage in:(NSButtonCell *)buttonCell;
- (void)viewSizeChanged:(id)sender;
- (void)alphaControlAddedOrRemoved:(id)sender;
- (void)attachColorList:(NSColorList *)colorList;
- (void)detachColorList:(NSColorList *)colorList;
- (void)setMode:(NSColorPanelMode)mode;




- (NSString *)buttonToolTip;



- (NSSize)minContentSize;


@end



@protocol NSColorPickingCustom <NSColorPickingDefault>



- (BOOL)supportsMode:(NSColorPanelMode)mode;



- (NSColorPanelMode)currentMode;



- (NSView *)provideNewView:(BOOL)initialRequest;



- (void)setColor:(NSColor *)newColor;
@end
