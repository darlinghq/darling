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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONCONTENT_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONCONTENT_H_

#import <Foundation/Foundation.h>
#import <UserNotifications/UNNotificationAttachment.h>
#import <UserNotifications/UNNotificationSound.h>

typedef NS_ENUM(NSUInteger, UNNotificationInterruptionLevel) {
    UNNotificationInterruptionLevelPassive = 0,
    UNNotificationInterruptionLevelActive = 1,
    UNNotificationInterruptionLevelTimeSensitive = 2,
    UNNotificationInterruptionLevelCritical = 3
};

@interface UNNotificationContent : NSObject <NSCopying, NSMutableCopying, NSSecureCoding> {
@package
    NSArray<UNNotificationAttachment *> *_attachments;
    NSNumber *_badge;
    NSString *_body;
    NSString *_categoryIdentifier;
    NSString *_launchImageName;
    UNNotificationSound *_sound;
    NSString *_subtitle;
    NSString *_threadIdentifier;
    NSString *_title;
    NSDictionary *_userInfo;
    UNNotificationInterruptionLevel _interruptionLevel;
    NSString *_summaryArgument;
    NSUInteger _summaryArgumentCount;
    NSString *_targetContentIdentifier;
    double _relevanceScore;
    NSString *_filterCriteria;
}

@property(readonly, copy) NSArray<UNNotificationAttachment *> *attachments;
@property(readonly, copy) NSNumber *badge;
@property(readonly, copy) NSString *body;
@property(readonly, copy) NSString *categoryIdentifier;
@property(readonly, copy) NSString *launchImageName;
@property(readonly, copy) UNNotificationSound *sound;
@property(readonly, copy) NSString *subtitle;
@property(readonly, copy) NSString *threadIdentifier;
@property(readonly, copy) NSString *title;
@property(readonly, copy) NSDictionary *userInfo;
@property(readonly) UNNotificationInterruptionLevel interruptionLevel;
@property(readonly, copy) NSString *summaryArgument;
@property(readonly) NSUInteger summaryArgumentCount;
@property(readonly, copy) NSString *targetContentIdentifier;
@property(readonly) double relevanceScore;
@property(readonly, copy) NSString *filterCriteria;

@end

#endif
