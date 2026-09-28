/*
	OnBoardingKit.private

	Apple's private framework behind the onboarding and privacy-consent flow: the
	welcome screens an app shows on first run, and the privacy presenter that links
	to the App Store and other services. Fifteen of the local apps link it, the
	widest-reaching gap in the private-framework tier.

	The controllers are real NSViewControllers with no view content, which is a
	state every caller already handles: a flow with nothing to show completes. The
	privacy presenter reports no services, because there is no App Store here to
	link to.
*/

#ifndef ONBOARDINGKIT_H
#define ONBOARDINGKIT_H

#import <Cocoa/Cocoa.h>

extern NSString* const OBPrivacyAppStoreIdentifier;
extern NSString* const OBPrivacyAppleMusicIdentifier;
extern NSString* const OBPrivacyAppleArcadeIdentifier;
extern NSString* const OBPrivacyTVAppIdentifier;
extern NSString* const OBPrivacyiTunesStoreIdentifier;

@interface OBBundle : NSObject
+ (NSArray*)privacyServiceIdentifiers;
@end

@interface OBTemplateView : NSView
@property (nonatomic) NSUInteger templateIdentifier;
@end

@interface OBTemplatePartBulletList : NSView
- (void)addBullet:(NSString*)text;
- (void)removeAllBullets;
@property (readonly) NSUInteger bulletCount;
@end

@interface OBBoldTrayButton : NSButton
@end

@interface OBLinkTrayButton : NSButton
@end

@interface OBBulletedListItemLinkButton : NSButton
- (void)setDestinationURL:(NSURL*)url;
@end

@interface OBTemplateContainerViewController : NSViewController
- (void)setBullets:(NSArray*)bullets;
@end

@interface OBTableWelcomeController : NSViewController
@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* body;
@end

@interface OBWelcomeController : NSViewController
- (void)advance;
- (void)finish;
@end

@interface OBNavigationController : NSViewController
- (void)pushViewController:(NSViewController*)controller;
- (void)popViewController;
@property (readonly) NSUInteger viewControllerCount;
@end

@interface OBPrivacyLinkController : NSViewController
- (void)openServiceWithIdentifier:(NSString*)identifier;
@end

@interface OBPrivacyPresenter : NSObject
+ (instancetype)presenterForBundleIdentifier:(NSString*)bundleIdentifier;
- (NSArray*)servicesRequiringConsent;
- (BOOL)presentConsentForService:(NSString*)identifier error:(NSError**)error;
@end

@interface OBPrivacyFlow : NSObject
+ (instancetype)flowWithBundleIdentifier:(NSString*)bundleIdentifier;
- (BOOL)run:(NSError**)error;
@end

#endif
