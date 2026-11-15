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

#import <UserNotifications/UNTextInputNotificationAction.h>

@implementation UNTextInputNotificationAction

@synthesize textInputButtonTitle = _textInputButtonTitle;
@synthesize textInputPlaceholder = _textInputPlaceholder;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (instancetype)actionWithIdentifier:(NSString *)identifier
                               title:(NSString *)title
                             options:(UNNotificationActionOptions)options
                textInputButtonTitle:(NSString *)textInputButtonTitle
                textInputPlaceholder:(NSString *)textInputPlaceholder
{
    UNTextInputNotificationAction *action = [super actionWithIdentifier:identifier title:title options:options];
    if (action) {
        action->_textInputButtonTitle = [textInputButtonTitle copy];
        action->_textInputPlaceholder = [textInputPlaceholder copy];
    }
    return action;
}

- (void)dealloc
{
    [_textInputButtonTitle release];
    [_textInputPlaceholder release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNTextInputNotificationAction *copy = [super copyWithZone:zone];
    copy->_textInputButtonTitle = [self.textInputButtonTitle copyWithZone:zone];
    copy->_textInputPlaceholder = [self.textInputPlaceholder copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [super encodeWithCoder:coder];
    [coder encodeObject:_textInputButtonTitle forKey:@"textInputButtonTitle"];
    [coder encodeObject:_textInputPlaceholder forKey:@"textInputPlaceholder"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        _textInputButtonTitle = [[coder decodeObjectOfClass:[NSString class] forKey:@"textInputButtonTitle"] copy];
        _textInputPlaceholder = [[coder decodeObjectOfClass:[NSString class] forKey:@"textInputPlaceholder"] copy];
    }
    return self;
}

@end
