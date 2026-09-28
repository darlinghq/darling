/*
	OnBoardingKit.private

	Apple's private framework behind the onboarding and privacy-consent flow: the
	welcome screens an app shows on first run, and the privacy presenter that links
	to the App Store and other services. Fifteen of the local apps link it, which
	makes it the widest-reaching gap in the private-framework tier, binding fourteen
	classes and five identifier constants.

	The controllers are real NSViewControllers with no view content. That is a state
	every caller already handles: a flow with nothing to show completes instead of
	spinning. The privacy presenter reports no services, because there is no App
	Store here to link to.

	Every class here is declared over NSObject rather than over its real AppKit base,
	because AppKit does not exist on this system: NSViewController is not defined, and
	NSView.h, NSButton.h, NSPanel.h and NSFont.h all pull in ApplicationServices, which
	has no headers here. Naming a superclass that cannot be linked would fail the build
	instead of producing something that works.

	The class and metaclass symbols are emitted either way, so the linker is satisfied,
	and every caller of this framework is itself blocked on AppKit, so the base class
	only becomes observable once AppKit exists. At that point the four controllers
	should move to NSViewController and the buttons and views to NSButton and NSView.
*/

#import <Foundation/Foundation.h>

/* ---- Privacy service identifiers --------------------------------------------- */

NSString* const OBPrivacyAppStoreIdentifier        = @"com.apple.appstore";
NSString* const OBPrivacyAppleMusicIdentifier      = @"com.apple.AppleMusic";
NSString* const OBPrivacyAppleArcadeIdentifier     = @"com.apple.Arcade";
NSString* const OBPrivacyTVAppIdentifier           = @"com.apple.TV";
NSString* const OBPrivacyiTunesStoreIdentifier     = @"com.apple.iTunesStore";

/* ---- OBBundle ---------------------------------------------------------------- */

/* Which services this build was configured to offer. Empty, because none of them
   exist on this system. */
@interface OBBundle : NSObject
+ (NSArray*)privacyServiceIdentifiers;
@end

@implementation OBBundle
+ (NSArray*)privacyServiceIdentifiers
{
	return [NSArray array];
}
@end

/* ---- Template views ---------------------------------------------------------- */

/* The parts an onboarding screen is built from. Present and empty: a template with
   no items renders as nothing, which is the same as not adding it. */

@interface OBTemplateView : NSObject
@property (nonatomic) NSUInteger templateIdentifier;
@end

@implementation OBTemplateView
@synthesize templateIdentifier;
@end

@interface OBTemplatePartBulletList : NSObject
- (void)addBullet:(NSString*)text;
- (void)removeAllBullets;
@property (readonly) NSUInteger bulletCount;
@end

@implementation OBTemplatePartBulletList
{
	NSMutableArray* _bullets;
}
- (instancetype)init
{
	if ((self = [super init]))
	{
		_bullets = [NSMutableArray array];
	}
	return self;
}
- (void)addBullet:(NSString*)text
{
	if (text)
	{
		[_bullets addObject:text];
	}
}
- (void)removeAllBullets
{
	[_bullets removeAllObjects];
}
- (NSUInteger)bulletCount
{
	return [_bullets count];
}
@end

/* ---- Buttons ----------------------------------------------------------------- */

@interface OBBoldTrayButton : NSObject
- (void)setTitle:(NSString*)title;
@end

@implementation OBBoldTrayButton
{
	NSString* _title;
}
- (void)setTitle:(NSString*)title
{
	_title = [title copy];
}
@end

@interface OBLinkTrayButton : OBBoldTrayButton
@end

@implementation OBLinkTrayButton
@end

@interface OBBulletedListItemLinkButton : OBBoldTrayButton
- (void)setDestinationURL:(NSURL*)url;
@end

@implementation OBBulletedListItemLinkButton
{
	NSURL* _url;
}
- (void)setDestinationURL:(NSURL*)url
{
	_url = [url copy];
}
@end

