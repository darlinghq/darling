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

#import <UserNotifications/UNNotification.h>

@implementation UNNotification

@synthesize date = _date;
@synthesize request = _request;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)notificationWithRequest:(UNNotificationRequest *)request date:(NSDate *)date
{
    UNNotification *notification = [[[self alloc] init] autorelease];
    notification->_request = [request copy];
    notification->_date = date ? [date copy] : [[NSDate date] copy];
    return notification;
}

- (void)dealloc
{
    [_date release];
    [_request release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotification *copy = [[[self class] allocWithZone:zone] init];
    copy->_date = [self.date copyWithZone:zone];
    copy->_request = [self.request copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_date forKey:@"date"];
    [coder encodeObject:_request forKey:@"request"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _date = [[coder decodeObjectOfClass:[NSDate class] forKey:@"date"] copy];
        _request = [[coder decodeObjectOfClass:[UNNotificationRequest class] forKey:@"request"] copy];
    }
    return self;
}

@end
