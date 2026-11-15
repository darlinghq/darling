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

#import <UserNotifications/UNMutableNotificationContent.h>

@implementation UNMutableNotificationContent

@dynamic attachments;
@dynamic badge;
@dynamic body;
@dynamic categoryIdentifier;
@dynamic launchImageName;
@dynamic sound;
@dynamic subtitle;
@dynamic threadIdentifier;
@dynamic title;
@dynamic userInfo;
@dynamic interruptionLevel;
@dynamic summaryArgument;
@dynamic summaryArgumentCount;
@dynamic targetContentIdentifier;
@dynamic relevanceScore;
@dynamic filterCriteria;

- (void)setAttachments:(NSArray<UNNotificationAttachment *> *)attachments
{
    if (_attachments != attachments) {
        [_attachments release];
        _attachments = [attachments copy];
    }
}

- (void)setBadge:(NSNumber *)badge
{
    if (_badge != badge) {
        [_badge release];
        _badge = [badge copy];
    }
}

- (void)setBody:(NSString *)body
{
    if (_body != body) {
        [_body release];
        _body = [body copy];
    }
}

- (void)setCategoryIdentifier:(NSString *)categoryIdentifier
{
    if (_categoryIdentifier != categoryIdentifier) {
        [_categoryIdentifier release];
        _categoryIdentifier = [categoryIdentifier copy];
    }
}

- (void)setLaunchImageName:(NSString *)launchImageName
{
    if (_launchImageName != launchImageName) {
        [_launchImageName release];
        _launchImageName = [launchImageName copy];
    }
}

- (void)setSound:(UNNotificationSound *)sound
{
    if (_sound != sound) {
        [_sound release];
        _sound = [sound copy];
    }
}

- (void)setSubtitle:(NSString *)subtitle
{
    if (_subtitle != subtitle) {
        [_subtitle release];
        _subtitle = [subtitle copy];
    }
}

- (void)setThreadIdentifier:(NSString *)threadIdentifier
{
    if (_threadIdentifier != threadIdentifier) {
        [_threadIdentifier release];
        _threadIdentifier = [threadIdentifier copy];
    }
}

- (void)setTitle:(NSString *)title
{
    if (_title != title) {
        [_title release];
        _title = [title copy];
    }
}

- (void)setUserInfo:(NSDictionary *)userInfo
{
    if (_userInfo != userInfo) {
        [_userInfo release];
        _userInfo = [userInfo copy];
    }
}

- (void)setInterruptionLevel:(UNNotificationInterruptionLevel)interruptionLevel
{
    _interruptionLevel = interruptionLevel;
}

- (void)setSummaryArgument:(NSString *)summaryArgument
{
    if (_summaryArgument != summaryArgument) {
        [_summaryArgument release];
        _summaryArgument = [summaryArgument copy];
    }
}

- (void)setSummaryArgumentCount:(NSUInteger)summaryArgumentCount
{
    _summaryArgumentCount = summaryArgumentCount;
}

- (void)setTargetContentIdentifier:(NSString *)targetContentIdentifier
{
    if (_targetContentIdentifier != targetContentIdentifier) {
        [_targetContentIdentifier release];
        _targetContentIdentifier = [targetContentIdentifier copy];
    }
}

- (void)setRelevanceScore:(double)relevanceScore
{
    _relevanceScore = relevanceScore;
}

- (void)setFilterCriteria:(NSString *)filterCriteria
{
    if (_filterCriteria != filterCriteria) {
        [_filterCriteria release];
        _filterCriteria = [filterCriteria copy];
    }
}

@end