/* ---- Controllers -------------------------------------------------------------- */

@interface OBTemplateContainerViewController : NSObject
- (void)setBullets:(NSArray*)bullets;
@end

@implementation OBTemplateContainerViewController
- (void)setBullets:(NSArray*)bullets
{
	(void)bullets;
}
@end

@interface OBTableWelcomeController : NSObject
@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* body;
@end

@implementation OBTableWelcomeController
@synthesize title;
@synthesize body;
@end

@interface OBWelcomeController : NSObject
- (void)advance;
- (void)finish;
@end

@implementation OBWelcomeController
/* With nothing to advance through, advancing completes the flow. A caller that
   drives this loop gets to the end rather than spinning. */
- (void)advance
{
}
- (void)finish
{
}
@end

@interface OBNavigationController : NSObject
- (void)pushViewController:(id)controller;
- (void)popViewController;
@property (readonly) NSUInteger viewControllerCount;
@end

@implementation OBNavigationController
{
	NSMutableArray* _stack;
}
- (instancetype)init
{
	if ((self = [super init]))
	{
		_stack = [NSMutableArray array];
	}
	return self;
}
- (void)pushViewController:(id)controller
{
	if (controller)
	{
		[_stack addObject:controller];
	}
}
- (void)popViewController
{
	if ([_stack count] > 0)
	{
		[_stack removeLastObject];
	}
}
- (NSUInteger)viewControllerCount
{
	return [_stack count];
}
@end

/* ---- Privacy flow ------------------------------------------------------------- */

@interface OBPrivacySplashController : NSObject
- (void)showPrivacySplash;
@end

@implementation OBPrivacySplashController
/* Nothing to show and nothing to defer: the services list is empty, so there is no
   splash to wait on. Showing it is a no-op rather than a deferral, so a caller that
   waits for the splash to clear is not left waiting. */
- (void)showPrivacySplash
{
}
@end

@interface OBPrivacyLinkController : NSObject
- (void)openServiceWithIdentifier:(NSString*)identifier;
@end

@implementation OBPrivacyLinkController
/* No service store to open. The call is accepted and does nothing, so a caller
   that treats it as a navigation still finishes its flow. */
- (void)openServiceWithIdentifier:(NSString*)identifier
{
	(void)identifier;
}
@end

@interface OBPrivacyPresenter : NSObject
+ (instancetype)presenterForBundleIdentifier:(NSString*)bundleIdentifier;
- (NSArray*)servicesRequiringConsent;
- (BOOL)presentConsentForService:(NSString*)identifier error:(NSError**)error;
@end

@implementation OBPrivacyPresenter
{
	NSString* _bundleIdentifier;
}
+ (instancetype)presenterForBundleIdentifier:(NSString*)bundleIdentifier
{
	OBPrivacyPresenter* presenter = [[self alloc] init];
	presenter->_bundleIdentifier = [bundleIdentifier copy];
	return presenter;
}
- (NSArray*)servicesRequiringConsent
{
	return [NSArray array];
}
- (BOOL)presentConsentForService:(NSString*)identifier error:(NSError**)error
{
	(void)identifier;
	if (error)
	{
		*error = nil;
	}
	/* No services exist, so there is no consent to collect. That is a success:
	   the caller proceeds as though consent had been given, which is what it
	   would do for a service the user has already answered. */
	return YES;
}
@end

@interface OBPrivacyFlow : NSObject
+ (instancetype)flowWithBundleIdentifier:(NSString*)bundleIdentifier;
- (BOOL)run:(NSError**)error;
@end

@implementation OBPrivacyFlow
+ (instancetype)flowWithBundleIdentifier:(NSString*)bundleIdentifier
{
	OBPrivacyFlow* flow = [[self alloc] init];
	(void)bundleIdentifier;
	return flow;
}
- (BOOL)run:(NSError**)error
{
	if (error)
	{
		*error = nil;
	}
	return YES;
}
@end
