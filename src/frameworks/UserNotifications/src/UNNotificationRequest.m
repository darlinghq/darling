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

#import <UserNotifications/UNNotificationRequest.h>

@implementation UNNotificationRequest

@synthesize identifier = _identifier;
@synthesize content = _content;
@synthesize trigger = _trigger;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)requestWithIdentifier:(NSString *)identifier
                              content:(UNNotificationContent *)content
                              trigger:(nullable UNNotificationTrigger *)trigger
{
    UNNotificationRequest *request = [[[self alloc] init] autorelease];
    request->_identifier = [identifier copy];
    request->_content = [content copy];
    request->_trigger = [trigger copy];
    return request;
}

- (void)dealloc
{
    [_identifier release];
    [_content release];
    [_trigger release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationRequest *copy = [[[self class] allocWithZone:zone] init];
    copy->_identifier = [self.identifier copyWithZone:zone];
    copy->_content = [self.content copyWithZone:zone];
    copy->_trigger = [self.trigger copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_identifier forKey:@"identifier"];
    [coder encodeObject:_content forKey:@"content"];
    [coder encodeObject:_trigger forKey:@"trigger"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _identifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"identifier"] copy];
        _content = [[coder decodeObjectOfClass:[UNNotificationContent class] forKey:@"content"] copy];
        _trigger = [[coder decodeObjectOfClass:[UNNotificationTrigger class] forKey:@"trigger"] copy];
    }
    return self;
}

@end
