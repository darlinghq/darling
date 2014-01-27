#import <AppKit/NSView.h>

@class NSScrollView, NSRulerMarker, NSMutableArray;


enum {
    NSHorizontalRuler,
    NSVerticalRuler
};
typedef NSUInteger NSRulerOrientation;

@interface NSRulerView : NSView {

  @private
    NSScrollView *_scrollView;
    NSRulerOrientation _orientation;

    NSString *_units;
    CGFloat _originOffset;

    CGFloat _ruleThickness;
    CGFloat _thicknessForMarkers;
    CGFloat _thicknessForAccessoryView;

    NSView *_clientView;
    NSMutableArray *_markers;
    NSView *_accessoryView;


    NSDictionary *_cachedHashDict;
    CGFloat _cachedDocViewToRulerConversion;


    NSPoint _cachedContentBoundsOrigin;


    NSRulerMarker *_draggingMarker;

    id _reservedRulerView1;
}



+ (void)registerUnitWithName:(NSString *)unitName abbreviation:(NSString *)abbreviation unitToPointsConversionFactor:(CGFloat)conversionFactor stepUpCycle:(NSArray *)stepUpCycle stepDownCycle:(NSArray *)stepDownCycle;



- (id)initWithScrollView:(NSScrollView *)scrollView orientation:(NSRulerOrientation)orientation;




- (void)setScrollView:(NSScrollView *)scrollView;
- (NSScrollView *)scrollView;


- (void)setOrientation:(NSRulerOrientation)orientation;
- (NSRulerOrientation)orientation;





- (CGFloat)baselineLocation;


- (CGFloat)requiredThickness;


- (void)setRuleThickness:(CGFloat)thickness;
- (CGFloat)ruleThickness;


- (void)setReservedThicknessForMarkers:(CGFloat)thickness;
- (CGFloat)reservedThicknessForMarkers;


- (void)setReservedThicknessForAccessoryView:(CGFloat)thickness;
- (CGFloat)reservedThicknessForAccessoryView;




- (void)setMeasurementUnits:(NSString *)unitName;
- (NSString *)measurementUnits;


- (void)setOriginOffset:(CGFloat)offset;
- (CGFloat)originOffset;




- (void)setClientView:(NSView *)client;
- (NSView *)clientView;

- (void)setMarkers:(NSArray *)markers;
- (void)addMarker:(NSRulerMarker *)marker;
- (void)removeMarker:(NSRulerMarker *)marker;
- (NSArray *)markers;


- (BOOL)trackMarker:(NSRulerMarker *)marker withMouseEvent:(NSEvent *)event;


- (void)setAccessoryView:(NSView *)accessory;
- (NSView *)accessoryView;


- (void)moveRulerlineFromLocation:(CGFloat)oldLocation toLocation:(CGFloat)newLocation;




- (void)invalidateHashMarks;


- (void)drawHashMarksAndLabelsInRect:(NSRect)rect;


- (void)drawMarkersInRect:(NSRect)rect;




- (BOOL)isFlipped;


@end


@interface NSView (NSRulerMarkerClientViewDelegation)

- (BOOL)rulerView:(NSRulerView *)ruler shouldMoveMarker:(NSRulerMarker *)marker;


- (CGFloat)rulerView:(NSRulerView *)ruler willMoveMarker:(NSRulerMarker *)marker toLocation:(CGFloat)location;


- (void)rulerView:(NSRulerView *)ruler didMoveMarker:(NSRulerMarker *)marker;


- (BOOL)rulerView:(NSRulerView *)ruler shouldRemoveMarker:(NSRulerMarker *)marker;


- (void)rulerView:(NSRulerView *)ruler didRemoveMarker:(NSRulerMarker *)marker;


- (BOOL)rulerView:(NSRulerView *)ruler shouldAddMarker:(NSRulerMarker *)marker;


- (CGFloat)rulerView:(NSRulerView *)ruler willAddMarker:(NSRulerMarker *)marker atLocation:(CGFloat)location;


- (void)rulerView:(NSRulerView *)ruler didAddMarker:(NSRulerMarker *)marker;


- (void)rulerView:(NSRulerView *)ruler handleMouseDown:(NSEvent *)event;


- (void)rulerView:(NSRulerView *)ruler willSetClientView:(NSView *)newClient;



- (CGFloat)rulerView:(NSRulerView *)ruler locationForPoint:(NSPoint)aPoint;
- (NSPoint)rulerView:(NSRulerView *)ruler pointForLocation:(CGFloat)aPoint;
@end
