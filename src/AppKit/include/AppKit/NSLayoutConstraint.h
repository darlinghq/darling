#import <AppKit/NSWindow.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSView.h>
#import <AppKit/NSAnimation.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSArray, NSDictionary;


enum {
    NSLayoutRelationLessThanOrEqual = -1,
    NSLayoutRelationEqual = 0,
    NSLayoutRelationGreaterThanOrEqual = 1,
};
typedef NSInteger NSLayoutRelation;

enum {
    NSLayoutAttributeLeft = 1,
    NSLayoutAttributeRight,
    NSLayoutAttributeTop,
    NSLayoutAttributeBottom,
    NSLayoutAttributeLeading,
    NSLayoutAttributeTrailing,
    NSLayoutAttributeWidth,
    NSLayoutAttributeHeight,
    NSLayoutAttributeCenterX,
    NSLayoutAttributeCenterY,
    NSLayoutAttributeBaseline,

    NSLayoutAttributeNotAnAttribute = 0
};
typedef NSInteger NSLayoutAttribute;

enum {
    NSLayoutFormatAlignAllLeft = (1 << NSLayoutAttributeLeft),
    NSLayoutFormatAlignAllRight = (1 << NSLayoutAttributeRight),
    NSLayoutFormatAlignAllTop = (1 << NSLayoutAttributeTop),
    NSLayoutFormatAlignAllBottom = (1 << NSLayoutAttributeBottom),
    NSLayoutFormatAlignAllLeading = (1 << NSLayoutAttributeLeading),
    NSLayoutFormatAlignAllTrailing = (1 << NSLayoutAttributeTrailing),
    NSLayoutFormatAlignAllCenterX = (1 << NSLayoutAttributeCenterX),
    NSLayoutFormatAlignAllCenterY = (1 << NSLayoutAttributeCenterY),
    NSLayoutFormatAlignAllBaseline = (1 << NSLayoutAttributeBaseline),

    NSLayoutFormatAlignmentMask = 0xFFFF,



    NSLayoutFormatDirectionLeadingToTrailing = 0 << 16,
    NSLayoutFormatDirectionLeftToRight = 1 << 16,
    NSLayoutFormatDirectionRightToLeft = 2 << 16,

    NSLayoutFormatDirectionMask = 0x3 << 16,
};
typedef NSUInteger NSLayoutFormatOptions;

enum {
    NSLayoutConstraintOrientationHorizontal = 0,
    NSLayoutConstraintOrientationVertical = 1
};
typedef NSInteger NSLayoutConstraintOrientation;

typedef struct {
    CGFloat top;
    CGFloat left;
    CGFloat bottom;
    CGFloat right;
} NSEdgeInsets;

 NSEdgeInsets NSEdgeInsetsMake(CGFloat top, CGFloat left, CGFloat bottom, CGFloat right) {
    NSEdgeInsets e;
    e.top = top;
    e.left = left;
    e.bottom = bottom;
    e.right = right;
    return e;
}





enum {
    NSLayoutPriorityRequired = 1000,
    NSLayoutPriorityDefaultHigh = 750,
    NSLayoutPriorityDragThatCanResizeWindow = 510,
    NSLayoutPriorityWindowSizeStayPut = 500,
    NSLayoutPriorityDragThatCannotResizeWindow = 490,
    NSLayoutPriorityDefaultLow = 250,
    NSLayoutPriorityFittingSizeCompression = 50,
};
typedef float NSLayoutPriority;


@interface NSLayoutConstraint : NSObject <NSAnimatablePropertyContainer>
{
    @private
    id _container;
    id _firstItem;
    id _secondItem;
    CGFloat _constant;
    CGFloat _loweredConstant;
    id _markerAndPositiveExtraVar;
    id _negativeExtraVar;
    float _coefficient;
    NSLayoutPriority _priority;
    uint64_t _layoutConstraintFlags;
    id _flange;
}



+ (NSArray *)constraintsWithVisualFormat:(NSString *)format options:(NSLayoutFormatOptions)opts metrics:(NSDictionary *)metrics views:(NSDictionary *)views;




