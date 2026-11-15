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

#import <UserNotifications/UNCalendarNotificationTrigger.h>

@implementation UNCalendarNotificationTrigger

@synthesize dateComponents = _dateComponents;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)triggerWithDateMatchingComponents:(NSDateComponents *)dateComponents repeats:(BOOL)repeats
{
    UNCalendarNotificationTrigger *trigger = [[[self alloc] init] autorelease];
    trigger->_dateComponents = [dateComponents copy];
    trigger->_repeats = repeats;
    return trigger;
}

- (void)dealloc
{
    [_dateComponents release];
    [super dealloc];
}

- (NSDate *)nextTriggerDate
{
    NSCalendar *calendar = [NSCalendar currentCalendar];
    return [calendar dateFromComponents:_dateComponents];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNCalendarNotificationTrigger *copy = [super copyWithZone:zone];
    copy->_dateComponents = [self.dateComponents copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_dateComponents forKey:@"dateComponents"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        if ([coder respondsToSelector:@selector(decodeObjectOfClass:forKey:)]) {
            _dateComponents = [[coder decodeObjectOfClass:[NSDateComponents class] forKey:@"dateComponents"] copy];
        }
        if (!_dateComponents) {
            _dateComponents = [[coder decodeObjectForKey:@"dateComponents"] copy];
        }
    }
    return self;
}

@end
