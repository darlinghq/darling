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

#import <UserNotifications/UNTextInputNotificationResponse.h>

@implementation UNTextInputNotificationResponse

@synthesize userText = _userText;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)responseWithNotification:(UNNotification *)notification actionIdentifier:(NSString *)actionIdentifier userText:(nullable NSString *)userText
{
    UNTextInputNotificationResponse *response = [[[self alloc] init] autorelease];
    response->_notification = [notification copy];
    response->_actionIdentifier = [actionIdentifier copy];
    response->_userText = [userText copy];
    return response;
}

- (void)dealloc
{
    [_userText release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNTextInputNotificationResponse *copy = [super copyWithZone:zone];
    copy->_userText = [self.userText copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_userText forKey:@"userText"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        _userText = [[coder decodeObjectOfClass:[NSString class] forKey:@"userText"] copy];
    }
    return self;
}

@end
