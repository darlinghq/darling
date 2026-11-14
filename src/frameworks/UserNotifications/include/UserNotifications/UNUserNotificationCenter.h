/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel
 Copyright (C) 2026 Darling developers

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
@class UNUserNotificationCenter, UNNotificationSettings, UNNotificationRequest, UNNotificationCategory, UNNotification, UNNotificationResponse;

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

typedef NS_OPTIONS(NSUInteger, UNNotificationPresentationOptions) {
    UNNotificationPresentationOptionNone = 0,
    UNNotificationPresentationOptionBadge = (1 << 0),
    UNNotificationPresentationOptionSound = (1 << 1),
    UNNotificationPresentationOptionAlert = (1 << 2),
    UNNotificationPresentationOptionList = (1 << 3),
    UNNotificationPresentationOptionBanner = (1 << 4)
};

@protocol UNUserNotificationCenterDelegate <NSObject>
@optional
- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler;

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
didReceiveNotificationResponse:(UNNotificationResponse *)response
         withCompletionHandler:(void (^)(void))completionHandler;

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
   openSettingsForNotification:(nullable UNNotification *)notification;
@end

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

- (void) getPendingNotificationRequestsWithCompletionHandler: (void (^)(NSArray<UNNotificationRequest *> *requests)) completionHandler;
- (void) removePendingNotificationRequestsWithIdentifiers: (NSArray<NSString *> *) identifiers;
- (void) removeAllPendingNotificationRequests;

- (void) getDeliveredNotificationsWithCompletionHandler: (void (^)(NSArray<UNNotification *> *notifications)) completionHandler;
- (void) removeDeliveredNotificationsWithIdentifiers: (NSArray<NSString *> *) identifiers;
- (void) removeAllDeliveredNotifications;

- (void) setNotificationCategories: (NSSet<UNNotificationCategory *> *) categories;
- (void) getNotificationCategoriesWithCompletionHandler: (void (^)(NSSet<UNNotificationCategory *> *categories)) completionHandler;

@end

#endif
