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

#import "CKInternal.h"
#import <CloudKit/CKNotification.h>

@implementation CKNotification

// The wire format of push payloads is not documented; no payload is recognised as a CloudKit notification.
+ (instancetype)notificationFromRemoteNotificationDictionary:(NSDictionary *)notificationDictionary
{
    return nil;
}

- (CKNotificationType)notificationType
{
    return 0;
}

- (CKNotificationID *)notificationID { return _notificationID; }
- (NSString *)containerIdentifier { return _containerIdentifier; }
- (BOOL)isPruned { return _isPruned; }
- (CKSubscriptionID)subscriptionID { return _subscriptionID; }
- (CKRecordID *)subscriptionOwnerUserRecordID { return _subscriptionOwnerUserRecordID; }
- (NSString *)alertBody { return _alertBody; }
- (NSString *)alertLocalizationKey { return _alertLocalizationKey; }
- (NSArray<NSString *> *)alertLocalizationArgs { return _alertLocalizationArgs; }
- (NSString *)title { return _title; }
- (NSString *)titleLocalizationKey { return _titleLocalizationKey; }
- (NSArray<NSString *> *)titleLocalizationArgs { return _titleLocalizationArgs; }
- (NSString *)subtitle { return _subtitle; }
- (NSString *)subtitleLocalizationKey { return _subtitleLocalizationKey; }
- (NSArray<NSString *> *)subtitleLocalizationArgs { return _subtitleLocalizationArgs; }
- (NSString *)alertActionLocalizationKey { return _alertActionLocalizationKey; }
- (NSString *)alertLaunchImage { return _alertLaunchImage; }
- (NSNumber *)badge { return _badge; }
- (NSString *)soundName { return _soundName; }
- (NSString *)category { return _category; }

@end
