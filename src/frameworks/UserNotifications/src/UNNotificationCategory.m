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

#import <UserNotifications/UNNotificationCategory.h>

@implementation UNNotificationCategory

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ (instancetype) categoryWithIdentifier: (NSString *) identifier
                                actions: (NSArray<UNNotificationAction *> *) actions
                      intentIdentifiers: (NSArray<NSString *> *) intentIdentifiers
                                options: (UNNotificationCategoryOptions) options
{
    UNNotificationCategory *cat = [[self alloc] init];
    cat->_identifier = [identifier copy];
    cat->_actions = [actions copy];
    cat->_intentIdentifiers = [intentIdentifiers copy];
    cat->_options = options;
    return [cat autorelease];
}

- (void) dealloc {
    [_identifier release];
    [_actions release];
    [_intentIdentifiers release];
    [super dealloc];
}

- (id) copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (void) encodeWithCoder: (NSCoder *) coder {
    [coder encodeObject: _identifier forKey: @"identifier"];
    [coder encodeObject: _actions forKey: @"actions"];
    [coder encodeObject: _intentIdentifiers forKey: @"intentIdentifiers"];
    [coder encodeInteger: _options forKey: @"options"];
}

- (instancetype) initWithCoder: (NSCoder *) coder {
    self = [super init];
    if (self) {
        _identifier = [[coder decodeObjectOfClass: [NSString class] forKey: @"identifier"] copy];
        _actions = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [UNNotificationAction class], nil] forKey: @"actions"] copy];
        _intentIdentifiers = [[coder decodeObjectOfClasses: [NSSet setWithObjects: [NSArray class], [NSString class], nil] forKey: @"intentIdentifiers"] copy];
        _options = [coder decodeIntegerForKey: @"options"];
    }
    return self;
}

- (NSString *) identifier { return _identifier; }
- (NSArray<UNNotificationAction *> *) actions { return _actions ? _actions : [NSArray array]; }
- (NSArray<NSString *> *) intentIdentifiers { return _intentIdentifiers ? _intentIdentifiers : [NSArray array]; }
- (UNNotificationCategoryOptions) options { return _options; }

@end
