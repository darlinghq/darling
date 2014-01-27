#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSResponder.h>
#import <AppKit/AppKitDefines.h>

@class NSView, NSViewController, NSWindow, NSNotification, NSString;












enum {


    NSPopoverAppearanceMinimal = 0,



    NSPopoverAppearanceHUD = 1
};
typedef NSInteger NSPopoverAppearance;



enum {


    NSPopoverBehaviorApplicationDefined = 0,



    NSPopoverBehaviorTransient = 1,



    NSPopoverBehaviorSemitransient = 2
};
typedef NSInteger NSPopoverBehavior;

@protocol NSPopoverDelegate;


@interface NSPopover : NSResponder {
@private
    id _bindingAdaptor;
    id <NSPopoverDelegate> _delegate;
    id _visualRepresentation;
    NSView *_positioningView;
    NSViewController *_contentViewController;
    NSWindow *_positioningWindow;
    NSPopoverAppearance _appearance;
    NSPopoverBehavior _behavior;
    NSRectEdge _unused;
    NSRectEdge _preferredEdge;
    NSPoint _unused2;
    NSSize _contentSize;
    NSRect _positioningRect;
#if !__OBJC2__
    id _reserved[3];
#endif
    struct {
        unsigned int animates:1;
        unsigned int positioningRectIsBounds:1;
        unsigned int registeredAsTransient:1;
        unsigned int registeredAsSemitransient:1;
        unsigned int shown:1;
        unsigned int toolbarHidesAnchor:1;
        unsigned int closing:1;
        unsigned int reserved:25;
    } _flags;
}





















@property(assign) IBOutlet id <NSPopoverDelegate> delegate;



@property NSPopoverAppearance appearance;



@property NSPopoverBehavior behavior;



@property BOOL animates;



@property(retain) IBOutlet NSViewController *contentViewController;



@property NSSize contentSize;



@property(readonly, getter=isShown) BOOL shown;



@property NSRect positioningRect;






- (void)showRelativeToRect:(NSRect)positioningRect ofView:(NSView *)positioningView preferredEdge:(NSRectEdge)preferredEdge;



- (IBAction)performClose:(id)sender;



- (void)close;

@end






EXTERN_C NSString * const NSPopoverCloseReasonKey;



EXTERN_C NSString * const NSPopoverCloseReasonStandard;



EXTERN_C NSString * const NSPopoverCloseReasonDetachToWindow;



EXTERN_C NSString * const NSPopoverWillShowNotification;



EXTERN_C NSString * const NSPopoverDidShowNotification;



EXTERN_C NSString * const NSPopoverWillCloseNotification;



EXTERN_C NSString * const NSPopoverDidCloseNotification;




@protocol NSPopoverDelegate <NSObject>
@optional



- (BOOL)popoverShouldClose:(NSPopover *)popover;



- (NSWindow *)detachableWindowForPopover:(NSPopover *)popover;



- (void)popoverWillShow:(NSNotification *)notification;



- (void)popoverDidShow:(NSNotification *)notification;



- (void)popoverWillClose:(NSNotification *)notification;



- (void)popoverDidClose:(NSNotification *)notification;

@end
