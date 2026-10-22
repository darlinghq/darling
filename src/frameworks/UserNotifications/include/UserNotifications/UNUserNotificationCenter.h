/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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

#ifndef _USERNOTIFICATIONS_UNUSERNOTIFICATIONCENTER_H_
#define _USERNOTIFICATIONS_UNUSERNOTIFICATIONCENTER_H_

#import <Foundation/Foundation.h>

@protocol UNUserNotificationCenterDelegate;
@class UNNotificationSettings, UNNotificationRequest, UNNotificationCategory;

typedef NS_OPTIONS(NSUInteger, UNAuthorizationOptions) {
    UNAuthorizationOptionBadge = (1 << 0),
    UNAuthorizationOptionSound = (1 << 1),
    UNAuthorizationOptionAlert = (1 << 2),
    UNAuthorizationOptionCarPlay = (1 << 3),
    UNAuthorizationOptionCriticalAlert = (1 << 4),
    UNAuthorizationOptionProvidesAppNotificationSettings = (1 << 5),
    UNAuthorizationOptionProvisional = (1 << 6),
    UNAuthorizationOptionAnnouncement = (1 << 7)
};

@interface UNUserNotificationCenter : NSObject {
    id<UNUserNotificationCenterDelegate> _delegate;
}

+ (UNUserNotificationCenter *) currentNotificationCenter;

@property(assign) id<UNUserNotificationCenterDelegate> delegate;
@property(readonly) BOOL supportsContentExtensions;

- (void) requestAuthorizationWithOptions: (UNAuthorizationOptions) options
                       completionHandler: (void (^)(BOOL granted, NSError *error)) completionHandler;

- (void) getNotificationSettingsWithCompletionHandler: (void (^)(UNNotificationSettings *settings)) completionHandler;

- (void) addNotificationRequest: (UNNotificationRequest *) request
          withCompletionHandler: (void (^)(NSError *error)) completionHandler;

- (void) removeAllPendingNotificationRequests;
- (void) removeAllDeliveredNotifications;

- (void) setNotificationCategories: (NSSet<UNNotificationCategory *> *) categories;
- (void) getNotificationCategoriesWithCompletionHandler: (void (^)(NSSet<UNNotificationCategory *> *categories)) completionHandler;

@end

#endif
