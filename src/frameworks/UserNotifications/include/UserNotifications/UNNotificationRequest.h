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

#ifndef _USERNOTIFICATIONS_UNNOTIFICATIONREQUEST_H_
#define _USERNOTIFICATIONS_UNNOTIFICATIONREQUEST_H_

#import <Foundation/Foundation.h>
#import <UserNotifications/UNNotificationContent.h>
#import <UserNotifications/UNNotificationTrigger.h>

@interface UNNotificationRequest : NSObject <NSCopying, NSSecureCoding> {
    NSString *_identifier;
    UNNotificationContent *_content;
    UNNotificationTrigger *_trigger;
}

+ (instancetype)requestWithIdentifier:(NSString *)identifier
                              content:(UNNotificationContent *)content
                              trigger:(nullable UNNotificationTrigger *)trigger;

@property(readonly, copy) NSString *identifier;
@property(readonly, copy) UNNotificationContent *content;
@property(readonly, copy, nullable) UNNotificationTrigger *trigger;

@end

#endif
