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

#import <UserNotifications/UNNotificationResponse.h>

NSString *const UNNotificationDefaultActionIdentifier = @"com.apple.UNNotificationDefaultActionIdentifier";
NSString *const UNNotificationDismissActionIdentifier = @"com.apple.UNNotificationDismissActionIdentifier";

@implementation UNNotificationResponse

@synthesize notification = _notification;
@synthesize actionIdentifier = _actionIdentifier;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)responseWithNotification:(UNNotification *)notification actionIdentifier:(NSString *)actionIdentifier
{
    UNNotificationResponse *response = [[[self alloc] init] autorelease];
    response->_notification = [notification copy];
    response->_actionIdentifier = [actionIdentifier copy];
    return response;
}

- (void)dealloc
{
    [_notification release];
    [_actionIdentifier release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationResponse *copy = [[[self class] allocWithZone:zone] init];
    copy->_notification = [self.notification copyWithZone:zone];
    copy->_actionIdentifier = [self.actionIdentifier copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_notification forKey:@"notification"];
    [coder encodeObject:_actionIdentifier forKey:@"actionIdentifier"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _notification = [[coder decodeObjectOfClass:[UNNotification class] forKey:@"notification"] copy];
        _actionIdentifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"actionIdentifier"] copy];
    }
    return self;
}

@end
