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
*/

#import <OnBoardingKit/OnBoardingKit.h>

/* ---- Privacy service identifiers --------------------------------------------- */

NSString* const OBPrivacyAppStoreIdentifier        = @"com.apple.appstore";
NSString* const OBPrivacyAppleMusicIdentifier      = @"com.apple.AppleMusic";
NSString* const OBPrivacyAppleArcadeIdentifier     = @"com.apple.Arcade";
NSString* const OBPrivacyTVAppIdentifier           = @"com.apple.TV";
NSString* const OBPrivacyiTunesStoreIdentifier     = @"com.apple.iTunesStore";

/* ---- OBBundle ---------------------------------------------------------------- */

/* Which services this build was configured to offer. Empty, because none of them
   exist on this system. */
@implementation OBBundle
+ (NSArray*)privacyServiceIdentifiers
{
	return [NSArray array];
}
@end

/* ---- Template views ---------------------------------------------------------- */

/* The parts an onboarding screen is built from. Present and empty: a template with
   no items renders as nothing, which is the same as not adding it. */

@implementation OBTemplateView
@synthesize templateIdentifier;
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

@implementation OBBoldTrayButton
@end

@implementation OBLinkTrayButton
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

@implementation OBTemplateContainerViewController
- (void)setBullets:(NSArray*)bullets
{
	(void)bullets;
}
@end

@implementation OBTableWelcomeController
@synthesize body;
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

@implementation OBPrivacySplashController
/* Nothing to show and nothing to defer: the services list is empty, so there is no
   splash to wait on. Showing it is a no-op rather than a deferral, so a caller that
   waits for the splash to clear is not left waiting. */
- (void)showPrivacySplash
{
}
@end

@implementation OBPrivacyLinkController
/* No service store to open. The call is accepted and does nothing, so a caller
   that treats it as a navigation still finishes its flow. */
- (void)openServiceWithIdentifier:(NSString*)identifier
{
	(void)identifier;
}
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
