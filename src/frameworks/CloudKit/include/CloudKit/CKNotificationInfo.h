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

NS_ASSUME_NONNULL_BEGIN

// What the push notification of a subscription looks like.
NS_SWIFT_NAME(CKSubscription.NotificationInfo)
@interface CKNotificationInfo : NSObject <NSSecureCoding, NSCopying> {
@private
    NSString *_alertBody;
    NSString *_alertLocalizationKey;
    NSString *_title;
    NSString *_titleLocalizationKey;
    NSString *_subtitle;
    NSString *_subtitleLocalizationKey;
    NSString *_alertActionLocalizationKey;
    NSString *_alertLaunchImage;
    NSString *_soundName;
    NSString *_category;
    NSString *_collapseIDKey;
    NSArray<CKRecordFieldKey> *_alertLocalizationArgs;
    NSArray<CKRecordFieldKey> *_titleLocalizationArgs;
    NSArray<CKRecordFieldKey> *_subtitleLocalizationArgs;
    NSArray<CKRecordFieldKey> *_desiredKeys;
    BOOL _shouldBadge;
    BOOL _shouldSendContentAvailable;
    BOOL _shouldSendMutableContent;
}

@property (nonatomic, copy, nullable) NSString *alertBody;
@property (nonatomic, copy, nullable) NSString *alertLocalizationKey;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *alertLocalizationArgs NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) NSString *title;
@property (nonatomic, copy, nullable) NSString *titleLocalizationKey;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *titleLocalizationArgs NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) NSString *subtitle;
@property (nonatomic, copy, nullable) NSString *subtitleLocalizationKey;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *subtitleLocalizationArgs NS_REFINED_FOR_SWIFT;
@property (nonatomic, copy, nullable) NSString *alertActionLocalizationKey;
@property (nonatomic, copy, nullable) NSString *alertLaunchImage;
@property (nonatomic, copy, nullable) NSString *soundName;
@property (nonatomic, copy, nullable) NSArray<CKRecordFieldKey> *desiredKeys NS_REFINED_FOR_SWIFT;
@property (nonatomic, assign) BOOL shouldBadge;
@property (nonatomic, assign) BOOL shouldSendContentAvailable;
@property (nonatomic, assign) BOOL shouldSendMutableContent;
@property (nonatomic, copy, nullable) NSString *category;
@property (nonatomic, copy, nullable) NSString *collapseIDKey;

@end

NS_ASSUME_NONNULL_END
