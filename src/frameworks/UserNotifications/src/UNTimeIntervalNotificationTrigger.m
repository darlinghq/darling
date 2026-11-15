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

#import <UserNotifications/UNTimeIntervalNotificationTrigger.h>

@implementation UNTimeIntervalNotificationTrigger

@synthesize timeInterval = _timeInterval;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)triggerWithTimeInterval:(NSTimeInterval)timeInterval repeats:(BOOL)repeats
{
    UNTimeIntervalNotificationTrigger *trigger = [[[self alloc] init] autorelease];
    trigger->_timeInterval = timeInterval;
    trigger->_repeats = repeats;
    return trigger;
}

- (NSDate *)nextTriggerDate
{
    return [NSDate dateWithTimeIntervalSinceNow:_timeInterval];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNTimeIntervalNotificationTrigger *copy = [super copyWithZone:zone];
    copy->_timeInterval = self.timeInterval;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeDouble:_timeInterval forKey:@"timeInterval"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        _timeInterval = [coder decodeDoubleForKey:@"timeInterval"];
    }
    return self;
}

@end
