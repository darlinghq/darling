#import <Foundation/NSObject.h>
#import <AppKit/NSAnimation.h>
#import <AppKit/NSViewController.h>
#import <AppKit/NSAnimation.h>

@class NSMutableDictionary, NSMutableArray, NSView;
@protocol NSPageControllerDelegate;

enum {
    NSPageControllerTransitionStyleStackHistory,
    NSPageControllerTransitionStyleStackBook,
    NSPageControllerTransitionStyleHorizontalStrip
} NS_ENUM_AVAILABLE(10_8, NA);
typedef NSInteger NSPageControllerTransitionStyle;


@interface NSPageController : NSViewController <NSAnimatablePropertyContainer, NSCoding> {
@private
    NSView *_contentView;
    NSViewController *_selectedViewController;
    NSString *_currentIdentifier;
    NSPageControllerTransitionStyle _transitionStyle;
    id<NSPageControllerDelegate> _delegate;
    NSInteger _selectedIndex;
    NSMutableArray *_arrangedObjects;
    NSMutableArray *_snapshots;
    struct __pcDelegateFlags {
        unsigned int delegateRespondsToIdentifierForRepresentedObject:1;
        unsigned int delegateRespondsToViewControllerForIdentifier:1;
        unsigned int delegateRespondsToFrameForRepresentedObject:1;
        unsigned int delegateRespondsToPrepareView:1;
        unsigned int delegateRespondsToDidTransition:1;
        unsigned int delegateRespondsToWillLiveTransition:1;
        unsigned int delegateRespondsToDidLiveTransition:1;
        unsigned int delegateRespondsToReserved1:1;
        unsigned int reserved:24;
    }_pcDelegateFlags;
    struct __pcFlags {
        unsigned int templateCacheIsInvalid:1;
        unsigned int private1:1;
        unsigned int private2:1;
        unsigned int inSwipeGesture:1;
        unsigned int reserved:28;
    }_pcFlags;
    BOOL *_cancelAnimation;
    NSMutableDictionary *_reusueQueue;
    NSMutableDictionary *_templateCache;
    NSView *_transitionView;

    id _animator;
    NSDictionary *_animationsDictionary;

    id _private;
}

@property(assign) IBOutlet id<NSPageControllerDelegate> delegate;



@property(retain, readonly) NSViewController *selectedViewController;



@property NSPageControllerTransitionStyle transitionStyle;



@property(copy) NSArray *arrangedObjects;


@property NSInteger selectedIndex;




- (void)navigateForwardToObject:(id)object;




- (void)completeTransition;


- (IBAction)navigateBack:(id)sender;
- (IBAction)navigateForward:(id)sender;
- (IBAction)takeSelectedIndexFrom:(id)sender;
@end

@protocol NSPageControllerDelegate <NSObject>
@required
@optional
- (NSString *)pageController:(NSPageController *)pageController identifierForObject:(id)object;



- (NSViewController *)pageController:(NSPageController *)pageController viewControllerForIdentifier:(NSString *)identifier;






- (NSRect)pageController:(NSPageController *)pageController frameForObject:(id)object;



- (void)pageController:(NSPageController *)pageController prepareViewController:(NSViewController *)viewController withObject:(id)object;





- (void)pageController:(NSPageController *)pageController didTransitionToObject:(id)object;


- (void)pageControllerWillStartLiveTransition:(NSPageController *)pageController;



- (void)pageControllerDidEndLiveTransition:(NSPageController *)pageController;


@end
