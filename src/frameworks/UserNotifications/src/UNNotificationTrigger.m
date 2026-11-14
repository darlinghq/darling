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

#import <UserNotifications/UNNotificationTrigger.h>

@implementation UNNotificationTrigger

@synthesize repeats = _repeats;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationTrigger *copy = [[[self class] allocWithZone:zone] init];
    copy->_repeats = self.repeats;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeBool:_repeats forKey:@"repeats"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _repeats = [coder decodeBoolForKey:@"repeats"];
    }
    return self;
}

@end
