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

#import <UserNotifications/UNNotificationContent.h>
#import <UserNotifications/UNMutableNotificationContent.h>

@implementation UNNotificationContent

@synthesize attachments = _attachments;
@synthesize badge = _badge;
@synthesize body = _body;
@synthesize categoryIdentifier = _categoryIdentifier;
@synthesize launchImageName = _launchImageName;
@synthesize sound = _sound;
@synthesize subtitle = _subtitle;
@synthesize threadIdentifier = _threadIdentifier;
@synthesize title = _title;
@synthesize userInfo = _userInfo;
@synthesize interruptionLevel = _interruptionLevel;
@synthesize summaryArgument = _summaryArgument;
@synthesize summaryArgumentCount = _summaryArgumentCount;
@synthesize targetContentIdentifier = _targetContentIdentifier;
@synthesize relevanceScore = _relevanceScore;
@synthesize filterCriteria = _filterCriteria;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _interruptionLevel = UNNotificationInterruptionLevelActive;
    }
    return self;
}

- (void)dealloc
{
    [_attachments release];
    [_badge release];
    [_body release];
    [_categoryIdentifier release];
    [_launchImageName release];
    [_sound release];
    [_subtitle release];
    [_threadIdentifier release];
    [_title release];
    [_userInfo release];
    [_summaryArgument release];
    [_targetContentIdentifier release];
    [_filterCriteria release];
    [super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
    UNNotificationContent *copy = [[[self class] allocWithZone:zone] init];
    copy->_attachments = [self.attachments copyWithZone:zone];
    copy->_badge = [self.badge copyWithZone:zone];
    copy->_body = [self.body copyWithZone:zone];
    copy->_categoryIdentifier = [self.categoryIdentifier copyWithZone:zone];
    copy->_launchImageName = [self.launchImageName copyWithZone:zone];
    copy->_sound = [self.sound copyWithZone:zone];
    copy->_subtitle = [self.subtitle copyWithZone:zone];
    copy->_threadIdentifier = [self.threadIdentifier copyWithZone:zone];
    copy->_title = [self.title copyWithZone:zone];
    copy->_userInfo = [self.userInfo copyWithZone:zone];
    copy->_interruptionLevel = self.interruptionLevel;
    copy->_summaryArgument = [self.summaryArgument copyWithZone:zone];
    copy->_summaryArgumentCount = self.summaryArgumentCount;
    copy->_targetContentIdentifier = [self.targetContentIdentifier copyWithZone:zone];
    copy->_relevanceScore = self.relevanceScore;
    copy->_filterCriteria = [self.filterCriteria copyWithZone:zone];
    return copy;
}

- (id)mutableCopyWithZone:(NSZone *)zone
{
    UNMutableNotificationContent *copy = [[UNMutableNotificationContent allocWithZone:zone] init];
    copy.attachments = self.attachments;
    copy.badge = self.badge;
    copy.body = self.body;
    copy.categoryIdentifier = self.categoryIdentifier;
    copy.launchImageName = self.launchImageName;
    copy.sound = self.sound;
    copy.subtitle = self.subtitle;
    copy.threadIdentifier = self.threadIdentifier;
    copy.title = self.title;
    copy.userInfo = self.userInfo;
    copy.interruptionLevel = self.interruptionLevel;
    copy.summaryArgument = self.summaryArgument;
    copy.summaryArgumentCount = self.summaryArgumentCount;
    copy.targetContentIdentifier = self.targetContentIdentifier;
    copy.relevanceScore = self.relevanceScore;
    copy.filterCriteria = self.filterCriteria;
    return copy;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_attachments forKey:@"attachments"];
    [coder encodeObject:_badge forKey:@"badge"];
    [coder encodeObject:_body forKey:@"body"];
    [coder encodeObject:_categoryIdentifier forKey:@"categoryIdentifier"];
    [coder encodeObject:_launchImageName forKey:@"launchImageName"];
    [coder encodeObject:_sound forKey:@"sound"];
    [coder encodeObject:_subtitle forKey:@"subtitle"];
    [coder encodeObject:_threadIdentifier forKey:@"threadIdentifier"];
    [coder encodeObject:_title forKey:@"title"];
    [coder encodeObject:_userInfo forKey:@"userInfo"];
    [coder encodeInteger:_interruptionLevel forKey:@"interruptionLevel"];
    [coder encodeObject:_summaryArgument forKey:@"summaryArgument"];
    [coder encodeInteger:_summaryArgumentCount forKey:@"summaryArgumentCount"];
    [coder encodeObject:_targetContentIdentifier forKey:@"targetContentIdentifier"];
    [coder encodeDouble:_relevanceScore forKey:@"relevanceScore"];
    [coder encodeObject:_filterCriteria forKey:@"filterCriteria"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    if (self) {
        NSSet *classes = [NSSet setWithObjects:[NSArray class], [UNNotificationAttachment class], nil];
        _attachments = [[coder decodeObjectOfClasses:classes forKey:@"attachments"] copy];
        _badge = [[coder decodeObjectOfClass:[NSNumber class] forKey:@"badge"] copy];
        _body = [[coder decodeObjectOfClass:[NSString class] forKey:@"body"] copy];
        _categoryIdentifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"categoryIdentifier"] copy];
        _launchImageName = [[coder decodeObjectOfClass:[NSString class] forKey:@"launchImageName"] copy];
        _sound = [[coder decodeObjectOfClass:[UNNotificationSound class] forKey:@"sound"] copy];
        _subtitle = [[coder decodeObjectOfClass:[NSString class] forKey:@"subtitle"] copy];
        _threadIdentifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"threadIdentifier"] copy];
        _title = [[coder decodeObjectOfClass:[NSString class] forKey:@"title"] copy];
        _userInfo = [[coder decodeObjectOfClass:[NSDictionary class] forKey:@"userInfo"] copy];
        _interruptionLevel = [coder decodeIntegerForKey:@"interruptionLevel"];
        _summaryArgument = [[coder decodeObjectOfClass:[NSString class] forKey:@"summaryArgument"] copy];
        _summaryArgumentCount = [coder decodeIntegerForKey:@"summaryArgumentCount"];
        _targetContentIdentifier = [[coder decodeObjectOfClass:[NSString class] forKey:@"targetContentIdentifier"] copy];
        _relevanceScore = [coder decodeDoubleForKey:@"relevanceScore"];
        _filterCriteria = [[coder decodeObjectOfClass:[NSString class] forKey:@"filterCriteria"] copy];
    }
    return self;
}

@end
