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

#import <UserNotifications/UNUserNotificationCenter.h>
#import <UserNotifications/UNNotificationSettings.h>

@implementation UNUserNotificationCenter

@synthesize delegate = _delegate;

static UNUserNotificationCenter *s_currentCenter = nil;

+ (UNUserNotificationCenter *) currentNotificationCenter
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        s_currentCenter = [[UNUserNotificationCenter alloc] init];
    });
    return s_currentCenter;
}

- (BOOL) supportsContentExtensions
{
    return NO;
}

- (void) requestAuthorizationWithOptions: (UNAuthorizationOptions) options
                       completionHandler: (void (^)(BOOL granted, NSError *error)) completionHandler
{
    if (completionHandler) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler(YES, nil);
        });
    }
}

- (void) getNotificationSettingsWithCompletionHandler: (void (^)(UNNotificationSettings *settings)) completionHandler
{
    if (completionHandler) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler([[[UNNotificationSettings alloc] init] autorelease]);
        });
    }
}

- (void) addNotificationRequest: (UNNotificationRequest *) request
          withCompletionHandler: (void (^)(NSError *error)) completionHandler
{
    if (completionHandler) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler(nil);
        });
    }
}

- (void) removeAllPendingNotificationRequests
{
}

- (void) removeAllDeliveredNotifications
{
}

- (void) setNotificationCategories: (NSSet<UNNotificationCategory *> *) categories
{
}

- (void) getNotificationCategoriesWithCompletionHandler: (void (^)(NSSet<UNNotificationCategory *> *categories)) completionHandler
{
    if (completionHandler) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completionHandler([NSSet set]);
        });
    }
}

@end
