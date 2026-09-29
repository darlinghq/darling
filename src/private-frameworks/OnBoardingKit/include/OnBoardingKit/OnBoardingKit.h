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

	Every class is declared over NSObject rather than over its real AppKit base,
	because AppKit does not exist on this system.
*/

#ifndef ONBOARDINGKIT_H
#define ONBOARDINGKIT_H

#import <Foundation/Foundation.h>

extern NSString* const OBPrivacyAppStoreIdentifier;
extern NSString* const OBPrivacyAppleMusicIdentifier;
extern NSString* const OBPrivacyAppleArcadeIdentifier;
extern NSString* const OBPrivacyTVAppIdentifier;
extern NSString* const OBPrivacyiTunesStoreIdentifier;

@interface OBBundle : NSObject
+ (NSArray*)privacyServiceIdentifiers;
@end

@interface OBTemplateView : NSObject
@property (nonatomic) NSUInteger templateIdentifier;
@end

@interface OBTemplatePartBulletList : NSObject
- (void)addBullet:(NSString*)text;
- (void)removeAllBullets;
@property (readonly) NSUInteger bulletCount;
@end

@interface OBBoldTrayButton : NSObject
- (void)setTitle:(NSString*)title;
@end

@interface OBLinkTrayButton : OBBoldTrayButton
@end

@interface OBBulletedListItemLinkButton : OBBoldTrayButton
- (void)setDestinationURL:(NSURL*)url;
@end

@interface OBTemplateContainerViewController : NSObject
- (void)setBullets:(NSArray*)bullets;
@end

@interface OBTableWelcomeController : NSObject
@property (nonatomic, copy) NSString* title;
@property (nonatomic, copy) NSString* body;
@end

@interface OBWelcomeController : NSObject
- (void)advance;
- (void)finish;
@end

@interface OBNavigationController : NSObject
- (void)pushViewController:(id)controller;
- (void)popViewController;
@property (readonly) NSUInteger viewControllerCount;
@end

@interface OBPrivacyLinkController : NSObject
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

@interface OBPrivacySplashController : NSObject
- (void)showPrivacySplash;
@end

#endif
