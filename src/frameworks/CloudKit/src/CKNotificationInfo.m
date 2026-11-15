/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import "CKInternal.h"

@implementation CKNotificationInfo

@synthesize alertBody = _alertBody;
@synthesize alertLocalizationKey = _alertLocalizationKey;
@synthesize title = _title;
@synthesize titleLocalizationKey = _titleLocalizationKey;
@synthesize subtitle = _subtitle;
@synthesize subtitleLocalizationKey = _subtitleLocalizationKey;
@synthesize alertActionLocalizationKey = _alertActionLocalizationKey;
@synthesize alertLaunchImage = _alertLaunchImage;
@synthesize soundName = _soundName;
@synthesize category = _category;
@synthesize collapseIDKey = _collapseIDKey;
@synthesize alertLocalizationArgs = _alertLocalizationArgs;
@synthesize titleLocalizationArgs = _titleLocalizationArgs;
@synthesize subtitleLocalizationArgs = _subtitleLocalizationArgs;
@synthesize desiredKeys = _desiredKeys;
@synthesize shouldBadge = _shouldBadge;
@synthesize shouldSendContentAvailable = _shouldSendContentAvailable;
@synthesize shouldSendMutableContent = _shouldSendMutableContent;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

// The string properties, in the order they are archived.
static NSArray<NSString *> *CKNotificationInfoStringKeys(void)
{
    return @[@"alertBody", @"alertLocalizationKey", @"title", @"titleLocalizationKey", @"subtitle",
             @"subtitleLocalizationKey", @"alertActionLocalizationKey", @"alertLaunchImage", @"soundName",
             @"category", @"collapseIDKey"];
}

static NSArray<NSString *> *CKNotificationInfoArrayKeys(void)
{
    return @[@"alertLocalizationArgs", @"titleLocalizationArgs", @"subtitleLocalizationArgs", @"desiredKeys"];
}

static NSArray<NSString *> *CKNotificationInfoBoolKeys(void)
{
    return @[@"shouldBadge", @"shouldSendContentAvailable", @"shouldSendMutableContent"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super init])) {
        for (NSString *key in CKNotificationInfoStringKeys())
            [self setValue:[coder decodeObjectOfClass:[NSString class] forKey:key] forKey:key];
        NSSet *arrayClasses = [NSSet setWithObjects:[NSArray class], [NSString class], nil];
        for (NSString *key in CKNotificationInfoArrayKeys())
            [self setValue:[coder decodeObjectOfClasses:arrayClasses forKey:key] forKey:key];
        for (NSString *key in CKNotificationInfoBoolKeys())
            [self setValue:@([coder decodeBoolForKey:key]) forKey:key];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    for (NSString *key in CKNotificationInfoStringKeys())
        [coder encodeObject:[self valueForKey:key] forKey:key];
    for (NSString *key in CKNotificationInfoArrayKeys())
        [coder encodeObject:[self valueForKey:key] forKey:key];
    for (NSString *key in CKNotificationInfoBoolKeys())
        [coder encodeBool:[[self valueForKey:key] boolValue] forKey:key];
}

- (id)copyWithZone:(NSZone *)zone
{
    CKNotificationInfo *copy = [[[self class] allocWithZone:zone] init];
    for (NSString *key in [[CKNotificationInfoStringKeys() arrayByAddingObjectsFromArray:CKNotificationInfoArrayKeys()]
                           arrayByAddingObjectsFromArray:CKNotificationInfoBoolKeys()])
        [copy setValue:[self valueForKey:key] forKey:key];
    return copy;
}

@end
