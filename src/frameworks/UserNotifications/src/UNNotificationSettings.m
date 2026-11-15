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

#import <UserNotifications/UNNotificationSettings.h>

@implementation UNNotificationSettings

@synthesize authorizationStatus = _authorizationStatus;
@synthesize soundSetting = _soundSetting;
@synthesize badgeSetting = _badgeSetting;
@synthesize alertSetting = _alertSetting;
@synthesize notificationCenterSetting = _notificationCenterSetting;
@synthesize lockScreenSetting = _lockScreenSetting;
@synthesize carPlaySetting = _carPlaySetting;
@synthesize alertStyle = _alertStyle;
@synthesize showPreviewsSetting = _showPreviewsSetting;
@synthesize criticalAlertSetting = _criticalAlertSetting;
@synthesize providesAppNotificationSettings = _providesAppNotificationSettings;
@synthesize announcementSetting = _announcementSetting;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _authorizationStatus = UNAuthorizationStatusAuthorized;
        _soundSetting = UNNotificationSettingEnabled;
        _badgeSetting = UNNotificationSettingEnabled;
        _alertSetting = UNNotificationSettingEnabled;
        _notificationCenterSetting = UNNotificationSettingEnabled;
        _lockScreenSetting = UNNotificationSettingEnabled;
        _carPlaySetting = UNNotificationSettingDisabled;
        _alertStyle = UNAlertStyleBanner;
        _showPreviewsSetting = UNShowPreviewsSettingAlways;
        _criticalAlertSetting = UNNotificationSettingDisabled;
        _providesAppNotificationSettings = NO;
        _announcementSetting = UNNotificationSettingDisabled;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationSettings *copy = [[[self class] allocWithZone:zone] init];
    copy->_authorizationStatus = self.authorizationStatus;
    copy->_soundSetting = self.soundSetting;
    copy->_badgeSetting = self.badgeSetting;
    copy->_alertSetting = self.alertSetting;
    copy->_notificationCenterSetting = self.notificationCenterSetting;
    copy->_lockScreenSetting = self.lockScreenSetting;
    copy->_carPlaySetting = self.carPlaySetting;
    copy->_alertStyle = self.alertStyle;
    copy->_showPreviewsSetting = self.showPreviewsSetting;
    copy->_criticalAlertSetting = self.criticalAlertSetting;
    copy->_providesAppNotificationSettings = self.providesAppNotificationSettings;
    copy->_announcementSetting = self.announcementSetting;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeInteger:_authorizationStatus forKey:@"authorizationStatus"];
    [coder encodeInteger:_soundSetting forKey:@"soundSetting"];
    [coder encodeInteger:_badgeSetting forKey:@"badgeSetting"];
    [coder encodeInteger:_alertSetting forKey:@"alertSetting"];
    [coder encodeInteger:_notificationCenterSetting forKey:@"notificationCenterSetting"];
    [coder encodeInteger:_lockScreenSetting forKey:@"lockScreenSetting"];
    [coder encodeInteger:_carPlaySetting forKey:@"carPlaySetting"];
    [coder encodeInteger:_alertStyle forKey:@"alertStyle"];
    [coder encodeInteger:_showPreviewsSetting forKey:@"showPreviewsSetting"];
    [coder encodeInteger:_criticalAlertSetting forKey:@"criticalAlertSetting"];
    [coder encodeBool:_providesAppNotificationSettings forKey:@"providesAppNotificationSettings"];
    [coder encodeInteger:_announcementSetting forKey:@"announcementSetting"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _authorizationStatus = [coder decodeIntegerForKey:@"authorizationStatus"];
        _soundSetting = [coder decodeIntegerForKey:@"soundSetting"];
        _badgeSetting = [coder decodeIntegerForKey:@"badgeSetting"];
        _alertSetting = [coder decodeIntegerForKey:@"alertSetting"];
        _notificationCenterSetting = [coder decodeIntegerForKey:@"notificationCenterSetting"];
        _lockScreenSetting = [coder decodeIntegerForKey:@"lockScreenSetting"];
        _carPlaySetting = [coder decodeIntegerForKey:@"carPlaySetting"];
        _alertStyle = [coder decodeIntegerForKey:@"alertStyle"];
        _showPreviewsSetting = [coder decodeIntegerForKey:@"showPreviewsSetting"];
        _criticalAlertSetting = [coder decodeIntegerForKey:@"criticalAlertSetting"];
        _providesAppNotificationSettings = [coder decodeBoolForKey:@"providesAppNotificationSettings"];
        _announcementSetting = [coder decodeIntegerForKey:@"announcementSetting"];
    }
    return self;
}

@end
