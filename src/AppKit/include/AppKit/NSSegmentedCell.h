#import <Foundation/NSGeometry.h>
#import <AppKit/NSActionCell.h>
#import <AppKit/NSSegmentedControl.h>


enum {
    NSSegmentSwitchTrackingSelectOne = 0,
    NSSegmentSwitchTrackingSelectAny = 1,
    NSSegmentSwitchTrackingMomentary = 2
};
typedef NSUInteger NSSegmentSwitchTracking;

@class NSMutableArray;

@interface NSSegmentedCell : NSActionCell {
  @private
    NSMutableArray* _segmentItems;
    NSInteger _selectedSegment;
    NSInteger _keySegment;
    NSRect _lastBounds;
    struct {
 unsigned int trackingMode:3;
 unsigned int trimmedLabels:1;
 unsigned int drawing:1;
 unsigned int reserved1:2;
 unsigned int recalcToolTips:1;
        unsigned int usesWindowsStyle:1;
        unsigned int dontShowSelectedAndPressedAppearance:1;
        unsigned int menuShouldBeUniquedAgainstMain:1;
 unsigned int style:8;
        unsigned int flatMinX:1;
        unsigned int flatMaxX:1;
 unsigned int reserved:11;
    } _seFlags;
    id _segmentTrackingInfo;
    id _menuUniquer;
    NSInteger _reserved3;
    NSInteger _reserved4;
}



- (void)setSegmentCount:(NSInteger)count;
- (NSInteger)segmentCount;



- (void)setSelectedSegment:(NSInteger)selectedSegment;
- (NSInteger)selectedSegment;

- (BOOL)selectSegmentWithTag:(NSInteger)tag;



- (void)makeNextSegmentKey;
- (void)makePreviousSegmentKey;

- (void)setTrackingMode:(NSSegmentSwitchTracking)trackingMode;
- (NSSegmentSwitchTracking)trackingMode;






- (void)setWidth:(CGFloat)width forSegment:(NSInteger)segment;
- (CGFloat)widthForSegment:(NSInteger)segment;

- (void)setImage:(NSImage *)image forSegment:(NSInteger)segment;
- (NSImage *)imageForSegment:(NSInteger)segment;


- (void)setImageScaling:(NSImageScaling)scaling forSegment:(NSInteger)segment;
- (NSImageScaling)imageScalingForSegment:(NSInteger)segment;


- (void)setLabel:(NSString *)label forSegment:(NSInteger)segment;
- (NSString *)labelForSegment:(NSInteger)segment;

- (void)setSelected:(BOOL)selected forSegment:(NSInteger)segment;
- (BOOL)isSelectedForSegment:(NSInteger)segment;

- (void)setEnabled:(BOOL)enabled forSegment:(NSInteger)segment;
- (BOOL)isEnabledForSegment:(NSInteger)segment;

- (void)setMenu:(NSMenu *)menu forSegment:(NSInteger)segment;
- (NSMenu *)menuForSegment:(NSInteger)segment;

- (void)setToolTip:(NSString *)toolTip forSegment:(NSInteger)segment;
- (NSString *)toolTipForSegment:(NSInteger)segment;

- (void)setTag:(NSInteger)tag forSegment:(NSInteger)segment;
- (NSInteger)tagForSegment:(NSInteger)segment;


- (void)setSegmentStyle:(NSSegmentStyle)segmentStyle;
- (NSSegmentStyle)segmentStyle;



- (void)drawSegment:(NSInteger)segment inFrame:(NSRect)frame withView:(NSView *)controlView;

@end


@interface NSSegmentedCell (NSSegmentBackgroundStyle)


- (NSBackgroundStyle)interiorBackgroundStyleForSegment:(NSInteger)segment;
@end
