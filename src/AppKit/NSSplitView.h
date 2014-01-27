#import <AppKit/NSView.h>
#import <AppKit/NSLayoutConstraint.h>

@class NSNotification;
@protocol NSSplitViewDelegate;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
enum {
    NSSplitViewDividerStyleThick = 1,
    NSSplitViewDividerStyleThin = 2,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAX_OS_X_VERSION_10_6
    NSSplitViewDividerStylePaneSplitter = 3,
#endif
};
#endif
typedef NSInteger NSSplitViewDividerStyle;


@interface NSSplitView : NSView {
    @private
    id _variables;
}



- (void)setVertical:(BOOL)flag;
- (BOOL)isVertical;




- (void)setDividerStyle:(NSSplitViewDividerStyle)dividerStyle;
- (NSSplitViewDividerStyle)dividerStyle;



- (void)setAutosaveName:(NSString *)autosaveName;
- (NSString *)autosaveName;




- (void)setDelegate:(id <NSSplitViewDelegate>)delegate;
- (id <NSSplitViewDelegate>)delegate;



- (void)drawDividerInRect:(NSRect)rect;




- (NSColor *)dividerColor;




- (CGFloat)dividerThickness;



- (void)adjustSubviews;



- (BOOL)isSubviewCollapsed:(NSView *)subview;







- (CGFloat)minPossiblePositionOfDividerAtIndex:(NSInteger)dividerIndex;
- (CGFloat)maxPossiblePositionOfDividerAtIndex:(NSInteger)dividerIndex;



- (void)setPosition:(CGFloat)position ofDividerAtIndex:(NSInteger)dividerIndex;





- (NSLayoutPriority)holdingPriorityForSubviewAtIndex:(NSInteger)subviewIndex;
- (void)setHoldingPriority:(NSLayoutPriority)priority forSubviewAtIndex:(NSInteger)subviewIndex;

@end

@protocol NSSplitViewDelegate <NSObject>
@optional






- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview;




- (BOOL)splitView:(NSSplitView *)splitView shouldCollapseSubview:(NSView *)subview forDoubleClickOnDividerAtIndex:(NSInteger)dividerIndex;






- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex;





- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex;



- (CGFloat)splitView:(NSSplitView *)splitView constrainSplitPosition:(CGFloat)proposedPosition ofSubviewAt:(NSInteger)dividerIndex;





- (void)splitView:(NSSplitView *)splitView resizeSubviewsWithOldSize:(NSSize)oldSize;




- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view;





- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex;



- (NSRect)splitView:(NSSplitView *)splitView effectiveRect:(NSRect)proposedEffectiveRect forDrawnRect:(NSRect)drawnRect ofDividerAtIndex:(NSInteger)dividerIndex;



- (NSRect)splitView:(NSSplitView *)splitView additionalEffectiveRectOfDividerAtIndex:(NSInteger)dividerIndex;




- (void)splitViewWillResizeSubviews:(NSNotification *)notification;
- (void)splitViewDidResizeSubviews:(NSNotification *)notification;

@end



EXTERN_C NSString *NSSplitViewWillResizeSubviewsNotification;



EXTERN_C NSString *NSSplitViewDidResizeSubviewsNotification;


@interface NSSplitView (NSDeprecated)



- (void)setIsPaneSplitter:(BOOL)flag;
- (BOOL)isPaneSplitter;

@end