#define NSDictionaryOfVariableBindings(...) _NSDictionaryOfVariableBindings(@"" #__VA_ARGS__, __VA_ARGS__, nil)
EXTERN_C NSDictionary *_NSDictionaryOfVariableBindings(NSString *commaSeparatedKeysString, id firstValue, ...);





+ (id)constraintWithItem:(id)view1 attribute:(NSLayoutAttribute)attr1 relatedBy:(NSLayoutRelation)relation toItem:(id)view2 attribute:(NSLayoutAttribute)attr2 multiplier:(CGFloat)multiplier constant:(CGFloat)c;





@property NSLayoutPriority priority;




@property BOOL shouldBeArchived;




@property (readonly, assign) id firstItem;
@property (readonly) NSLayoutAttribute firstAttribute;
@property (readonly) NSLayoutRelation relation;
@property (readonly, assign) id secondItem;
@property (readonly) NSLayoutAttribute secondAttribute;
@property (readonly) CGFloat multiplier;



@property CGFloat constant;

@end






@interface NSView (NSConstraintBasedLayoutInstallingConstraints)

- (NSArray *)constraints;

- (void)addConstraint:(NSLayoutConstraint *)constraint;
- (void)addConstraints:(NSArray *)constraints;
- (void)removeConstraint:(NSLayoutConstraint *)constraint;
- (void)removeConstraints:(NSArray *)constraints;

@end


@interface NSWindow (NSConstraintBasedLayoutCoreMethods)
- (void)updateConstraintsIfNeeded;
- (void)layoutIfNeeded;
@end

@interface NSView (NSConstraintBasedLayoutCoreMethods)
- (void)updateConstraintsForSubtreeIfNeeded;
- (void)updateConstraints;
- (BOOL)needsUpdateConstraints;
- (void)setNeedsUpdateConstraints:(BOOL)flag;

- (void)layoutSubtreeIfNeeded;
- (void)layout;
- (BOOL)needsLayout;
- (void)setNeedsLayout:(BOOL)flag;
@end



@interface NSView (NSConstraintBasedCompatibility)



- (BOOL)translatesAutoresizingMaskIntoConstraints;
- (void)setTranslatesAutoresizingMaskIntoConstraints:(BOOL)flag;



+ (BOOL)requiresConstraintBasedLayout;

@end



@interface NSView (NSConstraintBasedLayoutLayering)
- (NSRect)alignmentRectForFrame:(NSRect)frame;
- (NSRect)frameForAlignmentRect:(NSRect)alignmentRect;



- (NSEdgeInsets)alignmentRectInsets;



- (CGFloat)baselineOffsetFromBottom;
EXTERN_C const CGFloat NSViewNoInstrinsicMetric;
- (NSSize)intrinsicContentSize;
- (void)invalidateIntrinsicContentSize;

- (NSLayoutPriority)contentHuggingPriorityForOrientation:(NSLayoutConstraintOrientation)orientation;
- (void)setContentHuggingPriority:(NSLayoutPriority)priority forOrientation:(NSLayoutConstraintOrientation)orientation;

- (NSLayoutPriority)contentCompressionResistancePriorityForOrientation:(NSLayoutConstraintOrientation)orientation;
- (void)setContentCompressionResistancePriority:(NSLayoutPriority)priority forOrientation:(NSLayoutConstraintOrientation)orientation;
@end

@interface NSControl (NSConstraintBasedLayoutLayering)


- (void)invalidateIntrinsicContentSizeForCell:(NSCell *)cell;
@end



@interface NSWindow (NSConstraintBasedLayoutAnchoring)


- (NSLayoutAttribute)anchorAttributeForOrientation:(NSLayoutConstraintOrientation)orientation;
- (void)setAnchorAttribute:(NSLayoutAttribute)attr forOrientation:(NSLayoutConstraintOrientation)orientation;
@end


@interface NSView (NSConstraintBasedLayoutFittingSize)


- (NSSize)fittingSize;
@end









@interface NSView (NSConstraintBasedLayoutDebugging)




- (NSArray *)constraintsAffectingLayoutForOrientation:(NSLayoutConstraintOrientation)orientation;






- (BOOL)hasAmbiguousLayout;
- (void)exerciseAmbiguityInLayout;
@end

@interface NSWindow (NSConstraintBasedLayoutDebugging)


- (void)visualizeConstraints:(NSArray *)constraints;
@end
