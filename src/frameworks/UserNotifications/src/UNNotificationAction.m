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

#import <UserNotifications/UNNotificationAction.h>

@implementation UNNotificationAction

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ (instancetype) actionWithIdentifier: (NSString *) identifier
                                title: (NSString *) title
                              options: (UNNotificationActionOptions) options
{
    UNNotificationAction *action = [[self alloc] init];
    action->_identifier = [identifier copy];
    action->_title = [title copy];
    action->_options = options;
    return [action autorelease];
}

- (void) dealloc {
    [_identifier release];
    [_title release];
    [super dealloc];
}

- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (void) encodeWithCoder: (NSCoder *) coder {
    [coder encodeObject: _identifier forKey: @"identifier"];
    [coder encodeObject: _title forKey: @"title"];
    [coder encodeInteger: _options forKey: @"options"];
}

- (instancetype) initWithCoder: (NSCoder *) coder {
    self = [super init];
    if (self) {
        _identifier = [[coder decodeObjectOfClass: [NSString class] forKey: @"identifier"] copy];
        _title = [[coder decodeObjectOfClass: [NSString class] forKey: @"title"] copy];
        _options = [coder decodeIntegerForKey: @"options"];
    }
    return self;
}

- (NSString *) identifier { return _identifier; }
- (NSString *) title { return _title; }
- (UNNotificationActionOptions) options { return _options; }

@end
