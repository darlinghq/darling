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

#import <UserNotifications/UNNotificationAttachment.h>

NSString *const UNNotificationAttachmentOptionsTypeHintKey = @"typeHint";

@implementation UNNotificationAttachment

@synthesize identifier = _identifier;
@synthesize URL = _URL;
@synthesize type = _type;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

+ (nullable instancetype)attachmentWithIdentifier:(nullable NSString *)identifier
                                              URL:(NSURL *)URL
                                          options:(nullable NSDictionary *)options
                                            error:(NSError **)error
{
    if (URL == nil) {
        if (error) {
            *error = [NSError errorWithDomain:@"UNErrorDomain" code:1 userInfo:nil];
        }
        return nil;
    }

    UNNotificationAttachment *attachment = [[[self alloc] init] autorelease];
    attachment->_identifier = identifier ? [identifier copy] : [[[NSUUID UUID] UUIDString] copy];
    attachment->_URL = [URL copy];
    attachment->_type = [[URL pathExtension] copy];
    return attachment;
}

- (void)dealloc
{
    [_identifier release];
    [_URL release];
    [_type release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationAttachment *copy = [[[self class] allocWithZone:zone] init];
    copy->_identifier = [self.identifier copyWithZone:zone];
    copy->_URL = [self.URL copyWithZone:zone];
    copy->_type = [self.type copyWithZone:zone];
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_identifier forKey:@"identifier"];
    [coder encodeObject:_URL forKey:@"URL"];
    [coder encodeObject:_type forKey:@"type"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        _identifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"identifier"] copy];
        _URL = [[coder decodeObjectOfClass:[NSURL class] forKey:@"URL"] copy];
        _type = [[coder decodeObjectOfClass:[NSString class] forKey:@"type"] copy];
    }
    return self;
}

@end
