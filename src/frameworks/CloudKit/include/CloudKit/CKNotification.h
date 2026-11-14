/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <Foundation/Foundation.h>
#import <CloudKit/CKDefines.h>
#import <CloudKit/CKNotificationID.h>

NS_ASSUME_NONNULL_BEGIN

@class CKRecordID;

typedef NS_ENUM(NSInteger, CKNotificationType) {
    CKNotificationTypeQuery = 1,
    CKNotificationTypeRecordZone = 2,
    CKNotificationTypeReadNotification = 3,
    CKNotificationTypeDatabase = 4,
} NS_SWIFT_NAME(CKNotification.NotificationType);

// Base class of the notifications that describe a change on the server.
@interface CKNotification : NSObject {
@private
    CKNotificationID *_notificationID;
    NSString *_containerIdentifier;
    BOOL _isPruned;
    CKSubscriptionID _subscriptionID;
    CKRecordID *_subscriptionOwnerUserRecordID;
    NSString *_alertBody;
    NSString *_alertLocalizationKey;
    NSArray<NSString *> *_alertLocalizationArgs;
    NSString *_title;
    NSString *_titleLocalizationKey;
    NSArray<NSString *> *_titleLocalizationArgs;
    NSString *_subtitle;
    NSString *_subtitleLocalizationKey;
    NSArray<NSString *> *_subtitleLocalizationArgs;
    NSString *_alertActionLocalizationKey;
    NSString *_alertLaunchImage;
    NSNumber *_badge;
    NSString *_soundName;
    NSString *_category;
}

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Parses the payload of a remote notification; nil when it is not a CloudKit notification.
+ (nullable instancetype)notificationFromRemoteNotificationDictionary:(NSDictionary *)notificationDictionary;

@property (nonatomic, readonly, assign) CKNotificationType notificationType;
@property (nonatomic, readonly, copy, nullable) CKNotificationID *notificationID;
@property (nonatomic, readonly, copy, nullable) NSString *containerIdentifier;
@property (nonatomic, readonly, assign) BOOL isPruned;
@property (nonatomic, readonly, copy, nullable) CKSubscriptionID subscriptionID NS_REFINED_FOR_SWIFT;
@property (nonatomic, readonly, copy, nullable) CKRecordID *subscriptionOwnerUserRecordID;

@property (nonatomic, readonly, copy, nullable) NSString *alertBody;
@property (nonatomic, readonly, copy, nullable) NSString *alertLocalizationKey;
@property (nonatomic, readonly, copy, nullable) NSArray<NSString *> *alertLocalizationArgs;
@property (nonatomic, readonly, copy, nullable) NSString *title;
@property (nonatomic, readonly, copy, nullable) NSString *titleLocalizationKey;
@property (nonatomic, readonly, copy, nullable) NSArray<NSString *> *titleLocalizationArgs;
@property (nonatomic, readonly, copy, nullable) NSString *subtitle;
@property (nonatomic, readonly, copy, nullable) NSString *subtitleLocalizationKey;
@property (nonatomic, readonly, copy, nullable) NSArray<NSString *> *subtitleLocalizationArgs;
@property (nonatomic, readonly, copy, nullable) NSString *alertActionLocalizationKey;
@property (nonatomic, readonly, copy, nullable) NSString *alertLaunchImage;
@property (nonatomic, readonly, copy, nullable) NSNumber *badge;
@property (nonatomic, readonly, copy, nullable) NSString *soundName;
@property (nonatomic, readonly, copy, nullable) NSString *category;

@end

NS_ASSUME_NONNULL_END
