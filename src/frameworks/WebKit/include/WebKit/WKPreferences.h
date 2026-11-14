/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

NS_ASSUME_NONNULL_BEGIN

@interface WKPreferences : NSObject

@property (nonatomic) CGFloat minimumFontSize;
@property (nonatomic) BOOL javaScriptCanOpenWindowsAutomatically;
@property (nonatomic) BOOL javaScriptEnabled;
@property (nonatomic) BOOL tabFocusesLinks;
@property (nonatomic, getter=isFraudulentWebsiteWarningEnabled) BOOL fraudulentWebsiteWarningEnabled;
@property (nonatomic, getter=isSiteSpecificQuirksModeEnabled) BOOL siteSpecificQuirksModeEnabled;
@property (nonatomic, getter=isTextInteractionEnabled) BOOL textInteractionEnabled;
@property (nonatomic, getter=isElementFullscreenEnabled) BOOL elementFullscreenEnabled;

@end

NS_ASSUME_NONNULL_END
