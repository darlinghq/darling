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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONSETTINGS_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONSETTINGS_H_

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, UNAuthorizationStatus) {
    UNAuthorizationStatusNotDetermined = 0,
    UNAuthorizationStatusDenied,
    UNAuthorizationStatusAuthorized,
    UNAuthorizationStatusProvisional,
    UNAuthorizationStatusEphemeral
};

typedef NS_ENUM(NSInteger, UNNotificationSetting) {
    UNNotificationSettingNotSupported = 0,
    UNNotificationSettingDisabled,
    UNNotificationSettingEnabled
};

typedef NS_ENUM(NSInteger, UNAlertStyle) {
    UNAlertStyleNone = 0,
    UNAlertStyleBanner,
    UNAlertStyleAlert
};

typedef NS_ENUM(NSInteger, UNShowPreviewsSetting) {
    UNShowPreviewsSettingAlways = 0,
    UNShowPreviewsSettingWhenAuthenticated,
    UNShowPreviewsSettingNever
};

@interface UNNotificationSettings : NSObject <NSCopying, NSSecureCoding> {
    UNAuthorizationStatus _authorizationStatus;
    UNNotificationSetting _soundSetting;
    UNNotificationSetting _badgeSetting;
    UNNotificationSetting _alertSetting;
    UNNotificationSetting _notificationCenterSetting;
    UNNotificationSetting _lockScreenSetting;
    UNNotificationSetting _carPlaySetting;
    UNAlertStyle _alertStyle;
    UNShowPreviewsSetting _showPreviewsSetting;
    UNNotificationSetting _criticalAlertSetting;
    BOOL _providesAppNotificationSettings;
    UNNotificationSetting _announcementSetting;
}

@property(readonly) UNAuthorizationStatus authorizationStatus;
@property(readonly) UNNotificationSetting soundSetting;
@property(readonly) UNNotificationSetting badgeSetting;
@property(readonly) UNNotificationSetting alertSetting;
@property(readonly) UNNotificationSetting notificationCenterSetting;
@property(readonly) UNNotificationSetting lockScreenSetting;
@property(readonly) UNNotificationSetting carPlaySetting;
@property(readonly) UNAlertStyle alertStyle;
@property(readonly) UNShowPreviewsSetting showPreviewsSetting;
@property(readonly) UNNotificationSetting criticalAlertSetting;
@property(readonly) BOOL providesAppNotificationSettings;
@property(readonly) UNNotificationSetting announcementSetting;

@end

#endif
