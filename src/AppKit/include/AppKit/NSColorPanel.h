#import <AppKit/NSPanel.h>
#import <AppKit/NSApplication.h>

@class NSColorList, NSMutableArray;

typedef NSInteger NSColorPanelMode;

enum {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

    NSNoModeColorPanel = -1,
#endif
    NSGrayModeColorPanel = 0,
    NSRGBModeColorPanel = 1,
    NSCMYKModeColorPanel = 2,
    NSHSBModeColorPanel = 3,
    NSCustomPaletteModeColorPanel = 4,
    NSColorListModeColorPanel = 5,
    NSWheelModeColorPanel = 6,
    NSCrayonModeColorPanel = 7
};

enum {
    NSColorPanelGrayModeMask = 0x00000001,
    NSColorPanelRGBModeMask = 0x00000002,
    NSColorPanelCMYKModeMask = 0x00000004,
    NSColorPanelHSBModeMask = 0x00000008,
    NSColorPanelCustomPaletteModeMask = 0x00000010,
    NSColorPanelColorListModeMask = 0x00000020,
    NSColorPanelWheelModeMask = 0x00000040,
    NSColorPanelCrayonModeMask = 0x00000080,
    NSColorPanelAllModesMask = 0x0000ffff
};



@interface NSColorPanel : NSPanel
{

    id _colorSwatch;
    id _reserved1;
    id _colorWell;
    NSMutableArray *_pickersWithLoadedViews;
    id _magnifyButton;
    id _middleView;
    id _opacitySlider;
    id _opacityText;
    id _opacityView;
    id _modalButtons;
    id _pickerView;
    id _customViewsList;
    id _customPickerList;
    id _currViewObject;
    id _boxAboveSwatch;
    id _target;
    id _accessoryView;
    SEL _action;
    NSSize _minColorPanelSize;
    NSSize _maxColorPanelSize;
    NSSize _reserved2;
    NSSize _reserved3;
    id _resizeDimple;
    BOOL _reserved5;
    BOOL _reserved6;
    BOOL _handlingOpacityMoveAction;
    BOOL _ignoreConstraints;
    BOOL _continuous;
    BOOL _allowColorSetting;
    BOOL _stillInitializing;
    id _opacityTextController;
}

+ (NSColorPanel *)sharedColorPanel;
+ (BOOL)sharedColorPanelExists;
+ (BOOL)dragColor:(NSColor *)color withEvent:(NSEvent *)theEvent fromView:(NSView *)sourceView;
+ (void)setPickerMask:(NSUInteger)mask;
+ (void)setPickerMode:(NSColorPanelMode)mode;

- (void)setAccessoryView:(NSView *)aView;
- (NSView *)accessoryView;
- (void)setContinuous:(BOOL)flag;
- (BOOL)isContinuous;
- (void)setShowsAlpha:(BOOL)flag;
- (BOOL)showsAlpha;
- (void)setMode:(NSColorPanelMode)mode;
- (NSColorPanelMode)mode;
- (void)setColor:(NSColor *)color;
- (NSColor *)color;
- (CGFloat)alpha;
- (void)setAction:(SEL)aSelector;
- (void)setTarget:(id)anObject;
- (void)attachColorList:(NSColorList *)colorList;
- (void)detachColorList:(NSColorList *)colorList;
@end

@interface NSApplication(NSColorPanel)
- (void)orderFrontColorPanel:(id)sender;
@end

@interface NSObject(NSColorPanelResponderMethod)
- (void)changeColor:(id)sender;
@end


EXTERN_C NSString *NSColorPanelColorDidChangeNotification;
